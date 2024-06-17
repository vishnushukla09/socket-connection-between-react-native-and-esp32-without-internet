#include <EEPROM.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include "RTClib.h"
RTC_DS1307 rtc;

#include <WiFi.h>
#include <WiFiServer.h>
#include <WifiMulti.h>
#include <HTTPClient.h>

const char *ssid = "ESP32";
const char *password = "123456789";
const int serverPort = 80;

WiFiServer server(serverPort);
WiFiMulti wifiMulti;

using std::string;
using std::to_string;

unsigned long lastMsgTime = 0;
const unsigned long timeout = 100;

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

#define SCK 14
#define MISO 12
#define MOSI 13
#define CS 15

#define NUM_LEDS 60      // Number of LEDs in your strip
#define DATA_PIN 6       // Pin where the data line is connected
#define COLOR_ORDER GRB  // Change the order to RGB if needed
#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define STATUS 0x07
#define OBSERVE_TX 0x08
#define CD 0x09
#define RX_ADDR_P0 0x0A
#define RX_ADDR_P1 0x0B
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F
#define TX_ADDR 0x10
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define FIFO_STATUS 0x17
#define DYNPD 0x1C

/* Bit Mnemonics */

/* configuratio nregister */
#define MASK_RX_DR 6
#define MASK_TX_DS 5
#define MASK_MAX_RT 4
#define EN_CRC 3
#define CRCO 2
#define PWR_UP 1
#define PRIM_RX 0

/* enable auto acknowledgment */
#define ENAA_P5 5
#define ENAA_P4 4
#define ENAA_P3 3
#define ENAA_P2 2
#define ENAA_P1 1
#define ENAA_P0 0

/* enable rx addresses */
#define ERX_P5 5
#define ERX_P4 4
#define ERX_P3 3
#define ERX_P2 2
#define ERX_P1 1
#define ERX_P0 0

/* setup of address width */
#define AW 0 /* 2 bits */

/* setup of auto re-transmission */
#define ARD 4 /* 4 bits */
#define ARC 0 /* 4 bits */

/* RF setup register */
#define PLL_LOCK 4
#define RF_DR 3
#define RF_PWR 1 /* 2 bits */

/* general status register */
#define RX_DR 6
#define TX_DS 5
#define MAX_RT 4
#define RX_P_NO 1 /* 3 bits */
#define TX_FULL 0

/* transmit observe register */
#define PLOS_CNT 4 /* 4 bits */
#define ARC_CNT 0  /* 4 bits */

/* fifo status */
#define TX_REUSE 6
#define FIFO_FULL 5
#define TX_EMPTY 4
#define RX_FULL 1
#define RX_EMPTY 0

/* dynamic length */
#define DPL_P0 0
#define DPL_P1 1
#define DPL_P2 2
#define DPL_P3 3
#define DPL_P4 4
#define DPL_P5 5

/* Instruction Mnemonics */
#define R_REGISTER 0x00 /* last 4 bits will indicate reg. address */
#define W_REGISTER 0x20 /* last 4 bits will indicate reg. address */
#define REGISTER_MASK 0x1F
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2
#define REUSE_TX_PL 0xE3
#define ACTIVATE 0x50
#define R_RX_PL_WID 0x60
#define NOP 0xFF
//
#define hooter3 0
#define power_supply 1
#define hooter 2
#define power_supply2 3
#define hooter2 4

#define buzzer 1

// #ifndef NRF24
#define NRF24

// #include "nRF24L01.h"
// #include <stdint.h>

#define nrf24_ADDR_LEN 5
#define nrf24_CONFIG ((1 << EN_CRC) | (0 << CRCO) | (0 << MASK_TX_DS) | (1 << MASK_MAX_RT))

#define NRF24_TRANSMISSON_OK 0
#define NRF24_MESSAGE_LOST 1
#define NOT_RESPOND 1
#define detected 1

#define Usart_Tx 7
#define set 1
#define clear 0
#define rx_data 2 // board number
#define total_switches 6
#define fetching_status 0xff

