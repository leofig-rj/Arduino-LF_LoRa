/*********
  Modelo de Teste da LF_LoRa para LoRa2MQTT, nome TEST01
  Publica uma Lâmpada Dimerizável e uma Entrada Discreta
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

  - Para parear o dispositivo no AddOn LoRa2MQTT do Home Assistant:
    - Na primeira vez, o dispositivo fica com a tela inicial indicando o nome do dispositivo e com o LED piscando.
    - Para o dispositivo entrar no modo pareamento (se não estiver), clique 5 vezes no botão "PRG"
    - No Home Assistant, vá para Configurações/Dispositivos & Serviços/MQTT/Dispositivos/LoRa2MQTT Bridge
    - Acione o Modo Config
    - Após agum tempo o LED para de piscar e a tela muda, indicando dados.
    - Desligue o Modo Config do LoRa2MQTT Bridge
    - Deve aparecer na tela do LoRa2MQTT Bridge, um novo dispositivo em "Dispositivos Conectados"

  - Note que para funcionar é necessário que haja um arquivo de configuração do dispositivo no AddOn LoRa2MQTT.
    No caso, para LORA_MODEL "TEST01", o arquivo é test01.py, já pertencente ao AddOn.
    Veja em: https://github.com/leofig-rj/leofig-hass-addons/blob/main/lora2mqtt/rootfs/usr/bin/models/test01.py
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

  LED_PIN         25
  BTN_PIN         0

  By Leonardo Figueiró @ 2025

*********/

// LoRa
#include <LoRa.h>
#include <LF_LoRa.h>

// OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LORA_MODEL    "TEST01"

//########## Para LoRa
// Pinos do lora (comunicação spi)
#define LORA_RST_PIN    14
#define LORA_SS_PIN     18
#define LORA_SCK_PIN     5
#define LORA_MOSI_PIN   27
#define LORA_MISO_PIN   19
#define LORA_DI00_PIN   26

//########## Para Diplay OLED
#define SCREEN_WIDTH    128 // OLED display width, in pixels
#define SCREEN_HEIGHT    64 // OLED display height, in pixels

#define OLED_RESET       16 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3C for 128x64, 0x3D for 128x32

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
unsigned long lastMsgTime = 0;
String lastID;
String lastRSSI;
String lastMsg;

// Mensagens
bool secondMsg = false;
bool ledState = false;
uint8_t ledBrightness = 255;

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
  // Utilizando PWM do ESP32
  const int pwmFreq = 5000;
  const int pwmResolution = 8; // 0-255
  // Atribuir o canal PWM ao pino do LED
  ledcAttach(LED_PIN, pwmFreq, pwmResolution);
  ledBrightness = 255;
}

void setup() {
  
  // Configuro Serial
  Serial.begin(115200); // para Debug
  
  Serial.println();
  Serial.println(F("Start!"));

  setup_lora();

  setup_display();

  setup_led();

}

void loop_lora() {

  if (LF_LoRa.loopLora()) {
    // Atualizo dados para display
    lastMsgTime = millis();
    lastID = String(LF_LoRa.lastMsgHeader().id);
    lastRSSI = String(LF_LoRa.lastRssi(), DEC) + " dB";
    lastMsg = LF_LoRa.lastMsg();
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
    if (ledState==true) {
      turnOffLED(false);
    } else {
      turnOnLED(false);
    }
  }
  // Verificando se foi dado um duplo click
  if (LF_LoRa.isBtnDblClickActive()) {
    secondMsg = true;
    setBrightnessLED(ledBrightness+30, false);
  }
  // Verificando se foi feito um pressionamento longo (1 a 3 segundos)
  if (LF_LoRa.isBtnLongActive()) {
    secondMsg = true;
    setBrightnessLED(255, false);
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

  loop_lora();

  loop_debug();

  loop_btn();

  loop_display();

  delay(50);
  yield(); 

}

/********************************************
 * Funções para Display
 ********************************************/

void displayLeadOff() {

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 14);
  display.println("Modelo");
  display.setCursor(16, 34);
  display.println(LORA_MODEL);

  display.display();

}

void refreshDisplay() {

    display.clearDisplay();

    String linha1 = "ID msg:   " + lastID;
    String linha2 = "RSSI:     " + lastRSSI;
    String linha3 = "Msg:      " + lastMsg;
    String linha4 = "Segundos: " + String(LF_LoRa.getDeltaMillis(lastMsgTime) / 1000);
    String linha5 = " ";

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
void onExecMsgModeLoop(String sMsg) {

  if (sMsg.substring(0,3).equals(String("000"))) {
    sendState(true);
  } else if (sMsg.substring(0,3).equals(String("101"))) { 
    if (sMsg.length() == 6) {
      setBrightnessLED(sMsg.substring(3,6).toInt(), true);
    } else {
      turnOnLED(true);
    }
  } else if (sMsg.substring(0,3).equals(String("102"))) { 
    turnOffLED(true);
  }
  
}

// Informa o estado do LED no pareamento       
bool onLedCheck() {
  return ledState;
}

// Liga o LED no pareamento     
void onLedTurnOnPairing() {
  ledState = true;
  ledcWrite(LED_PIN, 255);
}

// Desliga o LED no pareamento          
void onLedTurnOffPairing() {
  ledState = false;
  ledcWrite(LED_PIN, 0);
}

/********************************************
 * Funções para Comandos
 ********************************************/
 
// Aqui é tratada a mensagem recebida pelo LF_LoRa
void execMsgModeLoop(String sMsg) {

  if (sMsg.substring(0,3).equals(String("000"))) {
    sendState(true);
  } else if (sMsg.substring(0,3).equals(String("101"))) { 
    if (sMsg.length() == 6) {
      setBrightnessLED(sMsg.substring(3,6).toInt(), true);
    } else {
      turnOnLED(true);
    }
  } else if (sMsg.substring(0,3).equals(String("102"))) { 
    turnOffLED(true);
  }
  
}

void turnOnLED(bool ret) {
   // Liga o LED      
  ledState = true;
  if (ledBrightness==0) ledBrightness = LED_MIN_BRIGHTNESS;
  ledcWrite(LED_PIN, ledBrightness);
  // Envia Estado
  sendState(ret);
}

void turnOffLED(bool ret) {
   // Desliga o LED      
  ledState = false;
  ledcWrite(LED_PIN, 0);
  // Envia Estado
  sendState(ret);
}

void setBrightnessLED(uint8_t brightness, bool ret) {
  // Define o brilho do LED      
  ledBrightness = brightness;
  if (ledBrightness < LED_MIN_BRIGHTNESS) {
    ledBrightness = 0;
    turnOffLED(ret);
  } else {
    turnOnLED(ret);
  }
}

String buildState() {
  char brightness[4];
  sprintf(brightness, "%03d", ledBrightness);
  return String('#') + String(ledState) + String('#') + String(brightness) + String('#') + String(ledState);
}

void sendState(bool retorno) {

  LF_LoRa.sendState(buildState(), retorno);

}
