/*
 * Copyright (c) 2025 by Leonardo Figueiro <leonardo@wilight.com.br>
 * LF_LoRa library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include <LF_LoRa.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

// WiFi para poder desabilitar
#include <WiFi.h>

// LoRa
#include <SPI.h>
#include <LoRa.h>

// ## Variáveis fora da Classe


// LF_LoRaClass Class Methods
/* -------------------------------------------------------------------------- */
LF_LoRaClass::LF_LoRaClass()
{

}

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::hardwareCfg(uint8_t rstPin, uint8_t ssPin, uint8_t sckPin, uint8_t mosiPin, uint8_t misoPin, uint8_t di00Pin) {
  _loraRstPin = rstPin;
  _loraSsPin = ssPin;
  _loraSckPin = sckPin;
  _loraMosiPin = mosiPin;
  _loraMisoPin = misoPin;
  _loraDi00Pin = di00Pin;

  // Iniciamos a comunicação SPI
  SPI.begin(_loraSckPin, _loraMisoPin, _loraMosiPin, _loraSsPin);

  // Configuração do módulo transceptor LoRa
  LoRa.setPins(_loraSsPin, _loraRstPin, _loraDi00Pin);

} /* hardwareCfg */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::slaveCfg(String model) {
  _sModel = model;
  _opMode = LORA_OP_MODE_CFG;
  _stepNegotiation = LORA_STEP_NEG_INIC;
} /* slaveCfg */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::btnLedCfg(uint8_t btn_pin, boolean btn_inverted, uint8_t led_pin, boolean led_inverted) {
  _btnPin = btn_pin;
  _btnInverted = btn_inverted;
  _ledPin = led_pin;
  _ledInverted = led_inverted;
  pinMode(_btnPin, INPUT);
  pinMode(_ledPin, OUTPUT);
  digitalWrite(_ledPin, _ledInverted ? HIGH : LOW);
} /* btnLedCfg */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::inic() {

  // Habilito o WiFi para pegar o MAC
  WiFi.softAP("AP_TEMP", "12345678"); // Configura o ESP32 como AP
  _sMac = WiFi.softAPmacAddress();
  while (_sMac.lastIndexOf(':') >= 0) {
    _sMac.remove(_sMac.lastIndexOf(':'), 1);
  }
  _sLast6Mac = _sMac.substring(6);
  // Desligo o WiFi para economizar energia
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // Abro Preferences com o namespace "LoRa". Cada módulo, biblioteca, etc
  // deve usar um namespace para previnir colisões de nome de chave. Irá abrir o
  // armazenamento em modo RW (segundo parâmetro deve ser false).
  // se o segundo parâmetro for true, abrirá no modo Read Only.
  // Nota: O namespace é limitado a 15 caracteres.
  pref.begin("LoRa", true);
  // Pego o valor do contador de vazão, se a chave não existrir, retorna o valor default de 0.
  // Nota: O nome da chave é limitado a 15 caracteres.
  _opMode = pref.getUInt("opMode", _opMode);
  _masterAddr = pref.getUInt("masterAddr", 0);
  _myAddr = pref.getUInt("meuAddr", 0);
  // Fecho Preferences
  pref.end();

  // Inicialização do módulo transceptor LoRa
  while (!LoRa.begin(_loraFrequency)) {
    if (_debugEnabeld) {
      Serial.println(".");
    }
    delay(500);
  }

  LoRa.setTxPower(20);

  LoRa.setSyncWord(_syncWord);

  // entro no modo "receive"
  LoRa.receive();

  if (_debugEnabeld) {
    Serial.println("LoRa Iniciando, OK!");
  }

} /* inic */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::setDebugEnable(boolean debugEnable)
{
  _debugEnabeld = debugEnable;
} /* setDebugEnable */

/* -------------------------------------------------------------------------- */
LF_LoRaClass& LF_LoRaClass::setExecMsgModeLoop(LF_LORA_EXEC_MSG_MODE_LOOP) {
  this->execMsgModeLoop = execMsgModeLoop;
  return *this;
} /* setExecMsgModeLoop */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::setFrequency(long frequency) {
  _loraFrequency = frequency;
} /* setFrequency */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::setSyncWord(int synch) {
  _syncWord = synch;
  LoRa.setSyncWord(_syncWord);
} /* setSyncWord */

