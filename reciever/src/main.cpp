#define NSS D8
#define RESET 0
#define dio0 D2

#define TRANSMITTER_ON_MESSAGE "TURNON"
#define TRANSMITTER_OFF_MESSAGE "TURNOFF"
#define TRANSMITTER_GET_STATUS_MESSAGE "GETSTATUS"

#define RECIEVER_ON_RESPONSE "TURNEDON"
#define RECIEVER_OFF_RESPONSE "TURNEDOFF"
#define RECIEVER_LOCKED_RESPONSE "LOCKED"
#define RECIEVER_WAIT_RESPONSE "WAIT"

#define MOTOR_ON_COUNT_BEFORE_LOCK 5
#define MOTOR_ACTION_TIMEOUT 1000
#define MESSAGE_CHECK_INTREVAL 1000
#define MOTOR_MINIMUM_ACTION_TIME 3E4

#define MOTOR_UNLOCK_BUTTON D4

#define MOTOR_ON_PIN D1
#define MOTOR_OFF_PIN D3

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <EEPROM.h>

// debug Serial Print Macros
#define DEBUG 1

#if DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define P(x)                \
    Serial.print(millis()); \
    Serial.print(F(": "));  \
    Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define P(x)
#endif

String message = "";
uint8_t motorOnCount = 0;

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

void sendLoRaMessage(const String &msg)
{
    LoRa.beginPacket();
    LoRa.print(msg);
    LoRa.endPacket();
}

void setup()
{
    pinMode(MOTOR_ON_PIN, OUTPUT);
    pinMode(MOTOR_OFF_PIN, OUTPUT);
    pinMode(MOTOR_UNLOCK_BUTTON, INPUT_PULLUP);

    EEPROM.begin(512);

    Serial.begin(9600);
    while (!Serial)
        delay(10);

    P("LoRa Receiver Callback");

    LoRa.setPins(NSS, RESET, dio0);

    if (!LoRa.begin(433E6))
    {
        P("Starting LoRa failed!");
        while (1)
            delay(10);
    }

    // set the sync word, which must be the same for the transmitter and receiver
    LoRa.setSyncWord(0xF3);

    // load motor on count from EEPROM
    motorOnCount = EEPROM.read(0);
    if (motorOnCount > MOTOR_ON_COUNT_BEFORE_LOCK)
    {
        motorOnCount = 0;
    }

    P("Motor On Count: " + String(motorOnCount));
    // register the receive callback
    LoRa.onReceive(onReceive);
    // put the radio into receive mode
    P("LoRa Initialized");
    LoRa.receive();
}

unsigned long lastMotorOnTime = 0;

bool isMotorLocked = false;
bool motorRunningStatus = false;

u_int32_t lastMotorActionTime = 0;
u_int32_t lastMessageTime = 0;
boolean motorAction = false;

void loop()
{

    // check for TRANSMITTER messages
    if (millis() - lastMessageTime > MESSAGE_CHECK_INTREVAL && message != "")
    {
        P("New message recieved!");
        {

            if (message == TRANSMITTER_GET_STATUS_MESSAGE)
            {
                P("Sending Status Response");
                if (isMotorLocked || motorOnCount >= MOTOR_ON_COUNT_BEFORE_LOCK)
                {
                    sendLoRaMessage(RECIEVER_LOCKED_RESPONSE);
                }
                else if (motorRunningStatus)
                {
                    sendLoRaMessage(RECIEVER_ON_RESPONSE);
                }
                else
                {
                    sendLoRaMessage(RECIEVER_OFF_RESPONSE);
                }
                message = "";
                LoRa.receive();
                return;
            }

            if (millis() - lastMotorActionTime < MOTOR_MINIMUM_ACTION_TIME)
            {
                P("No action taken. Waiting for motor to cool down. Wait for " + String((30 - millis() - lastMotorActionTime) / 1000) + "s");
                P("Sending Wait Response");
                sendLoRaMessage(RECIEVER_WAIT_RESPONSE);
                lastMessageTime = millis();
                message = "";
                LoRa.receive();
                return;
            }

            if (message == TRANSMITTER_ON_MESSAGE)
            {
                if (motorOnCount >= MOTOR_ON_COUNT_BEFORE_LOCK)
                {
                    P("Motor Locked");
                    isMotorLocked = true;
                    sendLoRaMessage(RECIEVER_LOCKED_RESPONSE);
                    message = "";
                    lastMessageTime = millis();
                    LoRa.receive();
                    return;
                }

                P("Turning Motor On");
                digitalWrite(MOTOR_ON_PIN, HIGH);
                motorRunningStatus = true;
                motorOnCount++;
                motorAction = true;
                lastMotorActionTime = millis();
                sendLoRaMessage(RECIEVER_ON_RESPONSE);
                // store motorOnCount in EEPROM
                EEPROM.write(0, motorOnCount);
                EEPROM.commit();
            }
            else if (message == TRANSMITTER_OFF_MESSAGE)
            {
                P("Turning Motor Off");
                digitalWrite(MOTOR_OFF_PIN, HIGH);
                motorRunningStatus = false;
                motorAction = true;
                lastMotorActionTime = millis();
                sendLoRaMessage(RECIEVER_OFF_RESPONSE);
            }
            else
            {
                P("Unknown Message :-");
                P(message);
            }
            message = "";
            lastMessageTime = millis();
            LoRa.receive();
        }
    }

    if (motorAction && millis() - lastMotorActionTime > MOTOR_ACTION_TIMEOUT)
    {
        P("Motor Action Timeout");
        digitalWrite(MOTOR_ON_PIN, LOW);
        digitalWrite(MOTOR_OFF_PIN, LOW);
        motorAction = false;
        LoRa.receive();
    }

    // check for MOTOR_UNLOCK_BUTTON if it is pressed for more than 5 seconds then unlock the motor
    if (digitalRead(MOTOR_UNLOCK_BUTTON) == LOW)
    {
        P("Motor Unlock Button Pressed");
        P("Unlocking Motor");
        motorOnCount = 0;
        isMotorLocked = false;
        LoRa.receive();
    }
}
