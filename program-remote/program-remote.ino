#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <DMDESP.h>
#include <ESP_EEPROM.h>

// Ukuran EEPROM (pastikan cukup untuk semua data)
#define EEPROM_SIZE 50

//SETUP DMD
#define DISPLAYS_WIDE 2
#define DISPLAYS_HIGH 2

DMDESP  Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  // Jumlah Panel P10 yang digunakan (KOLOM,BARIS)

// Pengaturan hotspot WiFi dari ESP8266
char ssid[20]     = "JAM_PANEL_MUSHOLLAH";
char password[20] = "00000000";
const char* host = "OTA-PANEL";

ESP8266WebServer server(80);

IPAddress local_IP(192, 168, 2, 1);      // IP Address untuk AP
IPAddress gateway(192, 168, 2, 1);       // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask

void AP_init() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ssid);
    WiFi.setSleepMode(WIFI_NONE_SLEEP); // Pastikan WiFi tidak sleep
  
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    
    server.on("/setTime", handleSetTime);
    server.begin();

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
    ArduinoOTA.begin();
    
    Serial.println("Server dimulai.");  
  }

void handleSetTime() {
  Serial.println("hansle run");
  
  Buzzer(1);
  
  if (server.hasArg("status")) {//status
    server.send(200, "text/plain", "CONNECTED");
  }
  
  if (server.hasArg("Tm")) {//jam
    String setJam = server.arg("Tm"); 
    Serial.println("Tm=" + setJam);
    
   // server.send(200, "text/plain", "Settingan jam berhasil diupdate");
  } 

  if (server.hasArg("text")) {
    String setText = server.arg("text"); 
    Serial.println("text=" + setText);

    //saveStringToEEPROM(66, dynamicText);
   // server.send(200, "text/plain", "Settingan nama berhasil diupdate");
  }
  
 if (server.hasArg("Br")) {
   uint8_t input  = server.arg("Br").toInt(); 
   uint8_t brightness = map(input,0,100,10,255);
   Serial.println("Br="+String(brightness));

   server.send(200, "text/plain", "Kecerahan berhasil diupdate");
 }
 
 if (server.hasArg("Sptx")) {
   uint8_t input = server.arg("Sptx").toInt(); // Atur kecepatan text
   uint8_t speedText =  map(input,0,100,10,80);

   Serial.println("Sptx="+String(speedText));
   //EEPROM.put(8, speedText);
   server.send(200, "text/plain", "Kecepatan nama berhasil diupdate");
 }

 if (server.hasArg("Spdt")) {
   uint8_t input = server.arg("Spdt").toInt(); // Atur kecepatan text
   uint8_t speedDate =  map(input,0,100,10,80);
   Serial.println("Spdt="+String(speedDate));
   //EEPROM.put(8, speedDate);
   server.send(200, "text/plain", "Kecepatan tanggal berhasil diupdate");
 }

 if (server.hasArg("Lk")) {
   String data = server.arg("Lk"); 
   Serial.print("Lk="+String(data));
   //parsingData(data);
   //flag=1;
 }

 if (server.hasArg("Iq")) { //iqomah
   uint8_t data = server.arg("Iq").toInt(); 
  //   // Mencari posisi tanda "-"
  //  int separatorIndex = data.indexOf('-');

  //  // Memisahkan angka pertama
  //  int indexSholat = data.substring(0, separatorIndex).toInt();

  //  // Memisahkan angka kedua
  //  int indexKoreksi = data.substring(separatorIndex + 1).toInt();  
  //  iqomah[indexSholat]=indexKoreksi;
   Serial.println("Iq=" + String(data));
 }           

 if (server.hasArg("Dy")) { //display off
   uint8_t data = server.arg("Dy").toInt(); ; 
    // Mencari posisi tanda "-"
  //  int separatorIndex = data.indexOf('-');

  //  // Memisahkan angka pertama
  //  int indexSholat = data.substring(0, separatorIndex).toInt();

  //  // Memisahkan angka kedua
  //  int indexKoreksi = data.substring(separatorIndex + 1).toInt();  
  //  displayBlink[indexSholat]=indexKoreksi;
   Serial.println("Dy=" + String(data));
 }        

 if (server.hasArg("Kr")) { //koreksi jadwal
   uint8_t data = server.arg("Kr").toInt(); ; 
  //   // Mencari posisi tanda "-"
  //  int separatorIndex = data.indexOf('-');

  //  // Memisahkan angka pertama
  //  int indexSholat = data.substring(0, separatorIndex).toInt();

  //  // Memisahkan angka kedua
  //  int indexKoreksi = data.substring(separatorIndex + 1).toInt();  
  //  dataIhty[indexSholat]=indexKoreksi;
   Serial.println("Kr=" + String(data));
 }        

 
 
 if (server.hasArg("Bzr")) {
   uint8_t stateBuzzer = server.arg("Bzr").toInt(); // Atur status buzzer
   Serial.println("Bzr="+String(stateBuzzer));
   //EEPROM.put(52, stateBuzzer);
   //server.send(200, "text/plain", (stateBuzzer)?"Suara Diaktifkan":"Suara Dimatikan");
 }

 if (server.hasArg("status")) {
   server.send(200, "text/plain", "CONNECTED");
 }
///////////////
 if (server.hasArg("newPassword")) {
   String newPassword = server.arg("newPassword");
   if(newPassword.length()==8){
     Serial.println("newPassword="+newPassword);
     newPassword.toCharArray(password, newPassword.length() + 1); // Set password baru
     //saveStringToEEPROM(56, password); // Simpan password AP
     server.send(200, "text/plain", "Password WiFi diupdate");
   }else{ Buzzer(2); Serial.println("panjang password melebihi 8 karakter"); }
  } 
   //write the data to EEPROM
  boolean ok1 = EEPROM.commit();
  Serial.println((ok1) ? "First commit OK" : "Commit failed");
  
}


void setup() {
  Serial.begin(115200);
  AP_init();
}

void loop() {
  server.handleClient(); // Menangani permintaan dari MIT App Inventor

}
void Buzzer(uint8_t state)
  {
    //if(!stateBuzzer) return;
    
    // switch(state){
    //   case 0 :
    //     digitalWrite(BUZZ,HIGH);
    //   break;
    //   case 1 :
    //     digitalWrite(BUZZ,LOW);
    //   break;
    //   case 2 :
    //     for(int i = 0; i < 2; i++){
    //       digitalWrite(BUZZ,LOW);
    //       delay(80);
    //       digitalWrite(BUZZ,HIGH);
    //       delay(80);
    //     }
    //   break;
    // };
  }
