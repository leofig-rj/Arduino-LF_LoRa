/*********
  Modelo de Teste da LF_LoRa para LoRa2MQTT, nome TEST03
  Publica uma Lâmpada RGB e uma Entrada Discreta
  - ESP32C3 Dev Module e modulo LoRa com SX1276.
  Bibliotecas:
  - LoRa por Sadeep Mistry Ver 0.8.0
  - LF_LoRa por Leonardo Figueiró Ver 0.0.1
  - Adafruit NeoPixel by Adafruit Ver 1.12.5

  - Configuração para compilação:
    Placa: "ESP32C3 Dev Module"
    Para gerar o firmware inicial e final, habilitar: 
      "USB CDC On Boot: Enabled"
      "Erase All Flash Before Sketch Upload: Enabled"

  - Para parear o dispositivo no AddOn LoRa2MQTT do Home Assistant:
    - Na primeira vez, o dispositivo fica com a tela inicial indicando o nome do dispositivo e com o LED piscando.
    - Para o dispositivo entrar no modo pareamento (se não estiver), clique 5 vezes no botão "PRG"
    - No Home Assistant, vá para Configurações/Dispositivos & Serviços/MQTT/Dispositivos/LoRa2MQTT Bridge
    - Acione o Modo Config
    - Após agum tempo o LED para de piscar e a tela muda, indicando dados.
    - Desligue o Modo Config do LoRa2MQTT Bridge
    - Deve aparecer na tela do LoRa2MQTT Bridge, um novo dispositivo em "Dispositivos Conectados"

  - Note que para funcionar é necessário que haja um arquivo de configuração do dispositivo no AddOn LoRa2MQTT.
    No caso, para LORA_MODEL "TEST03", o arquivo é test03.py, já pertencente ao AddOn.
    Veja em: https://github.com/leofig-rj/leofig-hass-addons/blob/main/lora2mqtt/rootfs/usr/bin/models/test03.py
    Para criar um novo dispositivo deve ser usado este padrão.
    O local para colocar arquivos do usuário no Home Assistant é /Config/lora2mqtt/models.

  SX1276    ----- ESP32-C3
  NRESET    ----- 2 
  NSS       ----- 3
  SCK       ----- 4
  MOSI      ----- 5
  MISO      ----- 6
  DIO0      ----- 7
  VCC       ----- 3.3V
  GND       ----- GND
 
  LED_PIN         8
  BTN_PIN         9

  By Leonardo Figueiró @ 2025

*********/

// LoRa
#include <SPI.h>
#include <LoRa.h>
#include <LF_LoRa.h>

// LED RGB
#include <Adafruit_NeoPixel.h>

//########## Para LoRa
#define LORA_MODEL      "TEST03"

// Pinos do lora (comunicação spi)
#define LORA_RST_PIN     2
#define LORA_SS_PIN      3
#define LORA_SCK_PIN     4
#define LORA_MOSI_PIN    5
#define LORA_MISO_PIN    6
#define LORA_DI00_PIN    7

//########## Para Botão
#define BTN_PIN   9
#define BTN_INVERTED       true
const unsigned long BUTTON_TIMEOUT = 50;  // Tempo de debounce
const unsigned long BUTTON_DELAY =  500;   // Tempo entre ações do botão

//########## Para LED
#define LED_PIN      8  // Digital IO pin connected to the NeoPixels.
#define LED_COUNT    1  // Número de NeoPixels

// Declarando nosso NeoPixel strip objet0:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

//########## Para Processamento
// Mensagens
bool ledState = false;
uint8_t ledBrightness = 255;
uint8_t ledRed = 255;
uint8_t ledGreen = 255;
uint8_t ledBlue = 255;

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

void setup_led() {
  // Utilizando Adafruit NeoPix
  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.show();  // Initialize all pixels to 'off'
}

void setup() {
  
  // Configuro Serial
  Serial.begin(115200); // para Debug
  
  Serial.println();
  Serial.println("Começo!");

  setup_lora();

  setup_led();

}

void loop_lora() {

  LF_LoRa.loopLora();

}

void loop_debug() {
  // Para debug, aceito comandos pela serial, simulando LoRa
  // Pode ser excluido
  if (Serial.available()) {
    String sMsg = Serial.readString();
    // Imprimo a msg
    Serial.println(sMsg);
    if (LF_LoRa.opMode() == LORA_OP_MODE_LOOP)
      onExecMsgModeLoop(sMsg.substring(1), MSG_TYPE_TELEMETRY);
    if (LF_LoRa.opMode() == LORA_OP_MODE_PAIRING)
      if (sMsg.substring(0,1).equals(String("!")))
        LF_LoRa.execMsgModePairing(sMsg.substring(1));
  }
}

