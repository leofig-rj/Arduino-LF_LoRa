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

// Gerais
//bool vIsDebugEnabled; // Para funcionar em serverSSDP, não pode ser variável da classe...

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
  _opMode = LORA_OP_MODE_PAIRING;
  _stepNegotiation = LORA_STEP_NEG_INIC;
} /* slaveCfg */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::btnCfg(uint8_t btn_pin, bool btn_inverted) {
  _btnPin = btn_pin;
  _btnInverted = btn_inverted;
  _btnEnabled = true;
  if (_btnInverted) {
    pinMode(_btnPin, INPUT_PULLUP);
  } else {
    pinMode(_btnPin, INPUT);
  }
} /* btnCfg */

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
  // Pego os valores de opMode, masterAddr e myAddr, se a chave não existrir, retorna o valor default.
  // Nota: O nome da chave é limitado a 15 caracteres.
  _opMode = pref.getUInt("opMode", LORA_OP_MODE_PAIRING);
  _netId = pref.getUInt("netId", 0);
  _masterAddr = pref.getUInt("masterAddr", 0);
  _myAddr = pref.getUInt("myAddr", 0);
  // Fecho Preferences
  pref.end();

  setOpMode(_opMode);

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
void LF_LoRaClass::setDebugEnable(bool debugEnable)
{
  _debugEnabeld = debugEnable;
} /* setDebugEnable */

/* -------------------------------------------------------------------------- */
LF_LoRaClass& LF_LoRaClass::setOnExecMsgModeLoop(LF_LORA_ON_EXEC_MSG_MODE_LOOP) {
  this->onExecMsgModeLoop = onExecMsgModeLoop;
  return *this;
} /* setOnExecMsgModeLoop */

/* -------------------------------------------------------------------------- */
LF_LoRaClass& LF_LoRaClass::setOnLedCheck(LF_LORA_ON_LED_CHECK) {
  this->onLedCheck = onLedCheck;
  return *this;
} /* setOnLedCheck */

/* -------------------------------------------------------------------------- */
LF_LoRaClass& LF_LoRaClass::setOnLedTurnOnPairing(LF_LORA_ON_LED_TURN_ON_PAIRING) {
  this->onLedTurnOnPairing = onLedTurnOnPairing;
  return *this;
} /* setOnLedTurnOnPairing */

/* -------------------------------------------------------------------------- */
LF_LoRaClass& LF_LoRaClass::setOnLedTurnOffPairing(LF_LORA_ON_LED_TURN_OFF_PAIRING) {
  this->onLedTurnOffPairing = onLedTurnOffPairing;
  return *this;
} /* setOnLedTurnOffPairing */

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
  if (_opMode ==LORA_OP_MODE_PAIRING) {
    _stepNegotiation = LORA_STEP_NEG_INIC;
    _lastModoOp = LORA_OP_MODE_PAIRING;
  }
} /* setOpMode */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::loraEncode(const char *in, int len, char *out)
{
  for (uint8_t i = 0; i < len; i++) {
    out[i] = in[i];
  }
  out[len] = 0;
} /* loraEncode */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::loraAddHeader(const char *in, int len, uint8_t para, char *out) {
  _lastSendId++;
  loraAddHeaderRet(in, len, para, _lastSendId, out);
} /* loraAddHeader */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::loraAddHeaderRet(const char *in, int len, uint8_t para, uint8_t id, char *out) {
  char aux[len + 12]; // Buffer aux para inserir cabeçalho
  sprintf(aux, "%02X%02X%02X%02X%04X", _netId, _myAddr, para, id, len + 12);
  // Completo com msg de entrada
  for (uint8_t i = 0; i < len; i++) {
    aux[i+12] = in[i];
  }
  loraEncode(aux, len + 12, out);
} /* loraAddHeader */

/* -------------------------------------------------------------------------- */
bool LF_LoRaClass::loraDecode(const char *in, int len, char *out)
{
  for (uint8_t i = 0; i < len; i++) {
    out[i] = in[i];
  }
  out[len] = 0;
  return true;
} /* loraDecode */

