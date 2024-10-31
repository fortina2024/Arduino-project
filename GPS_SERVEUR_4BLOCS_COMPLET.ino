#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <Arduino.h>

// RFID pins
#define SS_PIN 10
#define RST_PIN 9

// Create instances for RFID and SIM808
MFRC522 mfrc522(SS_PIN, RST_PIN);
SoftwareSerial sim808(7, 8); // RX, TX for SIM808

// Configuration pour l'écran LCD
const int rs = A0, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int buzzerPin = A1;

String server = "http://api.mumipoc.komatel.com/api/passagers"; //"https://mumipoc.komatel.com/api/passagers";

// Tableau des blocs à lire
byte blocksToRead[] = {5, 6, 8, 9};

// Tableau pour stocker les résultats des blocs
String blockResults[4];
// Tableau pour stocker les titres des blocs
String blockTitres[] = {"Matricule Agent:", "ID Agent:", "ID Bus:", "Date de validite:"};

// Clé d'authentification par défaut (toutes les valeurs sont 0xFF)
MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(9600); // Serial monitor
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  sim808.begin(9600); // Init SIM808
  // Initialisation de l'écran LCD
  lcd.begin(16, 2); // Initialisation de l'écran LCD 16x2
  lcd.print("Initialisation");
  lcd.setCursor(0, 1);
  lcd.print("en cours...");

  // Initialize SIM808
  initializeSIM808();
  lcd.clear();
  lcd.print("VOTRE CARTE:");
  lcd.setCursor(0, 1);

  // Initialiser la clé à 0xFF
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  digitalWrite(buzzerPin, LOW);
  String rfidData = readRFID();
  String gpsData;

  if (rfidData != "") {
    gpsData = readGPS();
    sendDataToServer(rfidData, gpsData);
  } else {
    Serial.println("Aucune carte RFID détectée");
  }
  mfrc522.PCD_Init(); // Réinitialisation du lecteur

  delay(1000); // Delay to avoid flooding the server
}

String readRFID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return "";
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return "";
  }
  //tone(buzzerPin, 1000, 500);
   
  String rfidData = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidData += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    rfidData += String(mfrc522.uid.uidByte[i], HEX);
  }
  rfidData.toUpperCase();
  Serial.print("UID CARTE : ");
  Serial.println(rfidData);
  lcd.clear();
  lcd.print("UID CARTE:");
  lcd.setCursor(0, 1);
  lcd.print(rfidData);
  //delay(2000);
   
  // Lire les blocs spécifiés dans la boucle
  /*byte blockData[16]; // Tableau pour stocker les données des blocs
  byte size; // Taille des données lues
  MFRC522::StatusCode status;

  for (int i = 0; i < sizeof(blocksToRead); i++) {
    byte blockNumber = blocksToRead[i];
    
    // Authentifier avec la clé A
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print("Authentication failed for block ");
      Serial.println(blockNumber);
      lcd.clear();
      lcd.print("Auth failed block ");
      lcd.print(blockNumber);
      delay(2000);
      continue;
    }

    // Lire le bloc
    status = mfrc522.MIFARE_Read(blockNumber, blockData, &size);
    lcd.clear(); // Effacer l'écran LCD pour une nouvelle lecture
    lcd.setCursor(0, 0);

    if (status == MFRC522::STATUS_OK) {
      // Convertir les données du bloc en String
      String blockContent = "";
      for (byte j = 0; j < size; j++) {
        blockContent += (char)blockData[j]; // Ajouter chaque caractère du bloc
      }
      blockResults[i] = blockContent; // Sauvegarder le résultat dans le tableau 
    } else {
      Serial.print("Erreur Bloc ");
      Serial.print(blockNumber);
      lcd.print("Erreur Bloc ");
      lcd.print(blockNumber);
    }
    delay(1000);
  }
  
  lcd.clear(); // Effacer l'écran LCD pour une nouvelle lecture
  lcd.print("Envoi de donnees");
  lcd.setCursor(0, 1);
  lcd.print("en cours...");
  tone(buzzerPin, 3000, 500);*/
  return rfidData;
}