void loop_botao() {
  
  // Loop da biblioteca para tratar Btn
  LF_LoRa.loopBtnLed();
  // Verificando se foi dado um click
  if (LF_LoRa.isBtnClickActive()) {
    // Comuta o LED
    if (ledState==true) {
      turnOffLED(MSG_TYPE_CONFIRM);
    } else {
      turnOnLED(MSG_TYPE_CONFIRM);
    }
  }
  // Verificando se foi dado um duplo click
  if (LF_LoRa.isBtnDblClickActive()) {
    setBrightnessLED(ledBrightness+30, MSG_TYPE_CONFIRM);
  }
  // Verificando se foi feito um pressionamento longo (1 a 3 segundos)
  if (LF_LoRa.isBtnLongActive()) {
    setBrightnessLED(255, MSG_TYPE_CONFIRM);
  }

}

void loop() {

  loop_lora();

  loop_debug();

  loop_botao();

  delay(50);
  yield(); 

}

/********************************************
 * Funções Auxiliares
 ********************************************/
 
// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
    yield(); 
  }
}

/********************************************
 * Funções de Callback
 ********************************************/

// Aqui é tratada a mensagem recebida pelo LF_LoRa
void onExecMsgModeLoop(String sMsg, MsgType mt) {
 
  if (sMsg.substring(0,3).equals(String("000"))) {
    sendState(mt);
  } else if (sMsg.substring(0,3).equals(String("101"))) { 
    if (sMsg.length() == 6) {
      setBrightnessLED(sMsg.substring(3,6).toInt(), mt);
    } else if (sMsg.length() == 12) {
        setColor(sMsg.substring(3,6).toInt(), sMsg.substring(6,9).toInt(), sMsg.substring(9,12).toInt(), mt);
    } else {
      turnOnLED(mt);
    }
  } else if (sMsg.substring(0,3).equals(String("102"))) { 
    turnOffLED(mt);
  }
  
}

// Informa o estado do LED no pareamento       
bool onLedCheck() {
  return ledState;
}

// Liga o LED no pareamento     
void onLedTurnOnPairing() {
  ledState = true;
  colorWipe(strip.Color(64,64,64), 10);
}

// Desliga o LED no pareamento          
void onLedTurnOffPairing() {
  ledState = false;
  colorWipe(strip.Color(0,0,0), 10);
}

/********************************************
 * Funções para Comandos
 ********************************************/

void turnOnLED(MsgType mt) {
   // Liga o LED      
  ledState = true;
  if (ledBrightness==0) ledBrightness = LED_MIN_BRIGHTNESS;
  uint8_t r = ledRed * ledBrightness / 256;
  uint8_t g = ledGreen * ledBrightness / 256;
  uint8_t b = ledBlue * ledBrightness / 256;
  colorWipe(strip.Color(r,g,b), 10);
  // Envia Estado
  sendState(mt);
}

void turnOffLED(MsgType mt) {
   // Desliga o LED      
  ledState = false;
  colorWipe(strip.Color(0,0,0), 10);
  // Envia Estado
  sendState(mt);
}

void setBrightnessLED(uint8_t brightness, MsgType mt) {
  // Define o brilho do LED      
  ledBrightness = brightness;
  if (ledBrightness < LED_MIN_BRIGHTNESS) {
    ledBrightness = 0;
    turnOffLED(mt);
  } else {
    turnOnLED(mt);
  }
}

void setColor(uint8_t r, uint8_t g, uint8_t b, MsgType mt) {
  ledRed = r;
  ledGreen = g;
  ledBlue = b;
  turnOnLED(mt);
}


String buildState() {
  char br[4];
  sprintf(br, "%03d", ledBrightness);
  char r[4];
  sprintf(r, "%03d", ledRed);
  char g[4];
  sprintf(g, "%03d", ledGreen);
  char b[4];
  sprintf(b, "%03d", ledBlue);
  return String('#') + String(ledState) + String('#') + String(br) + String('#') + String(r) + String('#') + String(g) + String('#') + String(b) + String('#') + String(ledState);
}

void sendState(MsgType mt) {
  LF_LoRa.sendState(buildState(), mt);
}
