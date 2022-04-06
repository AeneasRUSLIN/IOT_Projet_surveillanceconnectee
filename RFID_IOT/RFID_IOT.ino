#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------
#define RST_PIN  D3
#define SS_PIN   D4
#define BUZZER   D2     
//-----------------------------------------
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  
/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];
//-----------------------------------------
String card_holder_name;
const String sheet_url = "https://script.google.com/macros/s/AKfycbyHFGk0PHcxqFmBRMBD_O6_JbHDj3dP6UwSV5YvHr2V9sOT8ScuJ3dS1H8f1A77Jpml/exec?name=";
//-----------------------------------------
// Fingerprint for demo URL, expires on ‎Monday, ‎May ‎2, ‎2022 7:20:58 AM, needs to be updated well before this date
// 0x9a, 0x71, 0xde, 0xe7, 0x1a, 0xb2, 0x25, 0xca, 0xb4, 0xf2, 0x36, 0x49, 0xab, 0xce, 0xf6, 0x25, 0x62, 0x04, 0xe4, 0x3c
const uint8_t fingerprint[20] = {0x9a, 0x71, 0xde, 0xe7, 0x1a, 0xb2, 0x25, 0xca, 0xb4, 0xf2, 0x36, 0x49, 0xab, 0xce, 0xf6, 0x25, 0x62, 0x04, 0xe4, 0x3c};
//-----------------------------------------
#define WIFI_SSID "Ananas"
#define WIFI_PASSWORD "catbread"
//-----------------------------------------

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
String tag;
int attente = 2000;
int valid = 0;
int i,j;
int UID_receive[4];
char* UID_c;
char* identite;


#define NB_max 10 
int UID_ref[NB_max][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
char* liste_identite[NB_max] = {"User1","User2","User3","User4","User5","User6","User7","User8","User9","User10"};
int admin[4] = {162,80,234,26};
int NB_id = 0;

int UID_equals(int* a, int* b){
  for(int k = 0; k < 4; k++){
    if (a[k] != b[k]) return 0;
  }
  return 1;
}

#define LED_BLEUE D0
#define LED_VERTE D1
#define LED_ROUGE D8

/****************************************************************************************************
 * setup() function
 ****************************************************************************************************/
void setup()
{
  pinMode(LED_BLEUE, OUTPUT);
  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);
  digitalWrite(LED_BLEUE, LOW);
  digitalWrite(LED_VERTE, LOW);
  digitalWrite(LED_ROUGE, LOW);
  //--------------------------------------------------
  /* Initialize serial communications with the PC */
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  //--------------------------------------------------
  //WiFi Connectivity
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //--------------------------------------------------
  /* Set BUZZER as OUTPUT */
  pinMode(BUZZER, OUTPUT);
  //--------------------------------------------------
  /* Initialize SPI bus */
  SPI.begin();
  rfid.PCD_Init(); // Init MFRC522
  //--------------------------------------------------
}




/****************************************************************************************************
 * loop() function
 ****************************************************************************************************/
 void loop()
{
  //Serial.printf(".\n");
  //Serial.printf("###################\n");
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  //Serial.printf("........\n");
  if (rfid.PICC_ReadCardSerial()) {
    for (i = 0; i < 4; i++) {
      UID_receive[i] = rfid.uid.uidByte[i];
    }

    if(UID_equals(admin, UID_receive)){
      if (NB_id == NB_max - 1){
        Serial.printf("Nombre maximal d'utilisateurs atteint !\n");
      }else{
        digitalWrite(LED_VERTE, HIGH);
        Serial.printf("Patientez 5s\n");
        delay(1000);
        Serial.printf("Patientez 4s\n");
        delay(1000);
        Serial.printf("Patientez 3s\n");
        delay(1000);
        Serial.printf("Patientez 2s\n");
        delay(1000);
        Serial.printf("Patientez 1s\n");
        delay(1000);
        digitalWrite(LED_VERTE, LOW);
        digitalWrite(LED_BLEUE, HIGH);
        Serial.printf("Scannez le nouvelle utilisateur.\n");
        while(! rfid.PICC_IsNewCardPresent());
        if (rfid.PICC_ReadCardSerial()) {
          for (i = 0; i < 4; i++) {
            UID_receive[i] = rfid.uid.uidByte[i];
          }
        }
        for (i = 0; i < 4; i++) {
          UID_ref[NB_id][i] = UID_receive[i];
        }
        NB_id++;
        Serial.printf("La carte est enregistrée !\n");
        
        delay(1000);
        digitalWrite(LED_BLEUE, LOW);
      }
    }else{
      //Serial.printf("..........................\n");
      for(i=0;i<NB_id;i++){
        for (j=0;j<4;j++){
          if (UID_receive[j] != UID_ref[i][j]){
            j = 10;
          }
          if (j == 3){
            valid = 1;
            identite = liste_identite[i];
          }
        }
      }

      if (valid == 1){
        //sprintf(UID_c,"%d%d%d%d%d",UID[0],UID[1],UID[2],UID[3],UID[4]);
        digitalWrite(LED_VERTE, HIGH);
        //--------------------------------------------------
        digitalWrite(BUZZER, HIGH);
        delay(200);
        digitalWrite(BUZZER, LOW);
        delay(200);
        digitalWrite(BUZZER, HIGH);
        delay(200);
        digitalWrite(BUZZER, LOW);
        //--------------------------------------------------
  
        //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
        if (WiFi.status() == WL_CONNECTED) {
          //-------------------------------------------------------------------------------
          std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
          //-------------------------------------------------------------------------------
          client->setFingerprint(fingerprint);
          // Or, if you want to ignore the SSL certificate
          //then use the following line instead:
          // client->setInsecure();
          //-----------------------------------------------------------------
          card_holder_name = sheet_url + identite;
          card_holder_name.trim();
          Serial.println(card_holder_name);
          //-----------------------------------------------------------------
          HTTPClient https;
          Serial.print(F("[HTTPS] begin...\n"));
          //-----------------------------------------------------------------

          //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
          if (https.begin(*client, (String)card_holder_name)){
            //-----------------------------------------------------------------
            // HTTP
            Serial.print(F("[HTTPS] GET...\n"));
            // start connection and send HTTP header
            int httpCode = https.GET();
            //-----------------------------------------------------------------
            // httpCode will be negative on error
            if (httpCode > 0) {
              // HTTP header has been send and Server response header has been handled
              Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
              // file found at server
            }
            //-----------------------------------------------------------------
            else 
            {Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());}
            //-----------------------------------------------------------------
            https.end();
            delay(1000);
          }
          //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
          else {
            Serial.printf("[HTTPS} Unable to connect\n");
          }
          //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
        }
        digitalWrite(LED_VERTE, LOW);
        //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
      }else{
        digitalWrite(LED_ROUGE, HIGH);
        Serial.printf("Personne non reconnue !\n");
        delay(2000);
        digitalWrite(LED_ROUGE, LOW);
      }
    }
  }
  valid = 0;
  i = 0;
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  //Serial.printf("........................................................\n");
}
