/*********
  Adapatador USB de LF_LoRa para LoRa2MQTT
  - Usando ESP32, placa kit LoRa com SX1276 e OLED WiFi Lora V2 (TENSTAR ROBOT).

  Bibliotecas:
  - LoRa por Sadeep Mistry Ver 0.8.0
  - LF_LoRa por Leonardo Figueiró Ver 0.0.1
  - Adafruit_GFX por Adafruit Ver 1.12.0
  - Adafruit_SSD1306 por Adafruit Ver 2.5.13

  - Configuração para compilação:
    Placa: "ESP32 Dev Module"
    Para gerar o firmware inicial e final, habilitar: 
      "Erase All Flash Before Sketch Upload"

  SX1276    ----- ESP32
  NRESET    ----- 14 
  NSS       ----- 18
  SCK       ----- 5
  MOSI      ----- 27
  MISO      ----- 19
  DIO0      ----- 26
  VCC       ----- 3.3V
  GND       ----- GND
 
  OLED      ----- ESP32
  RST       ----- 16
  SDA       ----- 4
  SCL       ----- 15

  By Leonardo Figueiró @ 2025

*********/

// WiFi para poder desabilitar
#include <WiFi.h>

// LoRa
#include <SPI.h>
#include <LoRa.h>
#include <LF_LoRa.h>

// OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define CMD_GET_USB_MODEL      "!000"
#define CMD_SET_SYNCH_FREQ     "!001"
#define USB_MODEL              "USB Adapter Ver 1.0"

//########## Para LoRa
// Pinos do lora (comunicação spi)
#define LORA_RST_PIN    14
#define LORA_SS_PIN     18
#define LORA_SCK_PIN     5
#define LORA_MOSI_PIN   27
#define LORA_MISO_PIN   19
#define LORA_DI00_PIN   26

uint8_t synch_word = LORA_SYNC_WORD_DEF;
long frequency = LORA_FREQ_NA;

//########## Para Diplay OLED
#define SCREEN_WIDTH    128 // OLED display width, in pixels
#define SCREEN_HEIGHT    64 // OLED display height, in pixels

#define OLED_RESET       16 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3C for 128x64, 0x3D for 128x32

// Pinos do display (comunicação i2c)
#define  SDA_PIN    4
#define  SCL_PIN   15

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup_lora() {

  // Iniciando a comunicação SPI
  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
  
  // Configurando o módulo transceptor LoRa
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DI00_PIN);
  
  // Inicializando o módulo transceptor LoRa
  while (!LoRa.begin(frequency)) {
    Serial.println(".");
    delay(500);
  }

  // Ajustando a potência do transmissor
  LoRa.setTxPower(20);

  // Definindo a palavra de sincronismo
  LoRa.setSyncWord(synch_word);

  // Entrando no modo "receive"
  LoRa.receive();

  Serial.println("LoRa Iniciando, OK!");

}

void setup_wifi() {

  // Desligando o WiFi para economizar energia
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
}

void setup_display() {

  delay(2000);
  Serial.println(F("Display SSD1306 setup"));

  // Configuro os pinos do I2C (Wire)  
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // velocidade 100kHz

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Display SSD1306 allocation failed"));
    for(;;); // Se falhou, entro num loop eterno.
  }

  displayStatus();
  
}

void setup() {
  
  // Configurando a Serial
  Serial.begin(115200); // para comunicar com LoRa2MQTT
  
  Serial.println();
  Serial.println("Começo!");

  setup_wifi();

  setup_lora();

  setup_display();

}

void loop_lora() {
  
  // Verificando se tem pacote
  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    if (packetSize > LF_LORA_MAX_PACKET_SIZE) {
      return;
    }

    // Lendo o pacote
    String LoraData = "";

    while (LoRa.available()) {
      LoraData += (char)LoRa.read();
    }

    // Criando buffer para receber a mensagem processada
    char msg_data[LF_LORA_MAX_PACKET_SIZE];

    if (LF_LoRa.loraDecode(LoraData.c_str(), LoraData.length(), msg_data)) {
      if (msg_data[0] == '!') {
        // Mensagem com ! no início envia direto para LoRa2MQTT
        Serial.println(msg_data);
      } else {
        // Não começa com !, envia a mensagem para LoRa2MQTT com #RSSI no início
        char msg[LF_LORA_MAX_PACKET_SIZE];
        sprintf(msg, "#%04d%s",LoRa.packetRssi(),msg_data);
        Serial.println(msg);
      }
    }

  }

}

void loop_serial() {

  if (Serial.available()) {

    String sSerialMsg = Serial.readString();

    if (sSerialMsg.charAt(0) == '#') {
      // Mensagem com # no início. É comunicação LoRa...
      String sLoRaMsg = sSerialMsg.substring(1);  // Remove o primeiro caractere
      // Enviando a mensagem sem o primeiro caractere para o módulo LoRa
      enviaParaLoRa(sLoRaMsg);
    } else if (sSerialMsg.charAt(0) == '!') {
      // Mensagem com ! no início. É de configuração
      if (sSerialMsg.equals(CMD_GET_USB_MODEL)) {
        Serial.print("!");Serial.println(USB_MODEL);
        return;
      }
      if (sSerialMsg.substring(0,4).equals(CMD_SET_SYNCH_FREQ)) {
        synch_word = sSerialMsg.substring(4,7).toInt();
        frequency = (int)sSerialMsg.substring(7,12).toFloat();
        LoRa.setSyncWord(synch_word);
        LoRa.setFrequency(frequency);
        displayStatus();
        return;
      }
      // Enviando a mensagem completa para para o módulo LoRa
      enviaParaLoRa(sSerialMsg);
    }
  }

}

void loop() {

  loop_lora();

  loop_serial();

}

/********************************************
 * Funções para LoRa
 ********************************************/
 
void enviaParaLoRa(String sMsg) {

  // Enviando estado via LoRa
  LoRa.beginPacket();

  // Criando buffer para colocar dados LoRa
  char lora_data[LF_LORA_MAX_PACKET_SIZE];

  // Codificando pacote LoRa
  LF_LoRa.loraEncode(sMsg.c_str(), sMsg.length(), lora_data);

  // Enviando LoRa
  LoRa.print(lora_data);

  LoRa.endPacket();

  // Entrando no modo "receive"
  LoRa.receive();

}

/********************************************
 * Funções para Display
 ********************************************/

void displayStatus() {

    display.clearDisplay();

    char sw[6];
    sprintf(sw, "0x%02X", synch_word);
    char fr[20];          
    sprintf(fr, "%d", frequency / 1000000);

    String linha1 = "USB LF_LoRa Adapter";
    String linha2 = "Ver:       1.0";
    String linha3 = "SynchWord: " + String(sw);
    String linha4 = "Frequency: " + String(fr) + "E6";
    String linha5 = " ";

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println(linha1);
    display.setCursor(0, 24);
    display.println(linha2);
    display.setCursor(0, 38);
    display.println(linha3);
    display.setCursor(0, 52);
    display.println(linha4);

    display.display();

}

