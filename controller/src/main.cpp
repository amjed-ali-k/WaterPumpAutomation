// GPIO
#define MOTOR_ON_BUTTON D4
#define MOTOR_OFF_BUTTON D0

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

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LoRa.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define LORA_NSS D8
#define LORA_RESET -1
#define LORA_DIO0 D3

#define DEBUG 1

#if DEBUG
#define P(x) Serial.print(x)
#define PLN(x) Serial.println(x)
#define PT(x)               \
    Serial.print(millis()); \
    Serial.print(F(": "));  \
    Serial.println(x)
#else
#define P(x)
#define PLN(x)
#define PT(x)
#endif

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool motorStatus = false;
bool lockStatus = false;

String message = "";
uint8_t motorOnCount = 0;

void onReceive(int packetSize)
{
    // received a packet
    P("Received packet '");

    // read packet
    for (int i = 0; i < packetSize; i++)
    {
        message += (char)LoRa.read();
    }

    // print RSSI of packet
    P("' with RSSI ");
    PLN(LoRa.packetRssi());
}

void sendLoRaMessage(const String &msg)
{
    LoRa.beginPacket();
    LoRa.print(msg);
    LoRa.endPacket();
}

void printBottomMessageOnDisplay(const String &msg)
{
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.fillRect(0, 56, 128, 8, BLACK);
    display.setCursor(0, 56);
    display.println(msg);
    display.display();
}

void printStatusOnDisplay(const String &msg)
{
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.fillRect(0, 0, 128, 24, BLACK);
    display.setCursor(0, 0);
    display.println(msg);
    display.display();
}

void setup()
{
    pinMode(MOTOR_ON_BUTTON, INPUT_PULLUP);
    pinMode(MOTOR_OFF_BUTTON, INPUT_PULLUP);
#if DEBUG
    Serial.begin(9600);
#endif
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        P(F("SSD1306 allocation failed"));
        // Don't proceed, loop forever
        while (1)
        {
            delay(100);
        }
    }
    // Clear the buffer
    display.clearDisplay();
    printStatusOnDisplay("N/A");
    printBottomMessageOnDisplay("Initializing LoRa");
    delay(1000);

    LoRa.setPins(LORA_NSS, LORA_RESET, LORA_DIO0);
    if (!LoRa.begin(433E6))
    {
        printBottomMessageOnDisplay("Starting LoRa failed!");
        while (1)
            delay(100);
    }

    // set the sync word, which must be the same for the transmitter and receiver
    LoRa.setSyncWord(0xF3);
    printBottomMessageOnDisplay("LoRa Initialized");
    // register the receive callback
    LoRa.onReceive(onReceive);
    // put the radio into receive mode
    P("LoRa Initialized");
    LoRa.receive();
}

u_int32_t lastMessageTime;
u_int32_t lastSendTime = 0;
u_int32_t interval = 200;
u_int32_t lastInputReadTime = 0;
u_int32_t lastRecieveUpdateTime = 0;

void loop()
{
    if (millis() - lastInputReadTime > 200)
    {
        lastInputReadTime = millis();
        if (digitalRead(MOTOR_ON_BUTTON) == LOW)
        {
            if (millis() - lastMessageTime > 10000)
            {
                sendLoRaMessage(TRANSMITTER_ON_MESSAGE);
                lastMessageTime = millis();
                printBottomMessageOnDisplay("Sent MOTOR ON signal");
            }
            else
            {
                printBottomMessageOnDisplay("Wait for " + String(10 - (millis() - lastMessageTime) / 1000) + "s");
            }
        }
        else if (digitalRead(MOTOR_OFF_BUTTON) == LOW)
        {
            sendLoRaMessage(TRANSMITTER_OFF_MESSAGE);
            printBottomMessageOnDisplay("Sent MOTOR OFF signal");
        }
        LoRa.receive();
    }

    if (millis() - lastRecieveUpdateTime > 1000)

    {

        lastRecieveUpdateTime = millis();
        if (message != "")
            printBottomMessageOnDisplay("Message Recieved! Message=" + message);

        if (message == RECIEVER_LOCKED_RESPONSE)
        {
            printStatusOnDisplay("LOCK");
        }
        else if (message == RECIEVER_ON_RESPONSE)
        {
            printStatusOnDisplay("ON");
        }
        else if (message == RECIEVER_OFF_RESPONSE)
        {
            printStatusOnDisplay("OFF");
        }
        else if (message == RECIEVER_WAIT_RESPONSE)
        {
            printBottomMessageOnDisplay("Wait few seconds");
        }
        message = "";
        LoRa.receive();
    }
}