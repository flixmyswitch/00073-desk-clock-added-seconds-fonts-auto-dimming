/*  A very basic matrix clock using a Wemos d1 Mini and 4 x 1 Max7219 matrix
    Based upon https://www.youtube.com/watch?v=g62Atuf1cm4&t=324s
    There is no debugging
    Just add you wifi SSID and PASSWORD in to the myconfig.h file
    The clock will show the wrong time for upto 60 seconds once connected to wifi
    No AM/PM indication on 12 hour clock
    No date scrolling, we will add this in later code
    
    Edited by Andy @Flixmyswitch    www.flixmyswitch.com for updates on this code and more explanations

    code:- desk clock 0002 dated 03 March 2023

    New in this code

    1. Seconds added
    2. Auto dimming
    3. No flashing seperator dots between hours and minutes, just static
    4. Custom fonts
    5. Look out for ver2 in the code so you know whats new
*/

#ifdef ESP8266
#include <ESP8266WiFi.h>  // Built-in for ESP8266
#else
#include <WiFi.h>  // Built-in for ESP32
#endif
#include <MD_Parola.h>
//#include <SPI.h>   // use these for Arduino
//#include <time.h>  // use these for Arduino  /* time_t, struct tm, time, localtime, strftime */
#include "myconfig.h"
#include "Font_Data.h"  // ver2
// needed for Parola
#define SPEED_TIME 75
#define PAUSE_TIME 0

uint16_t h, m, s;    // to store hours, minutes ver2
String Hour_format;  // to select M or I for 24hrs or 12hr clock

char hour_output[3], s_output[3], m_output[3], hm_output[10];  // ver2

//********* USER - CHANGE YOUR SETTING BELOW **********

String Time_Display = "M";  // M for metric 24hrs clock,  I for imperial 12hrs clock

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW  // See video number 45 you have a choice of 4 settings
const uint8_t MAX_DEVICES = 4;             // As we have a 4x1 matrix
#define MAX_NUMBER_OF_ZONES 2              // We have two ZONES, one for hh:mm and the other for seconds

const uint8_t CLK_PIN = 14;   // or SCK    Connections to a Wemos D1 Mini
const uint8_t DATA_PIN = 13;  // or MOSI
const uint8_t CS_PIN = 15;    // or SS
byte Time_dim = 20;            // what time to auto dim the display 24 hrs clock ver2
byte Time_bright = 7;          // what time to auto dim the display 24 hr clock ver2
byte Max_bright = 3;           // max brightness of the maxtrix  ver2
byte Min_bright = 0;           // minimum brightness of the matrix  ver2

// bytes can store 0 to 255, an int can store -32,768 to 32,767 so bytes take up less memory

// ********** Change to your WiFi credentials, select your time zone and adjust your NTS **********

const char* ssid = WIFI_SSID;                         // SSID of local network
const char* password = WIFI_PW;                       // Password on network
const char* Timezone = "CET-1CEST,M3.5.0,M10.5.0/3";  // Rome/italy see link below for more short codes -1
const char* NTP_Server_1 = "it.pool.ntp.org";
const char* NTP_Server_2 = "time.nist.gov";
//********* END OF USER SETTINGS**********/

/*  Useful links

    Zones               https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

    NTP world servers   https://www.ntppool.org/en/

    World times         https://www.timeanddate.com/time/zones/
*/

MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);  // I have called my instance "P"



// https://www.tutorialspoint.com/c_standard_library/c_function_sprintf.htm


void setup() {
  Serial.begin(115200);
  StartWiFi();
  setenv("TZ", Timezone, 1);
  configTime(0, 0, NTP_Server_1, NTP_Server_2);

  Hour_format = Time_Display;  // using the string above to select the type of clock 24 hrs or 12hr

  P.begin(MAX_NUMBER_OF_ZONES);  // start Parola  ver2
  P.setInvert(false);
  P.displayClear();
  P.setIntensity(Max_bright);   // brightness of matrix  ver2
  P.setZone(0, 1, 3);          // for HH:MM  zone 0
  P.setZone(1, 0, 0);          // for SS   ver 2  zone 1
  P.setFont(0, F4x7straight);  // bigger font 4 x 7  ver2
  P.setFont(1, F3x7straight);  // small font 3 x 7  ver2

  P.displayZoneText(0, hm_output, PA_LEFT, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(1, s_output, PA_LEFT, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);  // ver2
}


void loop() {
  UpdateLocalTime(Hour_format);

  if (h >= Time_dim || h <= Time_bright) P.setIntensity(Min_bright);  // based using 24hr clock to save code dim at 20:00 bright at 07:00 ver2
  else {
    P.setIntensity(Max_bright);  // matrix brightness
  }

  P.displayAnimate();

/* you can put other code in here
*********************************
*********************************
*********************************
**********************************/

  P.displayReset(0);  // zone 0 update the display
  P.displayReset(1);  //  to show the seconds ver2

}
//#########################################################################################
void UpdateLocalTime(String Format) {
  time_t now;  // = time(nullptr);
  time(&now);

  //Serial.println(time(&now)); Unix time or Epoch or Posix time, time since 00:00:00 UTC 1st jan 1970 minus leap seconds
  // an example as of 28 feb 2023 at 16:19 1677597570

  //See http://www.cplusplus.com/reference/ctime/strftime/

  if (Hour_format == "M") { 

    strftime(hm_output, 10, "%H:%M", localtime(&now));  // hours and minutes dot flashing dots  ver 2
  } else {
    strftime(hm_output, 10, "%I:%M", localtime(&now));  // Formats hour as: 02,  12hrs clock  ver 2
  }
  strftime(hour_output, 3, "%H", localtime(&now));  // seconds xx  ver2
  strftime(s_output, 3, "%S", localtime(&now));     // seconds xx  ver2
  h = atoi(hour_output);                            // convert char to int NOTE  based on 24hr time ver2
}

//#########################################################################################
void StartWiFi() {
  /* Set the ESP to be a WiFi-client, otherwise by default, it acts as ss both a client and an access-point
      and can cause network-issues with other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  Serial.print(F("\r\nConnecting to SSID: "));
  Serial.println(String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(" ");
  Serial.print("WiFi connected to address: ");
  Serial.print(WiFi.localIP());
  Serial.println(" ");
}