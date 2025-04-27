/*
 * Copyright (c) 2025 by Leonardo Figueiro <leoagfig@gmail.com>
 * LF_LoRa library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef	LF_LORA_H
#define	LF_LORA_H

#include <Arduino.h>
#include <stdio.h>

// WiFi para poder desabilitar
#include <WiFi.h>

// LoRa
#include <SPI.h>
#include <LoRa.h>

// Preferences para salvar na memória não volátil
#include "Preferences.h"

//########## Para LoRa
#define LORA_OP_MODE_PAIRING 0   // Modo de pareamento
#define LORA_OP_MODE_LOOP    1   // Modo loop de mensagens

#define LORA_STEP_NEG_INIC   0   // Fase da negociação do LoRa2MQTT - Inicial
#define LORA_STEP_NEG_CFG    1   // Fase da negociação do LoRa2MQTT - Recebe Configuração
#define LORA_STEP_NEG_FIM    2   // Fase da negociação do LoRa2MQTT - Final - Salva e Muda Modo

// Frequência de comunicação
#define LORA_FREQ_AS  433E6  // Asia
#define LORA_FREQ_EU  868E6  // Europe
#define LORA_FREQ_NA  915E6  // North America

// "sync word" range de 0x00 - 0xFF
#define LORA_SYNC_WORD_DEF   0xE6

#define LF_LORA_MAX_PACKET_SIZE  255

#define LORA_MSG_CHECK_OK            0
#define LORA_MSG_CHECK_NOT_MASTER    1
#define LORA_MSG_CHECK_NOT_ME        2
#define LORA_MSG_CHECK_ALREADY_REC   3
#define LORA_MSG_CHECK_ERROR         4

#define LED_CICLE_TIME        500
#define LED_MIN_BRIGHTNESS     26

#define BTN_DEBOUNCE_TIME      20
#define BTN_ON_TIME          1000
#define BTN_OFF_TIME          400
#define BTN_LONG_TIME        3000

// Callbacks da Biblioteca
#define LF_LORA_ON_EXEC_MSG_MODE_LOOP std::function<void(String, bool)> onExecMsgModeLoop
#define LF_LORA_ON_LED_CHECK std::function<bool()> onLedCheck
#define LF_LORA_ON_LED_TURN_ON_PAIRING std::function<void()> onLedTurnOnPairing
#define LF_LORA_ON_LED_TURN_OFF_PAIRING std::function<void()> onLedTurnOffPairing

struct RegRec {
  uint8_t de;
  uint8_t para;
  uint8_t id;
};

class LF_LoRaClass {

  // LF_LoRaClass Public
  // -------------
public:

  // ## Methods
  LF_LoRaClass();

  void hardwareCfg(uint8_t rstPin, uint8_t ssPin, uint8_t sckPin, uint8_t mosiPin, uint8_t misoPin, uint8_t di00Pin);
  void slaveCfg(String model);
  void btnCfg(uint8_t btn_pin, bool btn_inverted);
  void inic();
  void setDebugEnable(bool debugEnable);
  LF_LoRaClass& setOnExecMsgModeLoop(LF_LORA_ON_EXEC_MSG_MODE_LOOP);
  LF_LoRaClass& setOnLedCheck(LF_LORA_ON_LED_CHECK);
  LF_LoRaClass& setOnLedTurnOnPairing(LF_LORA_ON_LED_TURN_ON_PAIRING);
  LF_LoRaClass& setOnLedTurnOffPairing(LF_LORA_ON_LED_TURN_OFF_PAIRING);
  void setFrequency(long frequency);
  void setSyncWord(int synch);
  uint8_t myAddr();
  void setMyAddr(uint8_t addr);
  uint8_t masterAddr();
  void setMasterAddr(uint8_t addr);
  uint8_t opMode();
  void setOpMode(uint8_t modo);
  void loraEncode(const char *in, int len, char *out);
  void loraAddHeader(const char *in, int len, uint8_t para, char *out);
  void loraAddHeaderRet(const char *in, int len, uint8_t para, uint8_t id, char *out);
  bool loraDecode(const char *in, int len, char *out);
  uint8_t loraCheckMsg(const char *in, int len, char *out);
  uint8_t loraCheckMsgMaster(const char *in, int len, char *out);
  RegRec lastMsgHeader();
  uint8_t lastSendId();
  void execMsgModePairing(String sMsg);
  bool loopLora();
  int lastRssi();
  String lastMsg();
  uint8_t lastIdRec();
  void sendState(String sState, bool ret);
  void loopBtnLed();
  bool isBtnClickActive();
  bool isBtnDblClickActive();
  bool isBtnLongActive();
  bool isBtnOn();
  int64_t getDeltaMillis(unsigned long lastTime);

  // LF_LoRaClass Private
  // --------------
private:

  // ## Methods
  LF_LORA_ON_EXEC_MSG_MODE_LOOP;
  LF_LORA_ON_LED_CHECK;
  LF_LORA_ON_LED_TURN_ON_PAIRING;
  LF_LORA_ON_LED_TURN_OFF_PAIRING;

  uint8_t loraCheckMsgIni(const char *in, int len, uint8_t &de, uint8_t &para, char *out);
  void addRegRec(uint8_t de, uint8_t para, uint8_t id);
  void removeRegRec(int index);
  void clearRegRecs();
  int findRegRec(uint8_t de, uint8_t para);
  void sendNegotiation(String sRet);
  void btnCheck();

  // ## Variables
  Preferences pref;

  bool _debugEnabeld = false;

  String _sMac;
  String _sLast6Mac;
  String _sModel;

  uint8_t _netId = 0;
  uint8_t _myAddr = 0;
  uint8_t _masterAddr = 0;
  uint8_t _lastSendId = 255;
  RegRec *_regRecs = nullptr;
  int _regRecsLen = 0;
  RegRec _lastRegRec = {0, 0, 0};

  uint8_t _opMode = LORA_OP_MODE_LOOP;
  uint8_t _stepNegotiation = LORA_STEP_NEG_INIC;

  uint8_t _lastIdRec;
  String _lastMsg;
  int _rssi;

  uint8_t _loraRstPin;
  uint8_t _loraSsPin;
  uint8_t _loraSckPin;
  uint8_t _loraMosiPin;
  uint8_t _loraMisoPin;
  uint8_t _loraDi00Pin;

  long _loraFrequency = LORA_FREQ_NA;
  int _syncWord = LORA_SYNC_WORD_DEF;

  uint8_t _btnPin;
  bool _btnInverted;
  bool _btnEnabled = false;
  unsigned long _lastLedTime = 0;
  unsigned long _lastBtnDebounceTime = 0;
  unsigned long _lastBtnOnTime = 0;
  unsigned long _lastBtnOffTime = 0;
  unsigned long _lastBtnLongTime = 0;
  bool _btnState = false;
  bool _lastBtnState = false;
  bool _btnClick = false;
  bool _btnLong = false;
  bool _btnClickActive = false;
  bool _btnDblClickActive = false;
  bool _btnLongActive = false;
  bool _btnCfgActive = false;
  uint8_t _btnCounter = 0;
  bool _lastModoOp = LORA_OP_MODE_LOOP;

};

/* Informo que a variável Global LF_LoRa foi criada em LF_LoRa.ccp
   e não será necessário definir no .ino
   Isso é util quando só é necessário uma instância de LF_LoRaClass */
extern LF_LoRaClass LF_LoRa;

#endif
