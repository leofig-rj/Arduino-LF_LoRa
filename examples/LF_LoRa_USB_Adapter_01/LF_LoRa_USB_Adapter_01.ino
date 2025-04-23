/*********
  Adapatador USB de Teste da LF_LoRa para LoRa2MQTT
  - Usando ESP32-C3-DevKitM-1 + SX1276
  Bibliotecas:
  - LoRa por Sadeep Mistry Ver 0.8.0
  - LF_LoRa por Leonardo Figueiró Ver 1.0.0

  By Leonardo Figueiró @ 2025

*********/

// WiFi para poder desabilitar
#include <WiFi.h>

// LoRa
#include <SPI.h>
#include <LoRa.h>
#include <LF_LoRa.h>

#define CMD_GET_USB_MODEL      "!000"
#define CMD_SET_SYNCH_WORD     "!001"
#define USB_MODEL              "USB Adapter Ver 1.0"

//########## Para LoRa
// Pinos do lora (comunicação spi)
#define LORA_RST_PIN     2
#define LORA_SS_PIN      3
#define LORA_SCK_PIN     4
#define LORA_MOSI_PIN    5
#define LORA_MISO_PIN    6
#define LORA_DI00_PIN    7

void setup_lora() {

  // Iniciando a comunicação SPI
  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
  
  // Configurando o módulo transceptor LoRa
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DI00_PIN);
  
  // Inicializando o módulo transceptor LoRa
  while (!LoRa.begin(LORA_FREQ_NA)) {
    Serial.println(".");
    delay(500);
  }

  // Ajustando a potência do transmissor
  LoRa.setTxPower(20);

  // Definindo a palavra de sincronismo
  LoRa.setSyncWord(LORA_SYNC_WORD_DEF);

  // Entrando no modo "receive"
  LoRa.receive();

  Serial.println("LoRa Iniciando, OK!");

}

void setup() {
  
  // Desligando o WiFi para economizar energia
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  // Configurando a Serial
  Serial.begin(115200); // para comunicar com LoRa2MQTT
  
  Serial.println();
  Serial.println("Começo!");

  setup_lora();

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
        char msg[16];
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
      if (sSerialMsg.substring(0,4).equals(CMD_SET_SYNCH_WORD)) {
        uint8_t synch_word = sSerialMsg.substring(4,7).toInt();
        LoRa.setSyncWord(synch_word);
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