/* -------------------------------------------------------------------------- */
uint8_t LF_LoRaClass::myAddr() {
  return _myAddr;
} /* myAddr */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::setMyAddr(uint8_t addr) {
  _myAddr = addr;
} /* setMyAddr */

/* -------------------------------------------------------------------------- */
uint8_t LF_LoRaClass::masterAddr() {
  return _masterAddr;
} /* masterAddr */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::setMasterAddr(uint8_t addr) {
  _masterAddr = addr;
} /* setMasterAddr */

/* -------------------------------------------------------------------------- */
uint8_t LF_LoRaClass::opMode() {
  return _opMode;
} /* opMode */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::setOpMode(uint8_t modo) {
  _opMode = modo;
  if (_opMode ==LORA_OP_MODE_CFG) {
    _stepNegotiation = LORA_STEP_NEG_INIC;
    _lastModoOp = LORA_OP_MODE_CFG;
  }
} /* setOpMode */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::loraEncode(const char *in, int len, char *out)
{
  out[0] = random(32, 126);
  out[1] = random(32, 126);
  uint8_t j = (uint8_t)out[0];
  uint8_t d = (uint8_t)out[1] % 7 + 1;
  for (uint8_t i = 0; i < len; i++) {
    if (j > 127) j -= 128;
    out[i*2+2] = char((uint8_t)in[i]/16+32+lf_key[j]);
    out[i*2+3] = char((uint8_t)in[i]%16+32+lf_key[j]);
    j += d;
  }
  out[len*2+2] = 0;
} /* loraEncode */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::loraAddHeader(const char *in, int len, uint8_t para, char *out) {
  _lastSendId++;
  loraAddHeaderRet(in, len, para, _lastSendId, out);
} /* loraAddHeader */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::loraAddHeaderRet(const char *in, int len, uint8_t para, uint8_t id, char *out) {
  char aux[len + 10]; // Buffer aux para inserir cabeçalho
  sprintf(aux, "%02X%02X%02X%04X", _myAddr, para, id, len + 10);
  // Completo com msg de entrada
  for (uint8_t i = 0; i < len; i++) {
    aux[i+10] = in[i];
  }
  loraEncode(aux, len + 10, out);
} /* loraAddHeader */

/* -------------------------------------------------------------------------- */
boolean LF_LoRaClass::loraDecode(const char *in, int len, char *out)
{
  // Testo se os caracteres estão OK
  for (uint8_t i = 0; i < len; i++) {
    if (((uint8_t)in[i]<32)||((uint8_t)in[i]>126)) {
      out[0] = 0; // Retorna nulo
      return false;
    }
  }
  // Agora faço a decodificação para aux
  uint8_t a1, a2, a3;
  uint8_t j = (uint8_t)in[0];
  uint8_t d = (uint8_t)in[1] % 7 + 1;
  uint8_t l = len/2-1;
  for (uint8_t i = 0; i < l; i++) {
    if (j > 127) j -= 128;
    a1 = (uint8_t)in[i*2+2]-32-lf_key[j];
    a2 = (uint8_t)in[i*2+3]-32-lf_key[j];
    a3 = a1*16+a2;
    if ((a3<32)||(a3>126)) {
      out[0] = 0; // Retorna nulo
      return false;
    }
    out[i] = char(a3);
    j += d;
  }
  out[l] = 0;
  return true;
} /* loraDecode */

