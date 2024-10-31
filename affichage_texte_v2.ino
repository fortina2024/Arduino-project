#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define RST_PIN 9    // Pin de reset du module RFID RC522
#define SS_PIN 10    // Pin SDA du module RFID RC522

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Création de l'instance MFRC522

const int rs = 7, en = 8, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // Création de l'instance LiquidCrystal

bool cardDetected = false;  // Variable pour suivre l'état de la carte

void setup() {
  Serial.begin(9600);  // Initialisation de la communication série
  SPI.begin();         // Initialisation du bus SPI
  mfrc522.PCD_Init();  // Initialisation du module RC522
  lcd.begin(16, 2);    // Initialisation de l'affichage LCD 16x2
  lcd.clear();         // Effacement de l'écran
   // lcd.scrollDisplayLeft();
  lcd.print("VOTRE CARTE SVP:");

}

void loop() {
  // Chercher une nouvelle carte présente
  if (!cardDetected && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    cardDetected = true; // Marquer la carte comme détectée

    // Afficher l'ID de la carte sur le moniteur série
    Serial.print("Card UID: ");
    String cardID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      cardID += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      cardID += String(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();

    // Afficher l'ID de la carte sur l'écran LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Card UID:");
    lcd.setCursor(0, 1);
    lcd.print(cardID);
    delay(2000);

    // Lire et afficher les blocs des secteurs variables
    /*int secteurs[] = {1, 2}; // Exemple de secteurs variables
    int blocs[] = {4, 5, 6, 7,8,9}; // Blocs à lire dans chaque secteur
    for (byte i = 0; i < sizeof(secteurs)/sizeof(secteurs[0]); i++) {
      int secteur = secteurs[i];
      for (byte j = 0; j < sizeof(blocs)/sizeof(blocs[0]); j++) {
        int bloc = blocs[j];
        readBlock(secteur, bloc);
      }
    }*/
    int rang_titre=0;
    int listes_blocs[] = {5,6,8,9};
      for (byte block = 0; block < 4; block++) {
        readBlock(listes_blocs[block], rang_titre);
        rang_titre++;
      }

    lcd.clear();         // Effacement de l'écran
    lcd.print("TERMINER.");
    delay(2000);
    lcd.clear();
    lcd.print("VOTRE CARTE SVP:");
    mfrc522.PICC_HaltA();
    cardDetected = false; // Réinitialiser l'état de la carte
  }
}

void readBlock(byte bloc, int rangs) {
  String titres[] = {"MATRICULE:", "ID SERIE RFID:", "ID AGENT:", "DATE VALIDITE:"};
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Authentifier le secteur
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF; // Clé par défaut

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloc, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentification échouée: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Lire le bloc
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(bloc, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Lecture échouée: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Afficher le bloc sur le LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(titres[rangs]); // Vous pouvez personnaliser les titres comme souhaité

  lcd.setCursor(0, 1);
  for (byte i = 0; i < 16; i++) {
    if (isPrintable(buffer[i])) {
      lcd.print((char)buffer[i]);
    } else {
      lcd.print('.');
    }
  }

  delay(2000); 
}

bool isPrintable(char c) {
  return c >= 32 && c <= 126;
}
