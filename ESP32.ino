//Incluímos la librería para decodificar señales IR
#include <IRremote.h>
//Pin 16 conectado a receptor infrarrojo:
IRrecv irrecv(15);
decode_results results;

int number = 0;
String stateA = "";
String stateP = "";


//Incluimos las librerias para establecer la conexión con la Firebase
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper funcitions.
#include "addons/RTDBHelper.h"

// Inser your network credentials
#define WIFI_SSID "Vico"
#define WIFI_PASSWORD "facil#34"

//#define API_KEY "AIzaSyBMHJxkLAUbJneLDtkdPXEZMq45TpXAdBE"//AIzaSyAjjTHMIV0y394tayvijhU-aVVcKdkIZxU
//#define DATABASE_URL "https://sp32-5f852-default-rtdb.firebaseio.com"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBMHJxkLAUbJneLDtkdPXEZMq45TpXAdBE"//AIzaSyAjjTHMIV0y394tayvijhU-aVVcKdkIZxU

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://sp32-5f852-default-rtdb.firebaseio.com"

//Define Firebase Data Object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;


void setup() {
  // put your setup code here, to run once:
  irrecv.enableIRIn();
  // Iniciamos comunicación serial
  Serial.begin(9600);
  Serial2.begin(9600); 
  //Pin Buzzer
  pinMode(4, OUTPUT);
  //Pin led rojo (warning)
  pinMode(19, OUTPUT);
  //Pin led verde (dato valido)
  pinMode(21, OUTPUT);
  //Pin led azul(internet)
  pinMode(22, OUTPUT);
  //Pin led base de datos
  pinMode(23, OUTPUT);


  //Se establece la conexión

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(22, HIGH);
    delay(250);
    digitalWrite(22, LOW);
    delay(250);
  }
  digitalWrite(22, HIGH);

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    digitalWrite(23, HIGH);
    signupOK = true;
  }
  else {
    digitalWrite(19, HIGH);
  }

  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // attach the channel to the GPIO to be controlled
  // put your setup code here, to run once:

  // ubidots.setDebud(true); // uncomment this to make debug messages available
  // PIR

}

void sendDataBase(String instruction) {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setString(&fbdo, "test/mp3", instruction)) {
      //Hacemos uso del buzzer para notificar que el dato se mando a la firebase
      digitalWrite(4, HIGH);
      delay(250);
      digitalWrite(4, LOW);
    }
    else {
      digitalWrite(4, HIGH);
      delay(1000);
      digitalWrite(4, LOW);

    }
  }
}

void invalidData() {
  digitalWrite(19, HIGH);
  delay(500);
  digitalWrite(19, LOW);
}

void validData() {
  digitalWrite(21, HIGH);
  delay(500);
  digitalWrite(21, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (irrecv.decode(&results)) {
    irrecv.resume();
    //PREV
    if ( results.value == 0xFEA25D) {
      validData();
      sendDataBase("Prev");
    }
    //NEXT
    else if ( results.value == 0xFE22DD ) {
      validData();
      sendDataBase("Next");
    }
    //PLAY - PAUSE
    else if ( results.value == 0xFE52AD ) {
      validData();
      sendDataBase("PP");
    }
    //STOP:
    else if ( results.value == 0xFED22D ) {
      validData();
      sendDataBase("Stop");
    }

    //Numbers:
    //0
    else if ( results.value == 0xFE00FF ) {
      number = number * 10 + 0;
      validData();
    }
    //1
    else if ( results.value == 0xFE807F ) {
      number = number * 10 + 1;
      validData();
    }
    //2
    else if ( results.value == 0xFE40BF ) {
      number = number * 10 + 2;
      validData();
    }
    //3
    else if ( results.value == 0xFEC03F ) {
      number = number * 10 + 3;
      validData();
    }
    //4
    else if ( results.value == 0xFE20DF ) {
      number = number * 10 + 4;
      validData();
    }
    //5
    else if ( results.value == 0xFEA05F ) {
      number = number * 10 + 5;
      validData();
    }
    //6
    else if ( results.value == 0xFE609F ) {
      number = number * 10 + 6;
      validData();
    }
    //7
    else if ( results.value == 0xFEE01F ) {
      number = number * 10 + 7;
      validData();
    }
    //8
    else if ( results.value == 0xFE10EF ) {
      number = number * 10 + 8;
      validData();
    }
    //9
    else if ( results.value == 0xFE906F ) {
      number = number * 10 + 9;
      validData();
    }
    else if ( results.value == 0xFE08F7) {
      if (number > 0 and number <= 100) {
        String numberS = String(number);
        validData();
        sendDataBase(numberS);
        number = 0;
      }
      else {
        number = 0;
        invalidData();
      }
    }
    else if (results.value = 0xFFFFFFFF) {
      number = number;
    }
    else {
      invalidData();
    }
  }

  if (Firebase.RTDB.getString(&fbdo, "test/dir")) {
    if (fbdo.dataType() == "string") {
      String stateA = fbdo.stringData();
      if (stateA != stateP) {
        Serial.print(stateA);
      }
      stateP = stateA;
    }
  }

  if (Serial2.available()) {
    int sensor = Serial2.read();
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      // Write an Int number on the database path test/int
      if (Firebase.RTDB.setInt(&fbdo, "test/sensor", sensor)) {
        //Hacemos uso del buzzer para notificar que el dato se mando a la firebas
      }
    }
  }
}