/* -------------------------------------------------------------------------- */
uint8_t LF_LoRaClass::loraCheckMsgIni(const char *in, int len, uint8_t &de, uint8_t &para, char *out)
{

  uint8_t l = len/2-1;
  uint8_t id;
  int index;

  // Buffer aux para decodificação
  char aux[LF_LORA_MAX_PACKET_SIZE];

  if (!loraDecode(in, len, aux)) {
    return LORA_MSG_CHECK_ERROR; // erro nos dados
  }

  // Agora testo o buffer decodificado
  for (uint8_t i = 0; i < 10; i++) {
    if (!isxdigit(aux[i])) {
      // Não é HEX
      out[0] = 0; // Retorna nulo
      return LORA_MSG_CHECK_ERROR; // erro nos dados
    }
  }

  // Analiso o buffer aux...
  char sub[4 + 1]; // +1 para o caractere nulo '\0'
  // Pego DE
  strncpy(sub, aux + 0, 2);
  sub[2] = '\0';
  de = strtol(sub, NULL, 16); // Base 16
  // Pego PARA
  strncpy(sub, aux + 2, 2);
  sub[2] = '\0';
  para = strtol(sub, NULL, 16); // Base 16
  // Pego ID
  strncpy(sub, aux + 4, 2);
  sub[2] = '\0';
  id = strtol(sub, NULL, 16); // Base 16
  // Pego LEN
  strncpy(sub, aux + 6, 4);
  sub[4] = '\0';
  int len_in_msg = strtol(sub, NULL, 16); // Base 16
  // Testo LEN
  if (len_in_msg != l) {
    out[0] = 0; // Retorna nulo
    return LORA_MSG_CHECK_ERROR; // erro nos dados
  }
  // Separo a msg limpa
  for (uint8_t i = 0; i < l-10; i++) {
    out[i] = aux[i+10];
  }
  out[l-10] = 0;

  // Salvo último cabeçalho recebido
  _lastRegRec = {de, para, id};

  // Procuro registro de cabeçalho
  index = findRegRec(de, para);
  if (index==-1) {
    // Não tem registro, crio...
    addRegRec(de, para, id);
  } else {
    // Tem registro...
    if (_regRecs[index].id==id) {
      return LORA_MSG_CHECK_ALREADY_REC; // msg já recebida
    }
    // Atualizo ID
    _regRecs[index].id=id;
  }
  return LORA_MSG_CHECK_OK; // OK

} /* loraCheckMsgIni */

/* -------------------------------------------------------------------------- */
uint8_t LF_LoRaClass::loraCheckMsg(const char *in, int len, char *out)
{
  uint8_t de;
  uint8_t para;
  uint8_t ret;

  ret = loraCheckMsgIni(in, len, de, para, out);

  if (ret != LORA_MSG_CHECK_OK) return ret;

  if (para!=_myAddr) {
    return LORA_MSG_CHECK_NOT_ME; // msg não é para mim
  }
  if (de!=_masterAddr) {
    return LORA_MSG_CHECK_NOT_MASTER; // msg não é do master
  }
  return LORA_MSG_CHECK_OK; // OK

} /* loraCheckMsg */

/* -------------------------------------------------------------------------- */
uint8_t LF_LoRaClass::loraCheckMsgMaster(const char *in, int len, char *out)
{
  uint8_t de;
  uint8_t para;
  uint8_t ret;

  ret = loraCheckMsgIni(in, len, de, para, out);

  if (ret != LORA_MSG_CHECK_OK) return ret;

  if (para!=_myAddr) {
    return LORA_MSG_CHECK_NOT_ME; // msg não é para mim
  }
  return LORA_MSG_CHECK_OK; // OK

} /* loraCheckMsgMaster */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::addRegRec(uint8_t de, uint8_t para, uint8_t id) {
  RegRec *novoArray = new RegRec[_regRecsLen + 1];

  // Copia os dados antigos
  for (int i = 0; i < _regRecsLen; i++) {
    novoArray[i] = _regRecs[i];
  }

  // Adiciona o novo registro
  novoArray[_regRecsLen] = {de, para, id};

  // Libera a memória antiga
  delete[] _regRecs;

  // Atualiza o ponteiro
  _regRecs = novoArray;
  _regRecsLen++;
} /* addRegRec */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::removeRegRec(int index) {
  if (index < 0 || index >= _regRecsLen) return;

  RegRec *novoArray = new RegRec[_regRecsLen - 1];

  // Copia os registros, ignorando o índice a ser removido
  for (int i = 0, j = 0; i < _regRecsLen; i++) {
    if (i != index) {
      novoArray[j++] = _regRecs[i];
    }
  }

  delete[] _regRecs;
  _regRecs = novoArray;
  _regRecsLen--;
} /* removeRegRec */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::clearRegRecs() {
  delete[] _regRecs; // Libera o array
  _regRecs = nullptr;
  _regRecsLen = 0;
} /* clearRegRecs */

