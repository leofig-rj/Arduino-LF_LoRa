/*********
  Modelo de Teste da LF_LoRa para LoRa2MQTT, nome TEST02
  Publica Tensão, Potência, Corrente, Energia, Frequência, Lâmpada (On/Off) e Reset Energia
  - Usando ESP32, placa kit LoRa com SX1276 e OLED WiFi Lora V2 (TENSTAR ROBOT).
  - Para medição de grandezas elétricas usa o PZEM004Tv30
  Bibliotecas:
  - LoRa por Sadeep Mistry Ver 0.8.0
  - LF_LoRa por Leonardo Figueiró Ver 0.0.1
  - PZEM004Tv30 por mandulaj (Jakub Mandula)
  - Adafruit_GFX por Adafruit Ver 1.12.0
  - Adafruit_SSD1306 por Adafruit Ver 2.5.13

  - Configuração para compilação:
    Placa: "ESP32 Dev Module"
    Para gerar o firmware inicial e final, habilitar: 
      "Erase All Flash Before Sketch Upload"

  - Para parear o dispositivo no AddOn LoRa2MQTT do Home Assistant:
    - Na primeira vez, o dispositivo fica com a tela inicial indicando o nome do dispositivo e com o LED piscando.
    - Para o dispositivo entrar no modo pareamento (se não estiver), clique 5 vezes no botão "PRG"
    - No Home Assistant, vá para Configurações/Dispositivos & Serviços/MQTT/Dispositivos/LoRa2MQTT Bridge
    - Acione o Modo Config
    - Após agum tempo o LED para de piscar e a tela muda, indicando dados.
    - Desligue o Modo Config do LoRa2MQTT Bridge
    - Deve aparecer na tela do LoRa2MQTT Bridge, um novo dispositivo em "Dispositivos Conectados"

  - Note que para funcionar é necessário que haja um arquivo de configuração do dispositivo no AddOn LoRa2MQTT.
    No caso, para LORA_MODEL "TEST02", o arquivo é test02.py, já pertencente ao AddOn.
    Veja em: https://github.com/leofig-rj/leofig-hass-addons/blob/main/lora2mqtt/rootfs/usr/bin/models/test02.py
    Para criar um novo dispositivo deve ser usado este padrão.
    O local para colocar arquivos do usuário no Home Assistant é /Config/lora2mqtt/models.

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
 
  PZEM-004T ----- ESP32
  RX        ----- 32 TX1
  TX        ----- 33 RX1

  LED_PIN         25
  BTN_PIN         0
  
  By Leonardo Figueiró @ 2025

*********/

//#define DEBUG_LF  // Para habilitar debug pela serial

// HardwareSerial para mudar pinos das UARTs
#include <HardwareSerial.h>

// WiFi para poder desabilitar
#include <WiFi.h>

// LoRa
#include <SPI.h>
#include <LoRa.h>
#include <LF_LoRa.h>

// PZEM004Tv30
#include <PZEM004Tv30.h>

// OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LORA_MODEL    "TEST02"

// Pinos do lora (comunicação spi)
#define LORA_RST_PIN    14
#define LORA_SS_PIN     18
#define LORA_SCK_PIN     5
#define LORA_MOSI_PIN   27
#define LORA_MISO_PIN   19
#define LORA_DI00_PIN   26

//########## Para PZEM-004T
#define PIN_TX1  32
#define PIN_RX1  33

HardwareSerial HdwSerial1(1); // Para definição dos pinos da UART0 em PZEM

PZEM004Tv30 pzem(Serial1, PIN_RX1, PIN_TX1);

//########## Para OLED
#define SCREEN_WIDTH    128 // OLED display width, in pixels
#define SCREEN_HEIGHT    64 // OLED display height, in pixels

#define OLED_RESET       16   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Pinos do display (comunicação i2c)
#define  SDA_PIN    4
#define  SCL_PIN   15

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//########## Para Botão
#define BTN_PIN            0
#define BTN_INVERTED       true

//########## Para LED
#define LED_PIN           25

//########## Para Processamento
// Display
unsigned long lastDisplayRefresh = 0;
String lastID;

// PZEM-004T
unsigned long pzemRefreshTime;
bool inibeLoopPzem;
float fTensao = -100.0;
float fPotencia = -100.0;
float fCorrente = -100.0;
float fEnergia = -100.0;
float fFrequencia = -100.0;

// Mensagens
uint8_t lastModoOp = LORA_OP_MODE_LOOP;
String sState;
bool secondMsg = false;
String sTensao;
String sPotencia;
String sCorrente;
String sEnergia;
String sFrequencia;
String sInterruptor;

