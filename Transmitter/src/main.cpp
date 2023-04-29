// GPIO
#define MOTOR_ON_BUTTON D4
#define MOTOR_OFF_BUTTON D0

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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool motorStatus = false;
bool lockStatus = false;

void printOnDisplay(const String &message)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(message);
    display.display();
    Serial.println(message);
}

void sendLoRaMessage(const String &message)
{
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
}

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

void setup()
{
    pinMode(MOTOR_ON_BUTTON, INPUT_PULLUP);
    pinMode(MOTOR_OFF_BUTTON, INPUT_PULLUP);

    Serial.begin(9600);
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        // Don't proceed, loop forever
        while (1)
        {
            delay(100);
        }
    }

    display.display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();
    display.display();

    printOnDisplay("Display Initialized");

    LoRa.setPins(LORA_NSS, LORA_RESET, LORA_DIO0);
    if (!LoRa.begin(433E6))
    {
        printOnDisplay("Starting LoRa failed!");
        delay(100);
        while (1)
            ;
    }
    printOnDisplay("LoRa Initialized");
}

u_int32_t lastMessageTime;
u_int32_t lastSendTime = 0;
u_int32_t interval = 200;

void loop()
{

    if (digitalRead(MOTOR_ON_BUTTON) == LOW)
    {
        printOnDisplay("Motor On Button Pressed");

        if (millis() - lastMessageTime > 10000)
        {

            printOnDisplay("Motor On Signal Sent");
            sendLoRaMessage("Muhsin turned on the motor");
            lastMessageTime = millis();
            delay(2000);
        }
    }
    else if (digitalRead(MOTOR_OFF_BUTTON) == LOW)
    {
        printOnDisplay("Motor Off Signal Sent");
        sendLoRaMessage("Muhsin MOTOR OFF Cheythu");
    }
}