/* -------------------------------------------------------------------------- */
int LF_LoRaClass::findRegRec(uint8_t de, uint8_t para) {
  for (int i = 0; i < _regRecsLen; i++) {
    if ((_regRecs[i].de==de)&&(_regRecs[i].para==para)) {
      return i;
    }
  }
  return -1;
} /* findRegRec */

/* -------------------------------------------------------------------------- */
RegRec LF_LoRaClass::lastMsgHeader() {
  return _lastRegRec;
} /* lastMsgHeader */

/* -------------------------------------------------------------------------- */
uint8_t LF_LoRaClass::lastSendId() {
  return _lastSendId;
} /* lastSendId */

/* -------------------------------------------------------------------------- */
const char* LF_LoRaClass::descCheckMsg(int ret) {
  if (ret<0||ret>4)
    return("");
  return descCheckMsgLora[ret];
} /* descCheckMsg */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::execMsgModeCfg(String sMsg) {

  if (_debugEnabeld) {
    Serial.println("Cfg Cmd: " + sMsg + " Len: " + String(sMsg.length()));
  }
  if (!sMsg.substring(6,7).equals(String("!"))) return;
  if (!sMsg.substring(13,14).equals(String("!"))) return;
  String sPara = sMsg.substring(0,6);
  String sDe = sMsg.substring(7,13);
  String sCmd = sMsg.substring(14,17);
  sDe.toUpperCase();
  if (_debugEnabeld) {
    Serial.println("De: " + sDe + " Para: "  + sPara + " Cmd: "  + sCmd);
  }

  if ((_stepNegotiation == LORA_STEP_NEG_INIC) || (_stepNegotiation == LORA_STEP_NEG_CFG)) {
    if (_debugEnabeld) {
      Serial.println("LORA_STEP_NEG_INIC ou LORA_STEP_NEG_CFG");
    }
    if (sMsg.length() == 17) { // Comando inicial...
      if (!sDe.equals(String("FFFFFF"))) return;
      if ((sPara.equals(String("000000"))) && (sCmd.equals(String("100")))) {
        String sRet = "!FFFFFF!" + _sLast6Mac + "!100!" + _sMac + "!" + _sModel;
        if (_debugEnabeld) {
          Serial.println(sRet);
        }
        sendNegotiation(sRet);
        delay(500);
        if (_debugEnabeld) {
          Serial.println(sRet);
        }
        sendNegotiation(sRet);
        _stepNegotiation = LORA_STEP_NEG_CFG;
        return;
      }
    }
  }

  if (_stepNegotiation == LORA_STEP_NEG_CFG) {
    if (_debugEnabeld) {
      Serial.println("LORA_STEP_NEG_CFG");
    }
    if (sMsg.length() != 34) return;
    if (!sMsg.substring(17,18).equals(String("!"))) return;
    if (!sMsg.substring(26,27).equals(String("!"))) return;
    if (!sMsg.substring(30,31).equals(String("!"))) return;
    if (!sDe.equals(String("FFFFFF"))) return;
    if ((sPara.equals(_sLast6Mac)) && (sCmd.equals(String("101")))) {
      String sCfg = sMsg.substring(18,26);
      String sAddrM = sMsg.substring(27,30);
      String sAddrE = sMsg.substring(31,34);
      String sRet = "!FFFFFF!" + _sLast6Mac + "!101!" + sCfg + "!" + sAddrM + "!" + sAddrE;
      if (_debugEnabeld) {
        Serial.println(sRet);
      }
      sendNegotiation(sRet);
      delay(500);
      if (_debugEnabeld) {
        Serial.println(sRet);
      }
      sendNegotiation(sRet);
      _stepNegotiation = LORA_STEP_NEG_FIM;
      _masterAddr = sAddrM.toInt();
      _myAddr = sAddrE.toInt();
      setOpMode(LORA_OP_MODE_LOOP);

      // Abro Preferences com o nomespace "LoRa"
      pref.begin("LoRa", false);
      // Salvo _opMode, _masterAddr e _myAddr na memória não volátil com os nomes das chaves "opMode", "masterAdd" e "myAddr"
      pref.putUInt("opMode", _opMode);
      pref.putUInt("masterAddr", _masterAddr);
      pref.putUInt("myAddr", _myAddr);
      // Fecho Preferences
      pref.end();

      return;
    }
  }

} /* execMsgModeCfg */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::sendNegotiation(String sRet) {

  if (_opMode != LORA_OP_MODE_CFG) return;

  // Enviando estado via LoRa
  LoRa.beginPacket();

  // Crio buffer para colocar dados LoRa
  char lora_data[LF_LORA_MAX_PACKET_SIZE];

  // Formato pacote LoRa
  loraEncode(sRet.c_str(), sRet.length(), lora_data);

  // Enviando LoRa
  LoRa.print(lora_data);

  LoRa.endPacket();

  // entro no modo "receive"
  LoRa.receive();

} /* sendNegotiation */