// const unsigned char Usart_Tx = 7;
// const unsigned char fetching_status = 0xFF;
unsigned char switch_position5 = 0, switch_position2 = 0, switch_position3 = 0, switch_position1 = 0, switch_position4 = 0;
unsigned int data_array[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char data_array_TX[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned int tx_address[5] = { 0, 0, 0, 0, 0 };
unsigned int send_to_main[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char pollingRespSwbArray[20];
unsigned int rx_address[5] = { 0, 0, 0, 0, rx_data }, get_data = 0;
unsigned char count = 0;
unsigned int read_data = 0;
unsigned char key_in_pressed = 0, pre_key_in_pressed = 0, pir_data = 0, pre_pir_data = 0;
unsigned int fetch_flag = 0, send_information = 0, i = 0, k;
unsigned int initiallSwbNo = 3;  // 0,1 we are not using bcz it effect to condition and 2 is for main board so swb start from 3
unsigned int pollinStatus = 0;   // 1 means  this switch board in polling state 0 means not in polling state
unsigned int poledSwitchStatus = 0, pollingSwitchTimeout = 0, pirDetectionTimeout = 0;
unsigned int pollingSwbArray[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned int led_data = 0;
unsigned int ackDataArray[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned int shiftRegSwitchPos = 0;
unsigned int prevStoredSwb = 0;
unsigned int capsOnShfPin = 0, capsOffShfPin = 0, countRespSwb = 2;
int msbValue = 0, lsbValue = 0, TemparRead = 0, prevTemparRead = 0, LuxRead = 0, prevLuxRead = 0, adcValue = 0;
unsigned int Timer_Count = 0, switchStatus = 0, tempVariable = 0;
unsigned char shiftRegPinArr[5] = { 11, 15, 14, 13, 12 };
unsigned int loopBreak = 0, SW = 0, n = 0, panic = 0;
bool FLAG = false;
const int ssPin = 17;
const int cePin = 5;
int swb(40);
String Appdata;
int arraySize;
int dataArray[10];
String payloadAPI;

SPIClass hspi = SPIClass(HSPI);
// const size_t bufferSize = 1024; // Adjust based on your JSON size
// DynamicJsonDocument jsonDocument(bufferSize);

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

void CSN_SetLow() {
  digitalWrite(ssPin, LOW);
}
void CSN_SetHigh() {
  digitalWrite(ssPin, HIGH);
}
void CS_SetLow() {
  digitalWrite(cePin, LOW);
}
void CS_SetHigh() {
  digitalWrite(cePin, HIGH);
}
/* send and receive multiple bytes over SPI */
void nrf24_transferSync(unsigned int *dataout, unsigned int *datain, unsigned int len) {
  unsigned int i;

  for (i = 0; i < len; i++) {
    datain[i] = spi_transfer(dataout[i]);
  }
}

// send multiple bytes over SPI

void nrf24_transmitSync(unsigned int *dataout, unsigned int len) {
  unsigned int i;

  for (i = 0; i < len; i++) {
    spi_transfer(dataout[i]);
    Serial.println(dataout[i]);
  }
}
// Clocks only one byte into the given nrf24 register */

void nrf24_configRegister(unsigned int reg, unsigned int value) {
  CSN_SetLow();
  spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
  spi_transfer(value);
  CSN_SetHigh();
}

/* Read single register from nrf24 */
void nrf24_readRegister(unsigned int reg, unsigned int *value, unsigned int len) {
  CSN_SetLow();
  spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
  nrf24_transferSync(value, value, len);
  CSN_SetHigh();
}

/* Write to a single register of nrf24 */
void nrf24_writeRegister(unsigned int reg, unsigned int *value, unsigned int len) {
  CSN_SetLow();
  spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
  nrf24_transmitSync(value, len);
  CSN_SetHigh();
}

unsigned int payload_len;

// init the hardware pins

void nrf24_init() {
  // nrf24_setupPins();
  CS_SetLow();
  CSN_SetHigh();
}

void nrf24_powerUpRx() {
  CSN_SetLow();
  spi_transfer(FLUSH_RX);
  CSN_SetHigh();
  nrf24_configRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
  CS_SetLow();
  nrf24_configRegister(CONFIG, nrf24_CONFIG | ((1 << PWR_UP) | (1 << PRIM_RX)));
  CS_SetHigh();
}

void nrf24_config(unsigned int channel, unsigned int pay_length) {
  /* Use static payload length ... */
  payload_len = pay_length;

  // Set RF channel
  nrf24_configRegister(RF_CH, channel);

  // Set length of incoming payload
  nrf24_configRegister(RX_PW_P0, 0x00);         // Auto-ACK pipe ...
  nrf24_configRegister(RX_PW_P1, payload_len);  // Data payload pipe
  nrf24_configRegister(RX_PW_P2, 0x00);         // Pipe not used
  nrf24_configRegister(RX_PW_P3, 0x00);         // Pipe not used
  nrf24_configRegister(RX_PW_P4, 0x00);         // Pipe not used
  nrf24_configRegister(RX_PW_P5, 0x00);         // Pipe not used

  // 250 Kbps, TX gain: 0dbm
  nrf24_configRegister(RF_SETUP, (1 << RF_DR) | ((0x03) << RF_PWR));

  // CRC enable, 1 byte CRC length
  nrf24_configRegister(CONFIG, nrf24_CONFIG);

  // Auto Acknowledgment
  nrf24_configRegister(EN_AA, (1 << ENAA_P0) | (1 << ENAA_P1) | (0 << ENAA_P2) | (0 << ENAA_P3) | (0 << ENAA_P4) | (0 << ENAA_P5));

  // Enable RX addresses
  nrf24_configRegister(EN_RXADDR, (1 << ERX_P0) | (1 << ERX_P1) | (0 << ERX_P2) | (0 << ERX_P3) | (0 << ERX_P4) | (0 << ERX_P5));

  //    // Auto retransmit delay: 1000 us and Up to 15 retransmit trials
  //    nrf24_configRegister(SETUP_RETR, (0x03 << ARD) | (0x0F << ARC));

  // Auto retransmit delay: 1000 us and Up to 15 retransmit trials  amar modification
  nrf24_configRegister(SETUP_RETR, (0x0F << ARD) | (0x0F << ARC));

  // Dynamic length configurations: No dynamic length
  nrf24_configRegister(DYNPD, (0 << DPL_P0) | (0 << DPL_P1) | (0 << DPL_P2) | (0 << DPL_P3) | (0 << DPL_P4) | (0 << DPL_P5));

  // Start listening
  nrf24_powerUpRx();
}

void nrf24_powerUpTx() {
  nrf24_configRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

  nrf24_configRegister(CONFIG, nrf24_CONFIG | ((1 << PWR_UP) | (0 << PRIM_RX)));
}

void nrf24_powerDown() {
  CS_SetLow();
  nrf24_configRegister(CONFIG, nrf24_CONFIG);
}

void nrf24_standby1() {
  CS_SetLow();
}

void nrf24_standby1_to_active() {
  CS_SetHigh();
}

/* Set the RX address */
void nrf24_rx_address(unsigned int *adr) {
  CS_SetLow();
  nrf24_writeRegister(RX_ADDR_P1, adr, nrf24_ADDR_LEN);
  CS_SetHigh();
}

/* Returns the payload length */
unsigned int nrf24_payload_length() {
  return payload_len;
}

/* Set the TX address */
void nrf24_tx_address(unsigned int *adr) {
  /* RX_ADDR_P0 must be set to the sending addr for auto ack to work. */
  nrf24_writeRegister(RX_ADDR_P0, adr, nrf24_ADDR_LEN);
  nrf24_writeRegister(TX_ADDR, adr, nrf24_ADDR_LEN);
}

/* Checks if receive FIFO is empty or not */
unsigned int nrf24_rxFifoEmpty() {
  unsigned int fifoStatus;

  nrf24_readRegister(FIFO_STATUS, &fifoStatus, 1);

  return (fifoStatus & (1 << RX_EMPTY));
}
/* Checks if data is available for reading */
/* Returns 1 if data is ready ... */

/* Returns the length of data waiting in the RX fifo */
unsigned int nrf24_payloadLength() {
  unsigned int status;
  CSN_SetLow();
  spi_transfer(R_RX_PL_WID);
  status = spi_transfer(0x00);
  CSN_SetHigh();
  return status;
}

/* Reads payload bytes into data array */
void nrf24_getData(unsigned int *data1) {
  /* Pull down chip select */
  CSN_SetLow();

  /* Send cmd to read rx payload */
  spi_transfer(R_RX_PAYLOAD);

  /* Read payload */
  nrf24_transferSync(data1, data1, payload_len);

  /* Pull up chip select */
  CSN_SetHigh();

  // CLRWDT();
  /* Reset status register */
  nrf24_configRegister(STATUS, (1 << RX_DR));
}

/* Returns the number of retransmissions occured for the last message */
unsigned int nrf24_retransmissionCount() {
  unsigned int rv;
  nrf24_readRegister(OBSERVE_TX, &rv, 1);
  rv = rv & 0x0F;
  return rv;
}

// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.

void nrf24_send(unsigned int *value) {
  /* Go to Standby-I first */
  CS_SetLow();

  /* Set to transmitter mode , Power up if needed */
  nrf24_powerUpTx();

  delayMicroseconds(150);

  /* Do we really need to flush TX fifo each time ? */
  // #if 1
  /* Pull down chip select */
  CSN_SetLow();

  /* Write cmd to flush transmit FIFO */
  spi_transfer(FLUSH_TX);

  /* Pull up chip select */
  CSN_SetHigh();
  //  #endif

  /* Pull down chip select */
  CSN_SetLow();
  delay(10);

  /* Write cmd to write payload */
  spi_transfer(W_TX_PAYLOAD);

  /* Write payload */

  nrf24_transmitSync(value, payload_len);

  /* Pull up chip select */
  CSN_SetHigh();

  /* Start the transmission */
  CS_SetHigh();
}

unsigned int nrf24_getStatus() {
  unsigned int rv;
  CSN_SetLow();
  rv = spi_transfer(NOP);
  CSN_SetHigh();
  return rv;
}

unsigned int nrf24_lastMessageStatus() {
  unsigned int rv;

  rv = nrf24_getStatus();

  /* Transmission went OK */
  if ((rv & ((1 << TX_DS)))) {

    return NRF24_TRANSMISSON_OK;
  } /* Maximum retransmission count is reached */
  /* Last message probably went missing ... */
  else if ((rv & ((1 << MAX_RT)))) {
    Serial1.write(0x46);
    //   //  Serial1.write(0x35);
    return NRF24_MESSAGE_LOST;
  } /* Probably still sending ... */
  else {
    return 0xFF;
  }
}

unsigned int nrf24_isSending() {
  unsigned int status;

  /* read the current status */
  status = nrf24_getStatus();
  //    Serial1.write(0x46);

  if ((status & ((1 << MAX_RT) | (1 << TX_DS)))) {
    // data_rx = 1;
    //        Serial1.write(0x50);
    return 0; /* false */
  } else {
    loopBreak++;

    //         Serial1.write(0x49);
    if (loopBreak > 1500) {
      //            Serial1.write(0x48);
      loopBreak = 0;
      return 0;
    }
    return 1;
  }

  /* if sending successful (TX_DS) or max retries exceded (MAX_RT). */
}

unsigned int nrf24_dataReady() {
  // See note in getData() function - just checking RX_DR isn't good enough
  unsigned int status = nrf24_getStatus();

  // We can int circuit on RX_DR, but if it's not set, we still need
  // to check the FIFO for any pending packets

  if (status & (1 << RX_DR)) {
    return 1;
  }

  return !nrf24_rxFifoEmpty();
}

void clear_data_array() {
  for (unsigned int i = 0; i <= 20; i++) {
    data_array[i] = 0;
  }
}

// Amar modification function from hear on words

void nRFPowerUp() {
  nrf24_powerUpRx();
  delayMicroseconds(130);
}

void txMode(unsigned int *txData)  // transmitting data function
{
  get_data = 0;
  count = 0;
  //_delay_us(50);
  //    if (txData[1] != 0x40)
  //    {
  //        EUSART_Write(0x60);
  //        EUSART_Write(tx_address[4]);
  //        for (unsigned int i = 0; i < txData[1] + 2; i++)
  //        {
  //            EUSART_Write(txData[i]);
  //        }
  //
  //    }

  nrf24_send(txData);  // sending data
                       // wdt_reset();
  while (nrf24_isSending())
    ;
  if (nrf24_lastMessageStatus() == NRF24_TRANSMISSON_OK)  // check auto acknowledge status if send successfully means it give ok
  {
    printf("data sent");
    // USART_Transmit(0x79);
    // USART_Transmit(tx_address[4]);
  } else {
send_again:
    // USART_Transmit(0xB1);
    nrf24_send(txData);
    // wdt_reset();
    while (nrf24_isSending())
      ;
    // _delay_us(35);
    if (nrf24_lastMessageStatus() == NRF24_TRANSMISSON_OK)  // check auto acknowledge status if send successfully means it give ok
    {
      printf("data sent");
      // USART_Transmit(0x76);
      // USART_Transmit(tx_address[4]);
    } else {
      // USART_Transmit(0xB2);
      count++;

      if (count > 1)  // sending three times data
      {
        if ((txData[0] == 0x80) && (txData[3] == rx_data) && (txData[2] == 0x02) || txData[1] == 0x40)  // this for if it polling command or path through different board send check ACK status and info to nearest board this information
        {
          // USART_Transmit(0x52);
          // USART_Transmit(send_to_main[get_data]);
          goto here;
        } else if (txData[0] == 0x80 && txData[1] == 0x01 && txData[2] == 0x01 && send_to_main[get_data] == 0x02) {
          // USART_Transmit(0x53);
          // USART_Transmit(send_to_main[get_data]);
          goto here;
        }

        else if (send_to_main[get_data] != 0)  // using the nearest  board address stored in array using that pass the information to main board
        {
          count = 0;
          // USART_Transmit(0x51);
          // USART_Transmit(send_to_main[get_data]);
          tx_address[4] = send_to_main[get_data];
          nrf24_tx_address(tx_address);
          get_data = ++get_data;
          goto send_again;
        } else {
          goto here;
        }
      }

      goto send_again;
    }
here:
    count = 0;
    printf("data not sent");
  }
}

void pollingDiffBoard(unsigned char *otherBoardData)  // using this board passing data to next board
{
  // EUSART_Write(0x83);
  unsigned int tempArray[10];
  unsigned int length = 0;
  length = otherBoardData[1];

  if (otherBoardData[2] == 0x02 || otherBoardData[2] == 0x01 || otherBoardData[2] == 0x06 || otherBoardData[2] == 0x07 || otherBoardData[2] == 0x11)  // 0x02 is polling cmd
  {
    length = length - 1;
    otherBoardData[1] = length;
    if (otherBoardData[2] == 0x07) {
      tx_address[4] = otherBoardData[length + 4];
    }  // next board data for switch status data
    else {
      tx_address[4] = otherBoardData[length + 3];
    }
    if (otherBoardData[2] == 0x02) {
    }
    for (unsigned int i = 0; i <= length + 4; i++)  // self storing data//
    {
      tempArray[i] = otherBoardData[i];  // load the data information
    }
    // EUSART_Write(0x84);
    // EUSART_Write(tx_address[4]);
    nrf24_tx_address(tx_address);
    txMode(tempArray);
    if (nrf24_lastMessageStatus() == NRF24_TRANSMISSON_OK)  // check auto acknowledge status if send successfully means it give ok
    {

      //
      // EUSART_Write(0x80);
      // EUSART_Write(0x03);
      // EUSART_Write(otherBoardData[2]);
      // EUSART_Write(0x11);

      // EUSART_Write(0x81);
    } else  // if cant accessible that board then send the NACK to main board
    {
      //      EUSART_Write(0x80);
      //    EUSART_Write(0x03);
      //  EUSART_Write(otherBoardData[2]);
      // EUSART_Write(0x00);

      //  EUSART_Write(0x81);
    }
    for (unsigned int i = 0; i <= 10; i++)  // self storing data//
    {
      tempArray[i] = otherBoardData[i];  // load the data information
    }
  }
}

void switchStatusSendSuff(unsigned int length, unsigned int command, unsigned int data) {

  // transmintAgain:
  ackDataArray[0] = 0x80;
  ackDataArray[1] = length;
  ackDataArray[2] = command;
  ackDataArray[3] = data;
  ackDataArray[length] = rx_data;
  ackDataArray[length + 1] = 0x81;
  ackDataArray[length + 2] = 0x00;
  //    if(send_to_main[0]==0)
  //    {
  tx_address[4] = 2;
  // }
  //    else{
  //        tx_address[4] = send_to_main[0];
  //    }

  //    tx_address[4] = 0x11;
  nrf24_tx_address(tx_address);
  txMode(ackDataArray);
  //    if (nrf24_lastMessageStatus() == NRF24_TRANSMISSON_OK) // if cant accessible that board then send the NACK to main board
  //    {
  //        goto scussfullyTransmited;
  //    }
  //    else
  //    {
  //        goto transmintAgain;
  //    }
  // scussfullyTransmited:
  nRFPowerUp();
}
void device_board_data(unsigned int length, unsigned int command, unsigned int data, unsigned int device_no) {
  // transmintAgain:
  ackDataArray[0] = 0x80;
  ackDataArray[1] = length;
  ackDataArray[2] = command;
  ackDataArray[3] = data;
  ackDataArray[4] = device_no;
  ackDataArray[5] = rx_data;
  ackDataArray[6] = 0x81;
  ackDataArray[7] = 0x00;
  tx_address[4] = device_no;
  nrf24_tx_address(tx_address);
  txMode(ackDataArray);
  nRFPowerUp();
}
// void device_board_data4(unsigned int length, unsigned int command, unsigned int data1 ,unsigned int data2,unsigned int data3,unsigned int data4,unsigned int data5,unsigned int data6,unsigned int data7, unsigned int device_no ) {
//   ackDataArray[0] = 0x80;
//   ackDataArray[1] = length;
//   ackDataArray[2] = command;
//   ackDataArray[3] = data1;
//   ackDataArray[4] = data2;
//   ackDataArray[5] = data3;
//   ackDataArray[6] = data4;
//   ackDataArray[7] = data5;
//   ackDataArray[8] = data6;
//   ackDataArray[9] = data7;
//   ackDataArray[12] = device_no;
//   ackDataArray[13] = rx_data;
//   ackDataArray[14] = 0x81;
//   tx_address[4] =   device_no;
//   nrf24_tx_address(tx_address);
//   txMode(ackDataArray);
//   nRFPowerUp();
//   Serial.println("device_board_data4");
// }
unsigned int spi_transfer(unsigned int tx) {

  int rs = 0;
  rs = SPI.transfer(tx);
  // Serial.println(tx);
  return rs;
}


void dataChecking(unsigned char mainData[]) {
  // printf("\ndata checking function");
  if ((mainData[0] == 0x80) && (mainData[1] == 0x01) && (mainData[2] == 0x02) && (mainData[4] == 0x02)) {

    printf("\nentered polling\n");
    pollinStatus = 1;                                                                                                                                                                         // set polling flag to high
    initiallSwbNo = 3;                                                                                                                                                                        // initial switch board number start form 3
                                                                                                                                                                                              // clear_data();
                                                                                                                                                                                              // flush_polling_buffer();
  } else if (mainData[0] == 0x80 && (mainData[1] > 1 && mainData[1] != 0x40) && (mainData[2] == 0x02) && ((mainData[mainData[1] + 3] == rx_data) || (mainData[mainData[1] + 4] == rx_data)))  // data[0] is start bit;data[1] is length; data[1] + 2 is checking for data array of last bye should not be zero
  {

    pollingDiffBoard(mainData);

    nRFPowerUp();
  } else if (mainData[0] == 0x80 && (mainData[1] > 1 && mainData[1] != 0x40) && (mainData[2] == 0x01 || mainData[2] == 0x06 || mainData[2] == 0x07 || mainData[2] == 0x11) && ((mainData[mainData[1] + 3] == rx_data) || (mainData[mainData[1] + 4] == rx_data)))  // data[0] is start bit;data[1] is length; data[1] + 2 is checking for data array of last bye should not be zero
  {
    // printf("entered otherboard data");
    pollingDiffBoard(mainData);

    nRFPowerUp();
    clear_data();
  }
}
void clear_data() {
  for (unsigned int i = 0; i <= 20; i++) {
    data_array_TX[i] = 0;
  }
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
  // if(dataArray1[0] == '23' || dataArray1[0] == 23){

  //       JsonDocument doc; 
  //       String path =  "/mode_scheduling.json";
  //       Serial.println("Directory created");   

  //     dataArray2[0] = dataArray1[0]; //command
  //     dataArray2[1] = dataArray1[2];  // mode number
  //     dataArray2[2] = dataArray1[3];  // switch status
  //     dataArray2[3] = 5 ;       // fan status        for multiple fan ------------
  //     dataArray2[4] = dataArray1[5];  // on time hr
  //     dataArray2[5] = dataArray1[6];  // on time min
  //     dataArray2[6] = dataArray1[7];  // off time hr
  //     dataArray2[7] = dataArray1[8];  // off time min
  //     dataArray2[8] = dataArray1[1];  // board number

  //       File jsonFile = SD.open(path.c_str(), FILE_WRITE);
  //       if (jsonFile) {
  //           serializeJson(doc, jsonFile);
  //           jsonFile.close();
  //           Serial.println("Saved mode scheduling successfully");

  //           DateTime now = rtc.now();
  //           int current_time = now.hour() * 100 + now.minute();

  //           int scheduledOnHour = dataArray1[5];
  //           int scheduledOnMinute = dataArray1[6];

  //           int scheduledOffHour = dataArray1[7];
  //           int scheduledOffMinute = dataArray1[8];

  //           if (now.hour() == scheduledOnHour && 
  //           now.minute() == scheduledOnMinute ){
  //           Serial.println("Scheduled ON time !");
            
  //          Serial.println("data stored in  dataArray2:");
  //          for (int i = 0; i< arraySize; i++){
  //          Serial.println( dataArray2[i]);
  //        }
  //     Serial.print( dataArray2[1]); 
  //    device_board_data4(1,  dataArray2[0] ,  dataArray2[1], dataArray2[2], dataArray2[3] , dataArray2[4], dataArray2[5], dataArray2[6], dataArray2[7], dataArray2[8]); 
  //    }

  //           if (now.hour() == scheduledOffHour &&
  //           now.minute() == scheduledOffMinute) {
  //           Serial.println("Scheduled OFF time!");
  //           Serial.println("data stored in  dataArray2:");
  //           for (int i = 0; i< arraySize; i++){
  //           Serial.println( dataArray2[i]);
  //        }
  //    Serial.print( dataArray2[1]); 
  //    device_board_data4(1,  dataArray2[0] ,  dataArray2[1], dataArray2[2], dataArray2[3] , dataArray2[4], dataArray2[5], dataArray2[6], dataArray2[7], dataArray2[8]);   
  //     }
  // }
  // }
  
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


// this function is use for validating id , pass then sending flat details data ------------------------------------------------------------
// void sendJSONDataToClient(WiFiClient client, String Appdata) {
//   String staticusername = "604_user";
//   String staticpassword = "QdRj4f";
//   String receivedusername = "";
//   String receivedpassword = "";

//   Serial.println(Appdata);
//   int startIndex = Appdata.indexOf("[\"");
//   int endIndex = Appdata.indexOf("\",\"");
//   if (startIndex!= -1 && endIndex!= -1) {
//     receivedusername = Appdata.substring(startIndex + 2, endIndex);
//   }

//   startIndex = endIndex + 2;
//   endIndex = Appdata.indexOf("\"]");
//   if (startIndex!= -1 && endIndex!= -1) {
//   receivedpassword = Appdata.substring(startIndex + 1, endIndex );
//   }
//   Serial.println(receivedusername);
//   Serial.println(receivedpassword);

//   if (receivedusername.equals(staticusername) && receivedpassword.equals(staticpassword)) {
//     Serial.println("ok");
//      String jsonResponse = "[{\"id\":\"57\",\"firstName\":\"Anand\",\"lastName\":\"null\",\"email\":\"anandraj05006@gmail.com\" ,\"username\":\"604_user\",\"password\":\"QdRj4f\",\"usertype\":\"1\",\"userToken\":\"BuK5Tb\"}]";
//     client.println(jsonResponse);
//   }
// }


void sendJSONDataToClient(WiFiClient client, String Appdata) {
  String staticusername = "604_user";
  String staticpassword = "QdRj4f";
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
    String jsonResponse = "[{\"id\":\"57\",\"firstName\":\"Anand\",\"lastName\":\"null\",\"email\":\"anandraj05006@gmail.com\",\"username\":\"604_user\",\"password\":\"QdRj4f\",\"usertype\":\"1\",\"userToken\":\"BuK5Tb\"}]";
    client.println(jsonResponse);
    Serial.println(jsonResponse);
  }
}

void setup() {

  delay(2000);
  Serial.begin(115200);
  spi_init();  // for sd card

  // for nrf
  SPI.begin();
  //  json_Write();
  //  json_Read();
  // nrf SPI settings
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  // Set SS and CE pins as outputs
  pinMode(ssPin, OUTPUT);
  pinMode(cePin, OUTPUT);
  pinMode(2, OUTPUT);     // Set pin 35 as an output
  digitalWrite(2, HIGH);  // Set pin 35 to HIGH
  nrf24_init();
  /* Channel #2 , payload length: 4 */
  nrf24_config(20, 20);
  /* Set the device addresses */
  nrf24_rx_address(rx_address);
  nRFPowerUp();


  //  listDir(SD, "/", 0);
  /*  createDir(SD, "/Su-Roh");
     writeFile(SD, "/payload.txt", "{128,2,5,1,10,128}");
     appendFile(SD, "/payload.txt", "{128,2,5,1,8,128}\n");
     readFile(SD, "/payload.txt");
     //deleteFile(SD, "/payload.txt");
     //renameFile(SD, "/payload.txt", "/anyothername.txt");
     //readFile(SD, "/anyothername.txt");
     testFileIO(SD, "/payload.txt");
     Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
     Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
   */
  // #ifndef ESP8266
  //   while (!Serial)
  //   ; // wait for serial port to connect. Needed for native USB
  // #endif

  //   if (! rtc.begin()) {
  //     Serial.println("Couldn't find RTC");
  //     Serial.flush();
  //     while (1) delay(10);
  //   }
  //
    // if (! rtc.isrunning()) {
    //   Serial.println("RTC is NOT running, let's set the time!");
  //     // When time needs to be set on a new device, or after a power loss, the
  //     // following line sets the RTC to the date & time this sketch was compiled
 // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //     // This line sets the RTC with an explicit date & time, for example to set
  //     // January 21, 2014 at 3am you would call:
  //     //  rtc.adjust(DateTime(2023, 12, 19, 18, 4, 50));
 //  }
  //  Serial.print("Time after adjustment: ");
  //   DateTime now = rtc.now();

  //   Serial.print("Current Time: ");
  //   Serial.print(now.year(), DEC);
  //   Serial.print('/');
  //   Serial.print(now.month(), DEC);
  //   Serial.print('/');
  //   Serial.print(now.day(), DEC);
  //   Serial.print(" (");
  //   Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  //   Serial.print(") ");
  //   Serial.print(now.hour(), DEC);
  //   Serial.print(':');
  //   Serial.print(now.minute(), DEC);
  //   Serial.print(':');
  //   Serial.print(now.second(), DEC);
  //   Serial.println();
  


   

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  data_array_TX[0] = 128;
  data_array_TX[1] = 1;
  data_array_TX[2] = 2;
  data_array_TX[3] = 4;
  data_array_TX[5] = 2;
  data_array_TX[6] = 129;

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();

    wifiMulti.addAP("suva_pyrox", "air74384");
  //  wifiMulti.addAP("Grangi-4thFloor -2.4/5G", "grangi@123");
    // Serial.println("wifi connected");
  }
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.begin();
  Serial.println("Server started");


  // for user validation -------------------------------------------------------------------------------------------------------------------------------------
  // if ((wifiMulti.run() == WL_CONNECTED)){
  //   String tempData;
  //   tempData = "[{\"Username\":\"11_user\",\"Password\":\"SPiagR\"}]";
  //   wifiWorking("http://52.66.113.96/i-switch/automation/loginFromApp.php", 1, 0, tempData,"user_credentials.json");
  // }
  //   delay(1000);

  //for flat details data ----------------------------------------------------------------------------------------------------------------------------------------
  if ((wifiMulti.run() == WL_CONNECTED)) {
    String tempData;
    tempData = "[{\"sl_no\":\"57\"}]";
    wifiWorking("http://52.66.113.96/i-switch/automation/Flat_details.php", 1, 0, tempData, "flatdetails.json");
  }
}


void loop() {

  //socket sending and receiving function ----------------------------------------------------------------------------------------------------------------
  // StaticJsonDocument<512> doc;
  // DeserializationError error = deserializeJson(doc, Appdata);
  // WiFiClient client = server.available();
  // if (client) {
  //   while (client.connected()) {
  //     if (client.available() || (millis() - lastMsgTime > timeout)) {
  //       String Appdata = client.readStringUntil('\r');
  //       Serial.println(Appdata);
  //       delay(5000);
  //     }

  //       sendJSONDataToClient(client, Appdata);
  //     //  stringtoint(Appdata);
  //     client.println("");
  //     lastMsgTime = millis();
  //   }
  //   client.stop();
  //   Serial.println("Client disconnected");
  // }




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



  ////////RECEIVING --------------------------------------------------------------------------------------------------------------------------------------
  if (nrf24_dataReady()) {


    nrf24_getData(data_array);
    printf("\n got nrf data : \n");
    for (unsigned int i = 0; i <= data_array[1] + 1; i++) {
      printf(" %d ,", data_array[i]);  // send received data over nrf to the rpi
    }
    if (data_array[0] == 0x80 && data_array[data_array[1] + 1] == 0x81) {
      String tempData;
      tempData = "[{\"flat_unique_id\":\"Habb_D_6_04\",\"operation\":\"";
      if (data_array[2] == 0x02) {
        for (unsigned int i = 0; i <= data_array[1] + 1; i++) {
          printf(" %d ,", data_array[i]);  // send received data over nrf to the rpi
        }
        for (unsigned int i = 0; i <= data_array[1] + 1; i++) {
          tempData += char(data_array[i]);
          tempData += char(',');
        }
      } else {
        // Handle the else case as needed
        for (unsigned int i = 0; i <= data_array[1] + 1; i++) {
          printf(" %d ,", data_array[i]);  // send received data over nrf to the rpi
        }
      }

      tempData += "\"}]";
      //  wifiWorking("http://3.128.231.248/i-switch/standalone/switchOper.php", 1, 0, tempData);
    }
    poledSwitchStatus = 0;
    clear_data_array();
    clear_data();
  }
  // delay(10000);



  ////// POLLING
  //        if (pollinStatus == 1)
  // if (pollinStatus == 1) //&& poledSwitchStatus == 0) // if this one means start polling 3 - 30 now
  // {

  //     pollingSwbArray[0] = 0x80;
  //     pollingSwbArray[1] = 0x40;
  //     pollingSwbArray[2] = initiallSwbNo;
  //     pollingSwbArray[3] = rx_data;
  //     pollingSwbArray[4] = 0;
  //     tx_address[4] = initiallSwbNo;
  //     // printf("send to initial switch board: %d", initiallSwbNo);
  //     nrf24_tx_address(tx_address);
  //     txMode(pollingSwbArray);
  //     if (nrf24_lastMessageStatus() == NRF24_TRANSMISSON_OK) // check auto acknowledge status if send successfully means it give ok
  //     {
  //         printf("polled done: ");
  //         countRespSwb++;
  //         pollingRespSwbArray[countRespSwb] = initiallSwbNo;
  //         printf("%d\n", pollingRespSwbArray[countRespSwb]);
  //         // poledSwitchStatus = 1;
  //     }
  //     else
  //     {
  //         printf("polled not done: ");
  //         printf("%d\n", pollingRespSwbArray[countRespSwb + 1]);
  //         poledSwitchStatus = 0;
  //     }
  //     // nRFPowerUp();

  //     initiallSwbNo++;
  //     // usleep(2000000);
  // }

  // if (initiallSwbNo == 30) // ones it reaches 48 board then send the stop command  now 48 we can increase how much we want  data is 0x80 0x02 0x02 0x00
  // {
  //     // printf("\nresponse received : \n");

  //     pollinStatus = 0;
  //     initiallSwbNo = 3; // its not switch board number when polling start 3 - 48 switch board for that
  //     pollingRespSwbArray[0] = 0x80;
  //     pollingRespSwbArray[1] = countRespSwb + 1;
  //     pollingRespSwbArray[2] = 0x02;
  //     pollingRespSwbArray[countRespSwb + 1] = rx_data;
  //     pollingRespSwbArray[countRespSwb + 2] = 0x81;
  //     pollingRespSwbArray[countRespSwb + 3] = 0x00;
  //     // printf("response data");

  //     for (unsigned int i = 0; i <= countRespSwb + 3; i++) // self storing data//
  //     {
  //         printf("%d ", pollingRespSwbArray[i]); // send this response to raspberry pi code
  //     }

  //     for (unsigned int i = 0; i < countRespSwb + 3; i++) // self storing data//
  //     {
  //         pollingRespSwbArray[i] = 0; // load the data information
  //     }
  //     countRespSwb = 2;
  //     // clear_data();
  // }
}
