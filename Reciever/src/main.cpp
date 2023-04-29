#define NSS D8
#define RESET 0
#define dio0 D2

#define TRANSMITTER_ON_MESSAGE "TURNON"
#define TRANSMITTER_OFF_MESSAGE "TURNOFF"

#define RECIEVER_ON_RESPONSE "TURNEDON"
#define RECIEVER_OFF_RESPONSE "TURNEDOFF"
#define RECIEVER_LOCKED_RESPONSE "LOCKED"
#define RECIEVER_WAIT_RESPONSE "WAIT"

#define MOTOR_ON_COUNT_BEFORE_LOCK 5
#define MOTOR_ACTION_TIMEOUT 1000
#define MESSAGE_CHECK_INTREVAL 1000
#define MOTOR_MINIMUM_ACTION_TIME 3E4

#define MOTOR_ON_PIN D1
#define MOTOR_OFF_PIN D3

#include <SPI.h>
#include <LoRa.h>

// debug Serial Print Macros
#define DEBUG 1

#if DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_P(x)          \
    Serial.print(millis()); \
    Serial.print(F(": "));  \
    Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_P(x)
#endif

String message = "";

void onReceive(int packetSize)
{
    // received a packet
    DEBUG_PRINT("Received packet '");

    // read packet
    for (int i = 0; i < packetSize; i++)
    {
        message += (char)LoRa.read();
    }

    // print RSSI of packet
    DEBUG_PRINT("' with RSSI ");
    DEBUG_PRINTLN(LoRa.packetRssi());
}

void sendLoRaMessage(const String &message)
{
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        delay(10);

    DEBUG_P("LoRa Receiver Callback");

    LoRa.setPins(NSS, RESET, dio0);

    if (!LoRa.begin(433E6))
    {
        DEBUG_P("Starting LoRa failed!");
        while (1)
            delay(10);
    }

    // register the receive callback
    LoRa.onReceive(onReceive);
    // put the radio into receive mode
    DEBUG_P("LoRa Initialized");
    LoRa.receive();
}

unsigned long lastMotorOnTime = 0;
uint8_t motorOnCount = 0;

enum motorStatusEnum
{
    OFF = 0,
    ON = 1,
    LOCKED = 2
};

motorStatusEnum motorStatus = OFF;

u_int32_t lastMotorActionTime = 0;
u_int32_t lastMessageTime = 0;
boolean motorAction = false;

void loop()
{

    // check for TRANSMITTER messages
    if (millis() - lastMessageTime > MESSAGE_CHECK_INTREVAL && message != "")
    {
        DEBUG_P("New message recieved!");
        {

            if (millis() - lastMotorActionTime > MOTOR_MINIMUM_ACTION_TIME)
            {
                DEBUG_P("No action taken. Waiting for motor to cool down");
                DEBUG_P("Sending Wait Response");
                sendLoRaMessage(RECIEVER_WAIT_RESPONSE);
                LoRa.receive();
                return;
            }

            if (message == TRANSMITTER_ON_MESSAGE)
            {
                if (motorOnCount >= MOTOR_ON_COUNT_BEFORE_LOCK)
                {
                    DEBUG_P("Motor Locked");
                    motorStatus = LOCKED;
                    sendLoRaMessage(RECIEVER_LOCKED_RESPONSE);
                    LoRa.receive();
                    return;
                }

                DEBUG_P("Turning Motor On");
                digitalWrite(MOTOR_ON_PIN, HIGH);
                motorStatus = ON;
                motorOnCount++;
                motorAction = true;
                lastMotorActionTime = millis();
                sendLoRaMessage(RECIEVER_ON_RESPONSE);
            }
            else if (message == TRANSMITTER_OFF_MESSAGE)
            {
                DEBUG_P("Turning Motor Off");
                digitalWrite(MOTOR_OFF_PIN, HIGH);
                motorStatus = OFF;
                motorAction = true;
                lastMotorActionTime = millis();
                sendLoRaMessage(RECIEVER_OFF_RESPONSE);
            }
            else
            {
                DEBUG_P("Unknown Message :-");
                DEBUG_P(message);
            }
            message = "";
            lastMessageTime = millis();
            LoRa.receive();
        }

        if (motorAction && millis() - lastMotorActionTime > MOTOR_ACTION_TIMEOUT)
        {
            DEBUG_P("Motor Action Timeout");
            digitalWrite(MOTOR_ON_PIN, LOW);
            digitalWrite(MOTOR_OFF_PIN, LOW);
            motorAction = false;
            LoRa.receive();
        }
    }
