#include <SoftwareSerial.h>

// Définir les pins pour la communication série
const int sim808TxPin = 7; // TX du module SIM808
const int sim808RxPin = 8; // RX du module SIM808

// Créer un objet SoftwareSerial pour le module SIM808
SoftwareSerial sim808(sim808RxPin, sim808TxPin);

// Fonction pour envoyer des commandes AT et attendre une réponse spécifique
bool sendATCommand(String command, String expected_response, unsigned long timeout = 5000) {
  sim808.println(command);
  unsigned long start_time = millis();
  String response = "";
  while (millis() - start_time < timeout) {
    while (sim808.available()) {
      char c = sim808.read();
      response += c;
    }
    if (response.indexOf(expected_response) != -1) {
      return true;
    }
  }
  return false;
}

void setup() {
  // Initialiser la communication série
  Serial.begin(9600); // Communication série avec le moniteur série
  sim808.begin(9600); // Communication série avec le module SIM808

  Serial.println("Initialisation du module SIM808...");
  
  // Configurer le module SIM808
  if (sendATCommand("AT", "OK")) {
    Serial.println("Module SIM808 prêt");
  } else {
    Serial.println("Échec de la communication avec le module SIM808");
    return;
  }

  if (sendATCommand("AT+CGATT=1", "OK")) {
    Serial.println("Attaché au réseau GPRS");
  } else {
    Serial.println("Échec de l'attachement au réseau GPRS");
    return;
  }

  if (sendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK")) {
    Serial.println("Type de connexion défini");
  } else {
    Serial.println("Échec de la définition du type de connexion");
    return;
  }

  if (sendATCommand("AT+SAPBR=3,1,\"APN\",\"your_apn\"", "OK")) {
    Serial.println("APN défini");
  } else {
    Serial.println("Échec de la définition de l'APN");
    return;
  }

  if (sendATCommand("AT+SAPBR=1,1", "OK", 10000)) { // Augmenter le timeout car l'ouverture peut prendre plus de temps
    Serial.println("Port GPRS ouvert");
  } else {
    Serial.println("Échec de l'ouverture du port GPRS");
    return;
  }

  if (sendATCommand("AT+SAPBR=2,1", "OK")) {
    Serial.println("État du port GPRS vérifié");
  } else {
    Serial.println("Échec de la vérification de l'état du port GPRS");
    return;
  }

  Serial.println("Configuration terminée.");
}

void loop() {
  // Données GPS simulées (remplacez par vos données réelles)
  float latitude = -5.936944;
  float longitude = 12.39302;
  float vitesse = 30.0;

  // Créer la requête GET
  String url = "http://www.google.com";

  // Envoyer la requête GET
  sendHttpRequest(url);

  // Attendre avant d'envoyer une nouvelle requête
  delay(10000); // Attendre 10 secondes avant de ré-envoyer (à ajuster selon vos besoins)
}

void sendHttpRequest(String url) {
  if (!sendATCommand("AT+HTTPTERM", "OK")) {
    Serial.println("Échec de la terminaison HTTP précédente");
  }

  if (!sendATCommand("AT+HTTPINIT", "OK")) {
    Serial.println("Échec de l'initialisation HTTP");
    return;
  }

  if (!sendATCommand("AT+HTTPPARA=\"CID\",1", "OK")) {
    Serial.println("Échec de la définition de l'identifiant de connexion");
    return;
  }

  if (!sendATCommand("AT+HTTPPARA=\"URL\",\"" + url + "\"", "OK")) {
    Serial.println("Échec de la définition de l'URL");
    return;
  }

  if (!sendATCommand("AT+HTTPACTION=0", "OK", 10000)) { // Augmenter le timeout car l'action peut prendre plus de temps
    Serial.println("Échec de la requête GET");
    return;
  }

  // Lire la réponse HTTP
  Serial.println("Lecture de la réponse HTTP...");
  unsigned long start_time = millis();
  while (millis() - start_time < 10000) {
    while (sim808.available()) {
      char c = sim808.read();
      Serial.print(c);
    }
  }

  if (!sendATCommand("AT+HTTPTERM", "OK")) {
    Serial.println("Échec de la terminaison HTTP");
  }
}