/* -------------------------------------------------------------------------- */
uint8_t LF_LoRaClass::loraCheckMsgIni(const char *in, int len, uint8_t &de, uint8_t &para, char *out)
{

  uint8_t net;
  uint8_t id;
  int index;

  // Buffer aux para decodificação
  char aux[LF_LORA_MAX_PACKET_SIZE];

  if (!loraDecode(in, len, aux)) {
    return LORA_MSG_CHECK_ERROR; // erro nos dados
  }

  // Agora testo o buffer decodificado
  for (uint8_t i = 0; i < 12; i++) {
    if (!isxdigit(aux[i])) {
      // Não é HEX
      out[0] = 0; // Retorna nulo
      return LORA_MSG_CHECK_ERROR; // erro nos dados
    }
  }

  // Analiso o buffer aux...
  char sub[4 + 1]; // +1 para o caractere nulo '\0'
  // Pego Net
  strncpy(sub, aux + 0, 2);
  sub[2] = '\0';
  net = strtol(sub, NULL, 16); // Base 16
  // Pego DE
  strncpy(sub, aux + 2, 2);
  sub[2] = '\0';
  de = strtol(sub, NULL, 16); // Base 16
  // Pego PARA
  strncpy(sub, aux + 4, 2);
  sub[2] = '\0';
  para = strtol(sub, NULL, 16); // Base 16
  // Pego ID
  strncpy(sub, aux + 6, 2);
  sub[2] = '\0';
  id = strtol(sub, NULL, 16); // Base 16
  // Pego LEN
  strncpy(sub, aux + 8, 4);
  sub[4] = '\0';
  int len_in_msg = strtol(sub, NULL, 16); // Base 16
  // Testo NetId
  if (net != _netId) {
    out[0] = 0; // Retorna nulo
    return LORA_MSG_CHECK_ERROR; // erro nos dados
  }
  // Testo LEN
  if (len_in_msg != len) {
    out[0] = 0; // Retorna nulo
    return LORA_MSG_CHECK_ERROR; // erro nos dados
  }
  // Separo a msg limpa
  for (uint8_t i = 0; i < len-12; i++) {
    out[i] = aux[i+12];
  }
  out[len-12] = 0;

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
void LF_LoRaClass::execMsgModePairing(String sMsg) {

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
        // Retardo aleatóriamente para enviar
        uint8_t t = random(0, 200);
        delay(t);
        if (_debugEnabeld) {
          Serial.println(sRet);
        }
        sendNegotiation(sRet);
        // Retardo aleatóriamente para enviar
        t = random(400, 600);
        delay(t);
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
    if (sMsg.length() != 29) return;
    if (!sMsg.substring(17,18).equals(String("!"))) return;
    if (!sMsg.substring(21,22).equals(String("!"))) return;
    if (!sMsg.substring(25,26).equals(String("!"))) return;
    if (!sDe.equals(String("FFFFFF"))) return;
    if ((sPara.equals(_sLast6Mac)) && (sCmd.equals(String("101")))) {
      String sNetId = sMsg.substring(18,21);
      String sAddrM = sMsg.substring(22,25);
      String sAddrE = sMsg.substring(26,29);
      String sRet = "!FFFFFF!" + _sLast6Mac + "!101!" + sNetId + "!" + sAddrM + "!" + sAddrE;
      if (_debugEnabeld) {
        Serial.println(sRet);
      }
      sendNegotiation(sRet);
      // Retardo aleatóriamente para enviar
      uint8_t t = random(400, 600);
      delay(t);
      if (_debugEnabeld) {
        Serial.println(sRet);
      }
      sendNegotiation(sRet);
      _stepNegotiation = LORA_STEP_NEG_FIM;
      _netId = sNetId.toInt();
      _masterAddr = sAddrM.toInt();
      _myAddr = sAddrE.toInt();
      setOpMode(LORA_OP_MODE_LOOP);
      if (_btnEnabled == true) {
        // Terminou a configuração... desligando o LED
        if (onLedTurnOffPairing)
          onLedTurnOffPairing();
      }
      // Abro Preferences com o nomespace "LoRa"
      pref.begin("LoRa", false);
      // Salvo _opMode, _masterAddr e _myAddr na memória não volátil com os nomes das chaves "opMode", "masterAdd" e "myAddr"
      pref.putUInt("opMode", _opMode);
      pref.putUInt("netId", _netId);
      pref.putUInt("masterAddr", _masterAddr);
      pref.putUInt("myAddr", _myAddr);
      // Fecho Preferences
      pref.end();

      return;
    }
  }

} /* execMsgModePairing */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::sendNegotiation(String sRet) {

  if (_opMode != LORA_OP_MODE_PAIRING) return;

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
bool LF_LoRaClass::loopLora() {

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
        if (onExecMsgModeLoop)
          onExecMsgModeLoop(sMsg);

        // Atualizo dados para atualizar display
        _lastMsg = sMsg;

        return true;

      } else {

        if (_debugEnabeld) {
          Serial.println("Msg não OK, retorno: " + String(res));
        }

      }

    }
    if (_opMode == LORA_OP_MODE_PAIRING) {

      if (loraDecode(LoraData.c_str(), LoraData.length(), msg_data)) {

        String sMsg = String(msg_data);
        if (_debugEnabeld) {
          Serial.println("Msg Cfg: " + sMsg);
        }
        if (sMsg.substring(0,1).equals(String("!")))
          execMsgModePairing(sMsg.substring(1));

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

  // Só executa se estiver habilitado
  if (_btnEnabled == false) return;

  // Avaliando o Btn
  btnCheck();

  // Loop Btn
  if (_btnCfgActive) {
    _btnCfgActive = false;
    // Comutando o modo de operação para CFG
    if (_opMode == LORA_OP_MODE_LOOP) {
      setOpMode(LORA_OP_MODE_PAIRING);
    }
  }

  // Loop LED

  // Vejo se os calbacks estão configurados
  if (!onLedCheck) return;
  if (!onLedTurnOnPairing) return;
  if (!onLedTurnOffPairing) return;

  if (_opMode == LORA_OP_MODE_PAIRING) {
    if (getDeltaMillis(_lastLedTime) > LED_CICLE_TIME) {
      _lastLedTime = millis();
      if (onLedCheck()) {
        onLedTurnOffPairing();
      } else {
        onLedTurnOnPairing();
      }
    }
  } else {
    // Apago o LED quando termina a configuração
    if (_lastModoOp = LORA_OP_MODE_PAIRING) {
      _lastModoOp = LORA_OP_MODE_LOOP;
      onLedTurnOffPairing();
    }
  }

} /* loopBtnLed */

/* -------------------------------------------------------------------------- */
void LF_LoRaClass::btnCheck() {

  _btnState = digitalRead(_btnPin) xor _btnInverted;

  if (_btnState && !_lastBtnState) {
    int64_t debouceDelay = getDeltaMillis(_lastBtnDebounceTime);
    if (debouceDelay < BTN_DEBOUNCE_TIME)
      return;
    _lastBtnState = true;
    _btnCounter ++;
    _btnClick= true;
    _btnLong = false;
    _lastBtnOnTime = millis();
    _lastBtnLongTime = millis();
  }
  if (_btnState && _lastBtnState) {
    _lastBtnDebounceTime = millis();
    int64_t onDelay = getDeltaMillis(_lastBtnOnTime);
    if (onDelay < BTN_ON_TIME)
      return;
    _btnClick= false;
    _btnLong = true;
    int64_t delay = getDeltaMillis(_lastBtnLongTime);
    if (delay < BTN_LONG_TIME)
      return;
    _btnLong = false;
  }
  if (!_btnState && _lastBtnState) {
    int64_t debouceDelay = getDeltaMillis(_lastBtnDebounceTime);
    if (debouceDelay < BTN_DEBOUNCE_TIME)
      return;
    _lastBtnState = false;
    _lastBtnOffTime = millis();
  }
  if (!_btnState && !_lastBtnState) {
    _lastBtnDebounceTime = millis();
    int64_t offDelay = getDeltaMillis(_lastBtnOffTime);
    if (offDelay < BTN_OFF_TIME)
      return;
    if (_btnClick) {
      if (_btnCounter == 1) {
        _btnClickActive = true;
      } else if (_btnCounter == 2) {
        _btnDblClickActive = true;
      } else if (_btnCounter >= 5) {
        _btnCfgActive = true;
      }
    }
    if (_btnLong) {
      _btnLongActive = true;
    }
    _btnClick = 0,
    _btnLong = 0;
    _btnCounter = 0;
  }

} /* btnCheck */

/* -------------------------------------------------------------------------- */
bool LF_LoRaClass::isBtnClickActive() {
  bool ret = _btnClickActive;
  _btnClickActive = false;
  return ret;
} /* isBtnClickActive */

/* -------------------------------------------------------------------------- */
bool LF_LoRaClass::isBtnDblClickActive() {
  bool ret = _btnDblClickActive;
  _btnDblClickActive = false;
  return ret;
} /* isBtnDblClickActive */

/* -------------------------------------------------------------------------- */
bool LF_LoRaClass::isBtnLongActive() {
  bool ret = _btnLongActive;
  _btnLongActive = false;
  return ret;
} /* isBtnLongActive */

/* -------------------------------------------------------------------------- */
bool LF_LoRaClass::isBtnOn() {
  return _btnState && !_btnClick;
} /* isBtnOn */

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

} /* getDeltaMillis */

/* Defino a variável Globla LF_LoRa aqui, para não ter que declarar no .ino */
LF_LoRaClass LF_LoRa;