/* -------------------------------------------------------------------------- */
boolean LF_LoRaClass::loopLora() {

  // Tentando analisar o pacote
  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    if (packetSize > LF_LORA_MAX_PACKET_SIZE) {
      if (_debugEnabeld) {
        Serial.println("ESTOUROU TAMANHO DO PACOTE!");
      }
      return false;
    }

    // Lendo o pacote
    String LoraData = "";

    while (LoRa.available()) {
      LoraData += (char)LoRa.read();
    }

    // Lendo o lastRSSI
    _rssi = LoRa.packetRssi();

    // Crio buffer para receber a mensagem processada
    char msg_data[packetSize];

    if (_opMode == LORA_OP_MODE_LOOP) {

      int res = loraCheckMsg(LoraData.c_str(), LoraData.length(), msg_data);

      if (res==LORA_MSG_CHECK_OK) {
        // está OK, trato a mensagem
        _lastIdRec = _lastRegRec.id;
        String sMsg = String(msg_data);
        if (_debugEnabeld) {
          Serial.println("Msg ID: " + String(_lastIdRec) + " Msg: " + sMsg);
          Serial.println("Recebendo Msg: " + sMsg);
          Serial.print("RSSI: "); Serial.println(String(_rssi, DEC));
        }
        // Trato o comando (calback)
        if (execMsgModeLoop)
          execMsgModeLoop(sMsg);

        // Atualizo dados para atualizar display
        _lastMsg = sMsg;

        return true;

      } else {

        if (_debugEnabeld) {
          Serial.println("Msg não OK, retorno: " + String(res));
        }

      }

    }
    if (_opMode == LORA_OP_MODE_CFG) {

      if (loraDecode(LoraData.c_str(), LoraData.length(), msg_data)) {

        String sMsg = String(msg_data);
        if (_debugEnabeld) {
          Serial.println("Msg Cfg: " + sMsg);
        }
        if (sMsg.substring(0,1).equals(String("!")))
          execMsgModeCfg(sMsg.substring(1));

      }

    }

  }

  return false;

} /* loopLora */

/* -------------------------------------------------------------------------- */
int LF_LoRaClass::lastRssi() {
  return _rssi;
} /* lastRssi */

/* -------------------------------------------------------------------------- */
String LF_LoRaClass::lastMsg() {
  return _lastMsg;
} /* lastMsg */