String readGPS() {
  String gps_Data = "";

  sim808.println("AT+CGNSINF");
  delay(3000); // Wait for the command to execute
  while (sim808.available()) {
    char c = sim808.read();
    gps_Data += c;
  }
  Serial.print("GPS Data : ");
  Serial.println(gps_Data);

  String longitude, latitude, datetime, reponseGPS = "";
  int index = gps_Data.indexOf(":");
  if (index != -1) {
    gps_Data = gps_Data.substring(index + 1);
    // Split the GPS data to extract datetime, latitude, and longitude
    int startIndex = gps_Data.indexOf(",") + 1;
    for (int i = 0; i < 1; i++) {
      startIndex = gps_Data.indexOf(",", startIndex) + 1;
    }
    int endIndex = gps_Data.indexOf(",", startIndex);
    datetime = gps_Data.substring(startIndex, endIndex);
    startIndex = endIndex + 1;
    endIndex = gps_Data.indexOf(",", startIndex);
    latitude = gps_Data.substring(startIndex, endIndex);
    startIndex = endIndex + 1;
    endIndex = gps_Data.indexOf(",", startIndex);
    longitude = gps_Data.substring(startIndex, endIndex);

    // Format the datetime string
    String formattedDatetime = datetime.substring(0, 4) + "-" + datetime.substring(4, 6) + "-" + datetime.substring(6, 8) + " " + datetime.substring(8, 10) + ":" + datetime.substring(10, 12) + ":" + datetime.substring(12, 14);

    Serial.print("DateEmbarquement: ");
    Serial.println(formattedDatetime);
    Serial.print("Latitude: ");
    Serial.println(latitude);
    Serial.print("Longitude: ");
    Serial.println(longitude);

    reponseGPS = "date_embarquement=" + formattedDatetime + "&lat=" + latitude + "&lon=" + longitude;

  }else{
    reponseGPS = "date_embarquement=&lat=&lon=";
  }
  return reponseGPS;
}

void initializeSIM808() {
  sendCommand("AT");
  delay(2000);
  sendCommand("AT+CSQ");
  delay(2000);
  sendCommand("AT+CREG?");
  delay(2000);
  sendCommand("AT+CGATT?");
  delay(2000);
  sendCommand("AT+CSTT=\"vodanet\",\"\",\"\""); // Set APN
  delay(2000);
  sendCommand("AT+CIICR"); // Bring up wireless connection
  delay(5000); // Wait for connection
  sendCommand("AT+CIFSR"); // Get IP address
  delay(2000);
  sendCommand("AT+CGNSPWR=1"); // Turn on GPS
  delay(2000);
}

void sendDataToServer(String rfid, String gps) {

  String postData = "uid_carte="+rfid+"&" + gps + "&id_agent=1&id_bus=2";
  Serial.println(postData);
  delay(1000);
  sendCommand("AT+HTTPTERM"); // Terminate any previous HTTP session
  delay(1000); // Wait for the command to execute
  sendCommand("AT+HTTPINIT");
  delay(1000);
  sendCommand("AT+HTTPPARA=\"CID\",1");
  delay(1000); // Wait for the command to execute
  sendCommand("AT+HTTPPARA=\"URL\",\"" + server + "\"");
  delay(3000); // Wait for the command to execute
  sendCommand("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
  delay(1000); // Wait for the command to execute
  /*sendCommand("AT+HTTPSSL=1");
  delay(1000); */ // Wait for the command to execute
  sendCommand("AT+HTTPDATA=" + String(postData.length()) + ",10000");
  delay(2000); // Give it time to be ready
  sim808.print(postData);
  delay(5000); // Give it time to send the data
  sendCommand("AT+HTTPACTION=1");
  delay(3000); 

  while (sim808.available()) {
    String response = sim808.readString();
    Serial.println(response);
  }

  sendCommand("AT+HTTPTERM");
  lcd.clear(); // Effacer l'écran LCD pour une nouvelle lecture
  lcd.print("Terminer");
  delay(3000);
  lcd.clear();
  lcd.print("VOTRE CARTE:");
  lcd.setCursor(0, 1);
}

void sendCommand(String command) {
  sim808.println(command);
  delay(2000); // Wait for the command to execute
  while (sim808.available()) {
    Serial.write(sim808.read());
  }
}
