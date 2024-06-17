#include <EEPROM.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ArduinoJson.h>
#include <Arduino.h>


#include <WiFi.h>
#include <WiFiServer.h>
#include <WifiMulti.h>
#include <HTTPClient.h>

const char *ssid = "ESP32";
const char *password = "password";
const int serverPort = port;

WiFiServer server(serverPort);
WiFiMulti wifiMulti;



SPIClass hspi = SPIClass(HSPI);

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char *path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}
void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  // Parse the JSON data from the file
  //DeserializationError error = deserializeJson(jsonDocument, file);

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println();
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void removeDir(fs::FS &fs, const char *path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}
void spi_init() {
  hspi.begin(SCK, MISO, MOSI, CS);

  delay(2000);
  if (!SD.begin(CS, hspi, 1000000)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}





// for sending data from app to board ----------------------------------------------------------------------------------------------------------------------------
WiFiClient client;
void stringtoint(String Appdata) {
  int dataLength = Appdata.length();
  int *dataArray1 = new int[dataLength];
  int *dataArray2 = new int[dataLength];

  for (int i = 0; i < dataLength; i++) {
    char currentChar = Appdata[i];
    if (currentChar >= '0' && currentChar <= '9') {
      dataArray1[i] = currentChar - '0';
    }
  }
  for (int i = 0; i < dataLength; i++) {
    Serial.println(dataArray1[i]);
  }

  if (dataArray1[0] == '1' || dataArray1[0] == 1) {  // switch operation - 1311(1-command , 3-board , 1-switch , 1-on/off)
    dataArray2[0] = dataArray1[0];
    dataArray2[1] = (dataArray1[2] * 4) + dataArray1[3];
    dataArray2[2] = dataArray1[1];

    Serial.println("Data stored in arr2:");
    for (int i = 0; i < arraySize; i++) {
      Serial.println(dataArray2[i]);
    }
    device_board_data(1, dataArray2[0], dataArray2[1], dataArray2[2]);
  }
  if (dataArray1[0] == '6' || dataArray1[0] == 6) {  // fan speed operation - 6311(6-command , 3-board , 1/5-fanspeed , 1-on/off)
    dataArray2[0] = dataArray1[0];
    dataArray2[1] = (dataArray1[2] + 60) + dataArray1[3];
    dataArray2[2] = dataArray1[1];

    Serial.println("Data stored in arr2:");
    for (int i = 0; i < arraySize; i++) {
      Serial.println(dataArray2[i]);
    }
    device_board_data(1, dataArray2[0], dataArray2[1], dataArray2[2]);
  }

  
  delete[] dataArray1;
  delete[] dataArray2;
}


// for saving data coming from api to file ---------------------------------------------------------------------------------------------------------------
void saveJSONToFile(String filename, String payloadAPI) {
  String path = "/" + filename;
  File jsonFile = SD.open(path.c_str(), FILE_WRITE);
  Serial.println("saved flat data");
  if (!jsonFile) {
    Serial.println("Failed to open file for writing");
    return;
  }
  StaticJsonDocument<512> doc;
  deserializeJson(doc, payloadAPI);
  serializeJson(doc, jsonFile);
  jsonFile.close();
  Serial.println("Data saved to file: " + filename);
}

// for calling api ------------------------------------------------------------------------------------------------------------------------------------------
HTTPClient http;
int httpCode;

void wifiWorking(String url, int ProtType, int urlcode, String tempData = "", String filename = "") {
  Serial.print("[HTTP] begin...\n");
  http.begin(url);

  if (ProtType == 0) {
    Serial.print("[HTTP] GET...\n");
    httpCode = http.GET();
  } else if (ProtType == 1) {
    Serial.print("[HTTP] POST...\n");
    httpCode = http.POST(tempData);
  }
  if (httpCode > 0) {
    Serial.printf("[HTTP] request code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      String payloadAPI = http.getString();
      if (payloadAPI == "[]" || payloadAPI == "") {
        Serial.println("result is empty");
      } else {
        Serial.println("Received payload:");
        Serial.println(payloadAPI);
        saveJSONToFile(filename, payloadAPI);
        Serial.println();
      }
    } else {
      Serial.printf("[HTTP] request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  } else {
    Serial.printf("[HTTP] Unable to connect\n");
  }

  http.end();
}


void flatdetails(WiFiClient client) {
  String path = "/flatdetails.json";
  File jsonFile = SD.open(path.c_str());
  Serial.println("file open");
  if (!jsonFile) {
    Serial.println("Failed to open file for reading");
    return;
  }
  // while (jsonFile.available()) {
  // client.write(jsonFile.read());
  
  String jsonData = "";
  while (jsonFile.available()) {
    jsonData += (char)jsonFile.read();
  }
  jsonFile.close();
  Serial.println(jsonData);
  client.print(jsonData);
}


void sendJSONDataToClient(WiFiClient client, String Appdata) {
  String staticusername = "user";
  String staticpassword = "password";
  String receivedusername = "";
  String receivedpassword = "";

  Serial.println(Appdata);
  int startIndex = Appdata.indexOf("\"Username\":\"");
  int endIndex = Appdata.indexOf("\",\"", startIndex);
  if (startIndex!= -1 && endIndex!= -1) {
    receivedusername = Appdata.substring(startIndex + 12, endIndex);
  }

  startIndex = Appdata.indexOf("\"Password\":\"");
  endIndex = Appdata.indexOf("\",", startIndex);
  if (startIndex!= -1 && endIndex!= -1) {
    receivedpassword = Appdata.substring(startIndex + 12, endIndex);
  }

  Serial.println(receivedusername);
  Serial.println(receivedpassword);

  if (receivedusername.equals(staticusername) && receivedpassword.equals(staticpassword)) {
    Serial.println("ok");
    String jsonResponse = "hello from esp";
    client.println(jsonResponse);
  }
}

void setup() {


  Serial.begin(115200);
  spi_init();  // for sd card


  SPI.begin();

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));



  //  listDir(SD, "/", 0);

    wifiMulti.addAP("wifi id", "password");
  }
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.begin();
  Serial.println("Server started");


  //for flat details data ----------------------------------------------------------------------------------------------------------------------------------------
  if ((wifiMulti.run() == WL_CONNECTED)) {
    String tempData;
    tempData = "some data";
    wifiWorking("Flat_details.php", 1, 0, tempData, "flatdetails.json");
  }
}


void loop() {

  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available() || (millis() - lastMsgTime > timeout)) {
        if (client.available()) {
          String Appdata = "";
          while (client.available()) {
            Appdata += (char)client.read();
          }
           sendJSONDataToClient(client , Appdata);
         // stringtoint(Appdata);
          client.println("");
          lastMsgTime = millis();
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }

}
