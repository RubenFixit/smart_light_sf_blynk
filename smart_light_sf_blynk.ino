/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs on Sparkfun Blynk Board.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  You can select NodeMCU 1.0 (compatible board)
  in the Tools -> Board menu

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

////////////////////////////////////
// Blynk Virtual Variable Mapping //
////////////////////////////////////
#define MODE_AUTO_M               V0
#define LIGHT_SCHEDULE            V1
#define LIGHT_LEVEL               V2
#define TERMINAL_VIRTUAL          V3
#define BOARD_TEMPERATURE_F       V5
#define EXT_TEMPERATURE_F         V6
#define BOARD_HUMIDITY            V7
#define ADC_VOLTAGE               V8
#define TEMP_OFFSET_VIRTUAL       V28
#define RUNTIME_VIRTUAL           V30 // Utility
#define RESET_VIRTUAL             V31 // Utility

#define ADC_VOLTAGE_DIVIDER 3.2

#define FADE_RATE 31

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <Wire.h>
#include "SparkFunHTU21D.h"

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "1e3fa9e4126f4b7ea07c565745d2d01c";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "QuantumGrid";
char pass[] = "t1meTra^el4fun";

// Global Variables
char Date[16];
char Time[16];
long startsecondswd;       // weekday start time in seconds
long stopsecondswd;        // weekday stop  time in seconds
long nowseconds;           // time now in seconds
String timepluswifi;
int wifisignal;
int max_light_level = 1023;
int current_light_level = 0;
int manual = 0;

BlynkTimer timer;

WidgetRTC rtc;
WidgetTerminal terminal(TERMINAL_VIRTUAL);

HTU21D thSense;

void setup() {
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass, IPAddress(45,55,96,146), 8442);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);

  // Begin synchronizing time
  rtc.begin();

  // Set up the temperature-humidity sensor
  thSense.begin();

  timer.setInterval(10000L, activetoday);  // check if schedule should run today
  timer.setInterval(5000L, clockvalue);  // check value for time
  timer.setInterval(5000L, sendWifi);    // Wi-Fi singal
}

void sendWifi() {
  wifisignal = map(WiFi.RSSI(), -105, -40, 0, 100);
}

void clockvalue() { // Digital clock display of the time

  int gmthour = hour();
  if (gmthour == 24) {
    gmthour = 0;
  }
  String displayhour =   String(gmthour, DEC);
  int hourdigits = displayhour.length();
  if (hourdigits == 1) {
    displayhour = "0" + displayhour;
  }
  String displayminute = String(minute(), DEC);
  int minutedigits = displayminute.length();
  if (minutedigits == 1) {
    displayminute = "0" + displayminute;
  }

  timepluswifi = "                               Clock:  " + displayhour + ":" + displayminute + "               Signal:  " + wifisignal + " %";
  Blynk.setProperty(TERMINAL_VIRTUAL, "label", timepluswifi);

}

void activetoday() {        // check if schedule should run today
  if (year() != 1970) {
    Blynk.syncVirtual(LIGHT_SCHEDULE); // sync timeinput widget
  }
}

BLYNK_WRITE(MODE_AUTO_M) {  // Manual/Auto selection
  manual = param.asInt();
  if (manual == 1) {
    Blynk.syncVirtual(LIGHT_LEVEL);
    terminal.println("Manual MODE");
    terminal.println("You can now use the Lights slider to control the LEDs");
    terminal.flush();
  } else {
    Blynk.syncVirtual(LIGHT_SCHEDULE);
    terminal.println("Auto MODE");
    terminal.println("The LEDs will now turn on when scheduled");
    terminal.flush();
  }
}

BLYNK_WRITE(LIGHT_LEVEL)  // Lights Slider
{
  int max_light_level_percent = param.asInt();
  max_light_level = map(max_light_level_percent, 0, 100, 0, 1023);
  if (manual == 1) {
    setLeds(max_light_level);
    terminal.print("LED's set to ");
    terminal.print(max_light_level_percent);
    terminal.println("%");
    terminal.flush();
  } else {
    terminal.print("LED's max set to ");
    terminal.print(max_light_level_percent);
    terminal.println("%");
    terminal.flush();
  }
}

