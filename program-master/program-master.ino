#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <DMDESP.h>

#include <Wire.h>
#include <RtcDS3231.h>
#include <ESP_EEPROM.h>

#include "PrayerTimes.h"


#include <fonts/SystemFont5x7.h>
#include <fonts/Font4x6.h>
#include <fonts/System4x7.h>
#include <fonts/SmallCap4x6.h>
#include <fonts/EMSans6x16.h>


//SETUP DMD
#define DISPLAYS_WIDE 2
#define DISPLAYS_HIGH 2

#define BUZZ  D4 // PIN BUZZER

#define Font0 SystemFont5x7
#define Font1 Font4x6
#define Font2 System4x7 
#define Font3 SmallCap4x6
#define Font4 EMSans6x16
//////////hijriyah
#define epochHijriah          1948439.5f //math.harvard.edu
#define tambahKurangHijriah   0

// Ukuran EEPROM (pastikan cukup untuk semua data)
#define EEPROM_SIZE 50
//#include "icon.h"

//create object
RtcDS3231<TwoWire> Rtc(Wire);
DMDESP  Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  // Jumlah Panel P10 yang digunakan (KOLOM,BARIS)
RtcDateTime now;
ESP8266WebServer server(80);
double times[sizeof(TimeName)/sizeof(char*)];

uint8_t maxday[]            = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
uint8_t ihtiSholat[]        = {0,0,0,0,0};
uint8_t iqomah[]        = {5,1,5,5,5,2,5};
uint8_t displayBlink[]  = {5,0,5,5,5,5,5};
uint8_t dataIhty[]      = {3,0,3,3,0,3,2};

struct Config {
  uint8_t chijir;
  uint8_t durasiadzan;
  uint8_t ihti;
  float latitude = -7.364057;
  float longitude = 112.646222;
  uint8_t zonawaktu = 7;
};


struct TanggalDanWaktu
{
  uint8_t detik;
  uint8_t menit;
  uint8_t jam;
  uint8_t hari;
  uint8_t tanggal;
  uint8_t bulan;
  uint8_t tahun;
};

struct Tanggal
{
  uint8_t tanggal;
  uint8_t bulan;
  uint16_t tahun;
};

struct TanggalJawa
{
  uint8_t pasaran;
  uint8_t wuku;
  uint8_t tanggal;
  uint8_t bulan;
  uint16_t tahun;
};

struct JamDanMenit
{
  uint8_t jam;
  uint8_t menit;
};

TanggalDanWaktu tanggalMasehi;
Tanggal tanggalHijriah;
TanggalJawa tanggalJawa;
JamDanMenit waktuMagrib;
Config config;

// Pengaturan hotspot WiFi dari ESP8266
 char ssid[20]     = "JAM_PANEL_MUSHOLLAH";
 char password[20] = "00000000";
 const char* host = "OTA-PANEL";

// Variabel untuk waktu, tanggal, teks berjalan, tampilan ,dan kecerahan
String setJam        = "00:00:00";
String setTanggal    = "01-01-2024";
String setText       = "Selamat Datang!";
char info1[100];
char info2[100];
uint16_t    brightness    = 50;
char   text[200] ;
bool   adzan         = 0;
bool   stateBuzzer   = 1;
uint8_t    trigJam       = 17;
uint8_t    trigMenit     = 30;
uint8_t    DWidth        = Disp.width();
uint8_t    DHeight       = Disp.height();
uint8_t    sholatNow     = -1;
bool       reset_x       = 0; 

/*======library tambahan=======*/
//byte   tampilan      = 1;
//byte   mode          = 1;
uint8_t   list          = 0; 
//bool   flag1         = 1;//variabel untuk menyimpan status animasi running text  
uint8_t    speedDate      = 40; // Kecepatan default date
uint8_t    speedText1     = 40; // Kecepatan default text  
uint8_t     speedText2    = 40;
float dataFloat[10];
int   dataInteger[10];
uint8_t indexText;
bool flagClock=false;
bool flagText=false;
bool flagUpdate=false;
/*==============================*/

enum Show{
  ANIM_INFO,
  ANIM_JAM,
  ANIM_TEXT,
  ANIM_SHOLAT,
  ANIM_ADZAN,
  ANIM_IQOMAH,
  ANIM_BLINK,
  //ANIM_ZONK
};

Show show = ANIM_JAM;

IPAddress local_IP(192, 168, 2, 1);      // IP Address untuk AP
IPAddress gateway(192, 168, 2, 1);       // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask

//----------------------------------------------------------------------
// HJS589 P10 FUNGSI TAMBAHAN UNTUK NODEMCU ESP8266

void ICACHE_RAM_ATTR refresh() {
  Disp.refresh();
  timer0_write(ESP.getCycleCount() + 80000);
}

void Disp_init() {
  Disp.start();
  Disp.clear();
  Disp.setBrightness(brightness);
  Serial.println("Setup dmd selesai");

  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(refresh);
  timer0_write(ESP.getCycleCount() + 80000);
  interrupts();
}

void AP_init() {
  WiFi.mode(WIFI_AP);
  //WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid);
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // Pastikan WiFi tidak sleep

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  /*
  ArduinoOTA.setHostname(host);
   ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  //ArduinoOTA.begin();
  server.on("/setPanel", handleSetTime);
  server.begin();
  */
  
  Serial.println("Server dimulai.");  
}


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