/* -------------------------------------------------------------------------- */
uint8_t LF_LoRaClass::lastIdRec() {
  return _lastIdRec;
} /* lastIdRec */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::sendState(String sState, bool ret) {

  if (_opMode != LORA_OP_MODE_LOOP) return;

  // Enviando estado via LoRa
  LoRa.beginPacket();

  // Crio buffer para colocar dados LoRa
  char lora_data[LF_LORA_MAX_PACKET_SIZE];

  // Formato pacote LoRa como resposta (retorno o ID recebido se retorno = true)
  if (ret) {
    loraAddHeaderRet(sState.c_str(), sState.length(), _masterAddr, _lastIdRec, lora_data);
  } else {
    loraAddHeader(sState.c_str(), sState.length(), _masterAddr, lora_data);
  }

  // Enviando LoRa
  LoRa.print(lora_data);

  LoRa.endPacket();

  // entro no modo "receive"
  LoRa.receive();

  if (_debugEnabeld) {
    Serial.print("Dado LoRa: "); Serial.println(lora_data);
    Serial.print("Estado: "); Serial.println(sState);
    Serial.print("Millis: "); Serial.println(millis());
    Serial.print(" Tamanho: "); Serial.println(sState.length());
  }

} /* sendState */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::loopBtnLed() {

  // Avaliando o Btn
  btnCheck();

  // Loop Btn
  if (_btnCfgActive) {
    _btnCfgActive = false;
    // Comutando o modo de operação para CFG
    if (_opMode == LORA_OP_MODE_LOOP) {
      setOpMode(LORA_OP_MODE_CFG);
    }
  }

  // Loop LED
  if (_opMode == LORA_OP_MODE_CFG) {
    if (getDeltaMillis(_lastLedTime) > LED_CICLE_TIME) {
      _lastLedTime = millis();
      if ((digitalRead(_ledPin)) xor _ledInverted) {
        digitalWrite(_ledPin, _ledInverted ? HIGH : LOW);
      } else {
        digitalWrite(_ledPin, _ledInverted ? LOW : HIGH);
      }
    }
  } else {
    // Apago o LED quando termina a configuração
    if (_lastModoOp = LORA_OP_MODE_CFG) {
      _lastModoOp = LORA_OP_MODE_LOOP;
      digitalWrite(_ledPin, _ledInverted ? HIGH : LOW);
    }
  }

} /* loopBtnLed */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::btnCheck() {

  _btnState = digitalRead(_btnPin) == _ledInverted ? HIGH : LOW;

  if (_btnState && !_lastBtnState) {
    int64_t debounceDelay = getDeltaMillis(_lastBtnDebounceTime);
    if (debounceDelay < BTN_DEBOUNCE_TIME)
      return;
    _lastBtnState = true;
    _btnSet = true;
    _btnCfg = false;
    _lastBtnConfigTime = millis();
  }
  if (_btnState && _lastBtnState) {
    int64_t configDelay = getDeltaMillis(_lastBtnConfigTime);
    if (configDelay < BTN_CONFIG_TIME)
      return;
    _btnCfg = true;
  }
  if (!_btnState && _lastBtnState) {
    _lastBtnState = false;
    _lastBtnDebounceTime = millis();
    if (_btnCfg) {
      _btnCfg = false;
      _btnCfgActive = true;
      return;
    }
    if (_btnSet) {
      _btnSet = false;
      _btnUsrActive = true;
      return;
    }
  }

} /* btnCheck */

/* -------------------------------------------------------------------------- */
boolean LF_LoRaClass::isBtnActive() {
  return _btnUsrActive;
} /* clearBtnActive */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::clearBtnActive() {
  _btnUsrActive = false;
} /* clearBtnActive */

/* -------------------------------------------------------------------------- */
int64_t LF_LoRaClass::getDeltaMillis(unsigned long lastTime) {

  // Avalia o tempo, considerando o overflow do millis()...
  // Usando int64_t para lidar com o overflow...
  unsigned long auxMillis = millis();
  int64_t deltaTime = auxMillis;
  // Tratando o overload de millis, que faria auxMillis < lastTime
  if (auxMillis < lastTime) {
    deltaTime = auxMillis + 0xFFFFFFFF;
  }
  deltaTime = deltaTime - lastTime;

  return deltaTime;

} /* clearBtnActive */


/* Defino a variável Globla LF_LoRa aqui, para não ter que declarar no .ino */
LF_LoRaClass LF_LoRa;
