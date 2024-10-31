#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
SoftwareSerial sim808(7, 8); // RX, TX

String server = "http://20.8.56.118/api/passager"; // Change to your server URL

void setup() {
  Serial.begin(9600); // Initialize serial communications
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522

  sim808.begin(9600); // Initialize SIM808 serial communications

  // Initialize SIM808
  sendCommand("AT");
  sendCommand("AT+CSQ");
  sendCommand("AT+CREG?");
  sendCommand("AT+CGATT?");
  sendCommand("AT+CSTT=\"vodanet\",\"\",\"\""); // Set APN
  sendCommand("AT+CIICR"); // Bring up wireless connection
  sendCommand("AT+CIFSR"); // Get IP address
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  String rfidData = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidData += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    rfidData += String(mfrc522.uid.uidByte[i], HEX);
  }
  rfidData.toUpperCase();
  Serial.print("IUD CARTE : ");
  Serial.println(rfidData);
  
  // Send RFID data to the server
  sendDataToServer(rfidData);

  delay(5000); // Delay to avoid flooding the server
}

void sendDataToServer(String rfid) {
  String postData = "idrfid=" + rfid + "&date_expiration_rfid=2024-12-31&id_agent=2&id_bus=1";
  Serial.println(postData);
  
  sendCommand("AT+HTTPTERM"); // Terminate any previous HTTP session
  delay(1000); // Wait for the command to execute
  sendCommand("AT+HTTPINIT");
  delay(1000); // Wait for the command to execute
  sendCommand("AT+HTTPPARA=\"CID\",1");
  delay(1000); // Wait for the command to execute
  sendCommand("AT+HTTPPARA=\"URL\",\"" + server + "\"");
  delay(1000); // Wait for the command to execute
  sendCommand("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
  delay(1000); // Wait for the command to execute
  sendCommand("AT+HTTPDATA=" + String(postData.length()) + ",10000");
  delay(2000); // Give it time to be ready
  sim808.println(postData);
  delay(10000); // Give it time to be ready
  sendCommand("AT+HTTPACTION=1");
  delay(5000); // Wait for the response
  sendCommand("AT+HTTPREAD");
  delay(1000); // Wait for the command to execute
  sendCommand("AT+HTTPTERM");
}

void sendCommand(String command) {
  sim808.println(command);
  delay(1000); // Wait for the command to execute
  while (sim808.available()) {
    Serial.write(sim808.read());
  }
}