// ######### Rotinas
void setup_lora() {
  // Configuração da biblioteca LF_LoRa, começando pelo hardware LoRa
  LF_LoRa.hardwareCfg(LORA_RST_PIN, LORA_SS_PIN, LORA_SCK_PIN, LORA_MOSI_PIN, LORA_MISO_PIN, LORA_DI00_PIN);
  // Definindo o nome do modelo
  LF_LoRa.slaveCfg(LORA_MODEL);
  // Utilizando a biblioteca para gerenciar o botão todo o tempo e callback do LED no pareamento
  LF_LoRa.btnCfg(BTN_PIN, BTN_INVERTED);
  // Configurando a rotina de callback onExecMsgModeLoop, rotina para tratar a Mensagem recebida pelo LF_LoRa
  LF_LoRa.setOnExecMsgModeLoop(onExecMsgModeLoop);
  // Configurando a rotina de callback onLedCheck, rotina para informar o estado do LED
  LF_LoRa.setOnLedCheck(onLedCheck);
  // Configurando a rotina de callback onLedTurnOnPairing, rotina para ligar do LED no pareamento
  LF_LoRa.setOnLedTurnOnPairing(onLedTurnOnPairing);
  // Configurando a rotina de callback onLedTurnOffPairing, rotina para desligar do LED no pareamento
  LF_LoRa.setOnLedTurnOffPairing(onLedTurnOffPairing);
  // Inicializando a biblioteca
  LF_LoRa.inic();
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

  displayLeadOff();
  
  delay(5000);

}

void setup_led() {
  pinMode(LED_PIN, OUTPUT); 
}

void setup() {
  
  // Configuro HdwSerial1 para definir pinos, Uso depois como Serial1
  HdwSerial1.begin(9600, SERIAL_8N1, PIN_RX1, PIN_TX1);

  // Configuro Serial
  Serial.begin(115200); // para Debug
  Serial1.begin(9600);  // TTL para conectar o PZEM
  
  Serial.println();
  Serial.println("Start!");

  setup_lora();
  
  setup_display();
  
  setup_led();

}

void loop_pzem() {
         
  if (inibeLoopPzem) return;

  if ( LF_LoRa.getDeltaMillis(pzemRefreshTime) > 5000) {
    pzemRefreshTime = millis();

#ifdef DEBUG_LF
    Serial.print("Endereço PZEM: ");
    Serial.println(pzem.readAddress(), HEX);
#endif

    // Leio dados do sensor
    float voltage = pzem.voltage();
    float power = pzem.power();
    float current = pzem.current();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    // Verifico se dados são válidos
    if(isnan(voltage)){
      Serial.println("Erro lendo tensão");
    } else if (isnan(power)) {
      Serial.println("Erro lendo potência");
    } else if (isnan(current)) {
      Serial.println("Erro lendo corrente");
    } else if (isnan(energy)) {
      Serial.println("Erro lendo energia");
    } else if (isnan(frequency)) {
      Serial.println("Erro lendo frequência");
    } else if (isnan(pf)) {
      Serial.println("Erro lendo fator de potência");
    } else {

      // Salvo valores para transmitir...
      fTensao = voltage;
      fPotencia = power;
      fCorrente = current;
      fEnergia = energy;
      fFrequencia = frequency;

#ifdef DEBUG_LF
      // Imprimo valores
      Serial.print("Tensão: ");        Serial.print(voltage,1);      Serial.println(" V");
      Serial.print("Potência: ");      Serial.print(power,1);        Serial.println(" W");
      Serial.print("Corrente: ");      Serial.print(current,3);      Serial.println(" A");
      Serial.print("Energia: ");       Serial.print(energy*1000,0);  Serial.println(" Wh");
      Serial.print("Frequência: ");    Serial.print(frequency,1);    Serial.println(" Hz");
      Serial.print("FP: ");            Serial.println(pf,2);
      Serial.println();
#endif

      // Evito overrange de energy...
      if (energy >= 99999.0) resetEnergy(false);

      if (LF_LoRa.opMode() == LORA_OP_MODE_LOOP)
        refreshDisplay();

    }

  }

}

void loop_lora() {

  if (LF_LoRa.loopLora()) {
    // Atualizo dados para display
    char id[32];
    sprintf(id, "%03d ",LF_LoRa.lastMsgHeader().id);
    lastID = String(id) + String(LF_LoRa.lastRssi(), DEC) + " dB";
    refreshDisplay();
  }

}

void loop_debug() {
  // Para debug, aceito comandos pela serial, simulando LoRa
  // Pode ser excluido
  if (Serial.available()) {
    String sMsg = Serial.readString();
    // Imprimo a msg
    Serial.println(sMsg);
    if (LF_LoRa.opMode() == LORA_OP_MODE_LOOP)
      onExecMsgModeLoop(sMsg.substring(1));
    if (LF_LoRa.opMode() == LORA_OP_MODE_PAIRING)
      if (sMsg.substring(0,1).equals(String("!")))
        LF_LoRa.execMsgModePairing(sMsg.substring(1));
  }
}

void loop_btn() {
  // Loop da biblioteca para tratar Btn
  LF_LoRa.loopBtnLed();
  // Verificando se foi dado um click
  if (LF_LoRa.isBtnClickActive()) {
    secondMsg = true;
    // Comuta o LED
    if (digitalRead(LED_PIN)) {
      turnOffLED(false);
    } else {
      turnOnLED(false);
    }
  }
  // Verificando se envia a segunda Mensagem
  if (secondMsg) {
    // Envio novamente o estado para tentar garantir
    delay(200);
    secondMsg = false;
    sendState(false);
  }
}