BLYNK_WRITE(LIGHT_SCHEDULE) // Schedule Changed
{
  if (manual == 0) {
    sprintf(Date, "%02d/%02d/%04d",  day(), month(), year());
    sprintf(Time, "%02d:%02d:%02d", hour(), minute(), second());

    TimeInputParam t(param);

    terminal.print("Checked schedule at: ");
    terminal.println(Time);
    terminal.flush();
    int dayadjustment = -1;
    if (weekday() == 1) {
      dayadjustment =  6; // needed for Sunday, Time library is day 1 and Blynk is day 7
    }
    if (t.isWeekdaySelected(weekday() + dayadjustment)) { //Time library starts week on Sunday, Blynk on Monday
      terminal.println(" ACTIVE today");
      terminal.flush();
      if (t.hasStartTime()) // Process start time
      {
        terminal.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute());
        terminal.flush();
      }
      if (t.hasStopTime()) // Process stop time
      {
        terminal.println(String("Stop : ") + t.getStopHour() + ":" + t.getStopMinute());
        terminal.flush();
      }
      // Display timezone details, for information purposes only
      terminal.println(String("Time zone: ") + t.getTZ()); // Timezone is already added to start/stop time
      terminal.flush();

      nowseconds = ((hour() * 3600) + (minute() * 60) + second());
      startsecondswd = (t.getStartHour() * 3600) + (t.getStartMinute() * 60);
      if (nowseconds >= startsecondswd) {
        terminal.print(" STARTED at");
        terminal.println(String(" ") + t.getStartHour() + ":" + t.getStartMinute());
        terminal.flush();
        if (nowseconds <= startsecondswd + 90) {  // 90s on 60s timer ensures 1 trigger command is sent
          setLeds(max_light_level);
        }
      }
      else {
        terminal.println(" Device NOT STARTED today");
        terminal.flush();

      }
      stopsecondswd = (t.getStopHour() * 3600) + (t.getStopMinute() * 60);
      if (nowseconds >= stopsecondswd) {
        setLeds(0);
        terminal.print(" STOPPED at");
        terminal.println(String(" ") + t.getStopHour() + ":" + t.getStopMinute());
        terminal.flush();
        if (nowseconds <= stopsecondswd + 90) { // 90s on 60s timer ensures 1 trigger command is sent
          setLeds(0);

          // code here to switch the relay OFF
        }
      }
      else {
        if (nowseconds >= startsecondswd) {
          setLeds(max_light_level);
          terminal.println("Schedule is ON");
          terminal.flush();

        }
      }
    }
    terminal.println();
  }
}

// Board runs hot, subtract an offset to try to compensate:
float tempCOffset = -8.33;

BLYNK_READ(BOARD_TEMPERATURE_F)
{
  float tempC = thSense.readTemperature(); // Read from the temperature sensor
  float tempF = tempC * 9.0 / 5.0 + 32.0; // Convert to farenheit
  // Create a formatted string with 1 decimal point:
  Blynk.virtualWrite(BOARD_TEMPERATURE_F, tempF); // Update Blynk virtual value
}

BLYNK_READ(EXT_TEMPERATURE_F)
{
  float adcRaw = analogRead(A0); // Read in A0
  // Divide by 1024, then multiply by the hard-wired voltage divider max (3.2)
  float voltage = ((float)adcRaw / 1024.0) * ADC_VOLTAGE_DIVIDER;
  float tempC = (voltage - 0.5) * 100.0; // Convert from ACD
  tempC += tempCOffset; // Add any offset
  float tempF = tempC * 9.0 / 5.0 + 32.0; // Convert to farenheit
  Blynk.virtualWrite(EXT_TEMPERATURE_F, tempF); // Update Blynk virtual value
}

BLYNK_READ(BOARD_HUMIDITY)
{
  float humidity = thSense.readHumidity(); // Read from humidity sensor
  Blynk.virtualWrite(BOARD_HUMIDITY, humidity); // Update Blynk virtual value
}

// ADC is read directly
BLYNK_READ(ADC_VOLTAGE)
{
  float adcRaw = analogRead(A0); // Read in A0
  // Divide by 1024, then multiply by the hard-wired voltage divider max (3.2)
  float voltage = ((float)adcRaw / 1024.0) * ADC_VOLTAGE_DIVIDER;
  Blynk.virtualWrite(ADC_VOLTAGE, voltage); // Output value to Blynk
}

BLYNK_WRITE(TEMP_OFFSET_VIRTUAL) // Very optional virtual to set the tempC offset
{
  tempCOffset = param.asInt();
  terminal.print("Temperature offset set to ");
  terminal.print(tempCOffset);
  terminal.println(" °C");
}

// Runtime tracker utility function. Displays the run time to a
// maximum of four digits. It'll show up to 999 seconds, then
// up to 999 minutes, then up to 999 hours, then up to 999 days
BLYNK_READ(RUNTIME_VIRTUAL)
{
  float runTime = (float) millis() / 1000.0; // Convert millis to seconds
  // Assume we can only show 4 digits
  if (runTime >= 1000) // 1000 seconds = 16.67 minutes
  {
    runTime /= 60.0; // Convert to minutes
    if (runTime >= 1000) // 1000 minutes = 16.67 hours
    {
      runTime /= 60.0; // Convert to hours
      if (runTime >= 1000) // 1000 hours = 41.67 days
        runTime /= 24.0;
    }
  }
  Blynk.virtualWrite(RUNTIME_VIRTUAL, runTime);
}

// Reset WiFi and Blynk auth token
// Utility function.
BLYNK_WRITE(RESET_VIRTUAL)
{
  int resetIn = param.asInt();
  if (resetIn)
  {
    terminal.println("TODO: Do Reset");
  }
}

void setLeds(int level) {
  current_light_level = level;
  analogWrite(5, level);
}

void loop() {
  Blynk.run();
  timer.run();
}

