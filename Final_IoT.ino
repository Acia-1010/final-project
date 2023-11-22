// Your blynk information
#define BLYNK_TEMPLATE_ID "TMPL6f6wvH9xo"
#define BLYNK_TEMPLATE_NAME "dht"
#define BLYNK_AUTH_TOKEN "-LrfiWQGfLRVrB0vPRGfAELBhHxnArbs"

#define BLYNK_PRINT Serial
#include "Config_Buzzer.h"

/*
  Library need to install:
  - DS3231: by Andrew Wickert
  - DallasTemperature: by Miles Burton
  - Blynk: by Volodymyr
  - Adafruit GFX Library: by Adafruit
  - Adafruit SSD 1306: by Adafruit
*/

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <DS3231.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

BlynkTimer timer;
RTClib myrtc;

// Your wifi information
const char* ssid = "#Acia_1310";
const char* password = "saku1310";

const int led = 2;         // Led state on pin D4 (LED_BUILTIN)
const int waterpump = 16;  // Waterpump connect output in D0
unsigned int current = 0;
int msg;

// Temperature is connect to the Arduino digital pin 4
#define ONE_WIRE_BUS 0  // D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define soilPin A0  // Soil sensor on pin A0

// Buzzer connect in Rx
// Waterpump connect input in D5

#define readData 12  // Pin D6 for Read data from sensor (Connect every VCC 3.3V)
#define waterRead 14
#define SCREEN_WIDTH 128  // Widht pixel on OLED
#define SCREEN_HEIGHT 64  // Height pixel on OLED

// Config OLED
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Connect wifi function
void connectWifi() 
{
  WiFi.begin(ssid, password);
  delay(1000);
  Serial.print("WiFi: Connecting");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(500);
    digitalWrite(led, HIGH);
  }

  Serial.println("");
  Serial.print("WiFi SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("WiFi Status: ");
  Serial.println("Connected");
  digitalWrite(led, LOW);
  delay(2000);
}

BLYNK_WRITE(V2)     // Virtual pin V2 in the Blynk app controls the waterpump.
{  
  relayState = param.asInt();

  if (relayState == 1)
  { 
    digitalWrite(waterpump, HIGH);
  }

  else 
  {
    digitalWrite(waterpump, LOW);
  }
}


// Working function
void startSensor() 
{

  display.clearDisplay();   // clear data in OLED

  // Get time from Ds3231 module
  DateTime mytime = myrtc.now();
  int hour = (mytime.hour(), DEC);
  int minute = (mytime.minute(), DEC);

  if ((hour == 6 && minute == 30) || (hour == 16 && minute == 30) || digitalRead(button) == LOW)
    digitalWrite(waterpump, HIGH);
  
  else if (relayState == 0) 
    digitalWrite(waterpump, LOW);

  // delay 5seconds
  if (millis() - msg > 5000) 
  {
    msg = millis();

    // Turn on sensor
    digitalWrite(readData, HIGH);

    // Get temperature data from Ds1820b
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);

    // Get moisture data from soil sensor
    int soil = analogRead(soilPin);             // 0-1024
    int moisture = map(soil, 0, 1023, 100, 0);  // 0-100%

    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, moisture);

    // Print to serial monitor
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.println(" *C");
    Serial.print("Moisture: ");
    Serial.print(moisture);
    Serial.println(" %");

    // Print the WiFi connection status in one line
    display.setCursor(30, 2);
    display.println("Tilly Tree");
    display.setCursor(0, 16);
    display.print("Mode: ");

    if (WiFi.status() != WL_CONNECTED) 
      display.println("Manual");
    else
      display.println("Wifi");
    
    display.setCursor(0, 26);
    display.print("Temperature: ");
    display.print(temp);
    display.print((char)247);
    display.println("C");
    display.setCursor(0, 36);
    display.print("Moisture: ");
    display.print(moisture);
    display.println("%");
    display.setCursor(0, 46);
    display.print("Water Status: ");

    Serial.print("Water State: ");
    Serial.println(relayState);

    if (digitalRead(waterRead) == HIGH) 
    {
      display.print("ON");
      playsong();
    } 
    else 
      display.print("OFF");
    
    display.display();

    delay(50);
    // Turn off sensor
    digitalWrite(readData, LOW);
  }
}

void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V3, digitalRead(waterRead));
}

void setup() {
  Serial.begin(9600);

  pinMode(led, OUTPUT);
  pinMode(waterpump, OUTPUT);
  pinMode(waterRead, INPUT);
  pinMode(button, INPUT);
  pinMode(readData, OUTPUT);

  Wire.begin();
  sensors.begin();
  connectWifi();

  digitalWrite(readData, LOW);

  // Blynk setup
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  //timer.setInterval(1000L, myTimerEvent);
  timer.setInterval(1000L, startSensor);

  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  // If lost connection from wifi. We'll start manual mode.
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    startSensor();
    digitalWrite(led, HIGH);
  }

  // If wifi have been connected. We'll send data to Blynk
  Blynk.run();
  timer.run();
  digitalWrite(led, LOW);
}