void loop_display() {
  // Refresco a cada segundo
  if (LF_LoRa.getDeltaMillis(lastDisplayRefresh) > 1000) {
    lastDisplayRefresh = millis();
    if (LF_LoRa.opMode() == LORA_OP_MODE_PAIRING) {
      displayLeadOff();
    } else {
      refreshDisplay();
    }
  }
}

void loop() {

  loop_pzem();

  loop_lora();
  
  loop_debug();

  loop_btn();

  loop_display();

  delay(50);
  yield(); 

}

/********************************************
 * Funções Auxiliares
 ********************************************/
 
void displayLeadOff() {

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 14);
  display.println("Model");
  display.setCursor(16, 34);
  display.println(LORA_MODEL);

  display.display();

}

void refreshDisplay() {

  String linha1 = "ID msg:   " + lastID;
  String linha2 = "Tensao:   " + String(fTensao,1) + " V";
  String linha3 = "Potencia: " + String(fPotencia,1) + " W";
  String linha4 = "Corrente: " + String(fCorrente,3) + " A";
  String linha5 = "Energia:  " + String(fEnergia*1000,1) + " Wh";

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 4);
  display.println(linha1);
  display.setCursor(0, 16);
  display.println(linha2);
  display.setCursor(0, 28);
  display.println(linha3);
  display.setCursor(0, 40);
  display.println(linha4);
  display.setCursor(0, 52);
  display.println(linha5);

  display.display();

}

/********************************************
 * Funções de Callback
 ********************************************/
 
// Aqui é tratada a mensagem recebida pelo LF_LoRa
void onExecMsgModeLoop(String sCmd) {
  
  if (sCmd.substring(0,3).equals(String("000"))) {
    sendState(true);
  } else if (sCmd.substring(0,3).equals(String("101"))) { 
    turnOnLED(true);
  } else if (sCmd.substring(0,3).equals(String("102"))) { 
    turnOffLED(true);
  } else if (sCmd.substring(0,3).equals(String("110"))) { 
    resetEnergy(true);
  }
  
}

// Informa o estado do LED no pareamento       
bool onLedCheck() {
  return digitalRead(LED_PIN);
}

// Liga o LED no pareamento     
void onLedTurnOnPairing() {
  digitalWrite(LED_PIN, HIGH);
}

// Desliga o LED no pareamento          
void onLedTurnOffPairing() {
  digitalWrite(LED_PIN, LOW);
}

/********************************************
 * Funções para Comandos
 ********************************************/
 
void turnOnLED(bool retorno) {
  // Liga o LED
  digitalWrite(LED_PIN, HIGH);
  // Envia Estado
  sendState(retorno);
}

void turnOffLED(bool retorno) {
  // Desliga o LED      
  digitalWrite(LED_PIN, LOW);
  // Envia Estado
  sendState(retorno);
}

void resetEnergy(bool retorno) {
  
  inibeLoopPzem = true;
  delay(500);
  // Reset da Energia do PZEM
  pzem.resetEnergy();
  delay(500);
  inibeLoopPzem = false;
  // Envia Estado
  sendState(retorno);
}

void leTensao() {
  char msg[16];
  int tensaoAjustada = fTensao * 10 + 0.5;
  sprintf(msg, "%04d",tensaoAjustada);
  sTensao = String(msg);
}

void lePotencia() {
  char msg[16];
  int potenciaAjustada = fPotencia * 10 + 0.5;
  sprintf(msg, "%06d",potenciaAjustada);
  sPotencia = String(msg);
}

void leCorrente() {
  char msg[16];
  if (fCorrente >= 100.0) fCorrente = 99.999;
  int correnteAjustada = fCorrente * 1000 + 0.5;
  sprintf(msg, "%06d",correnteAjustada);
  sCorrente = String(msg);
}

void leEnergia() {
  char msg[16];
  int energiaAjustada = fEnergia*1000;
  sprintf(msg, "%06d",energiaAjustada);
  sEnergia = String(msg);
}

void leFrequencia() {
  char msg[16];
  int frequenciaAjustada = fFrequencia * 10 + 0.5;
  sprintf(msg, "%06d",frequenciaAjustada);
  sFrequencia = String(msg);
}

void leInterruptor() {
  sInterruptor = String(digitalRead(LED_PIN));
}

void buildState() {
  sState = String('#') + sTensao;
  sState = sState + String('#') + sPotencia;
  sState = sState + String('#') + sCorrente;
  sState = sState + String('#') + sEnergia;
  sState = sState + String('#') + sFrequencia;
  sState = sState + String('#') + sInterruptor;
}

void sendState(bool retorno) {

  // Só processo se leitura do PZEM já estabilizou...
  if (fTensao < 0) return;
  if (fPotencia < 0) return;
  if (fCorrente < 0) return;
  if (fEnergia < 0) return;
  if (fFrequencia < 0) return;

  leTensao();
  lePotencia();
  leCorrente();
  leEnergia();
  leFrequencia();
  leInterruptor();
  buildState();

  LF_LoRa.sendState(sState, retorno);

}

