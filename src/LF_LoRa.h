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
#define LORA_OP_MODE_CFG     0   // Modo de configuração
#define LORA_OP_MODE_LOOP    1   // Modo loop de mensagens

#define LORA_STEP_NEG_INIC   0   // Fase da negociação do LoRa2MQTT - Inicial
#define LORA_STEP_NEG_CFG    1   // Fase da negociação do LoRa2MQTT - Recebe Configuração
#define LORA_STEP_NEG_FIM    2   // Fase da negociação do LoRa2MQTT - Final - Salva e Muda Modo

// Pinos do lora (comunicação spi) padrão
#define LORA_RST_PIN_DEF    14
#define LORA_SS_PIN_DEF     18
#define LORA_SCK_PIN_DEF     5
#define LORA_MOSI_PIN_DEF   27
#define LORA_MISO_PIN_DEF   19
#define LORA_DI00_PIN_DEF   26

// Frequência de comunicação
#define LORA_FREQ_AS  433E6  // Asia
#define LORA_FREQ_EU  868E6  // Europe
#define LORA_FREQ_NA  915E6  // North America

// "sync word" padrão LoRa2MQTT = 0x11.  range de 0x00 - 0xFF
#define LORA_SYNC_WORD_DEF  0x11

#define LF_LORA_MAX_PACKET_SIZE  255

#define LORA_MSG_CHECK_OK            0
#define LORA_MSG_CHECK_NOT_MASTER    1
#define LORA_MSG_CHECK_NOT_ME        2
#define LORA_MSG_CHECK_ALREADY_REC   3
#define LORA_MSG_CHECK_ERROR         4

#define LED_CICLE_TIME        500

#define BTN_DEBOUNCE_TIME      20
#define BTN_CONFIG_TIME      5000


// Callbacks da Biblioteca
#define LF_LORA_EXEC_MSG_MODE_LOOP std::function<void(String)> execMsgModeLoop

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
  void btnLedCfg(uint8_t btn_pin, boolean btn_inverted, uint8_t led_pin, boolean led_inverted);
  void inic();
  void setDebugEnable(boolean debugEnable);
  LF_LoRaClass& setExecMsgModeLoop(LF_LORA_EXEC_MSG_MODE_LOOP);
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
  boolean loraDecode(const char *in, int len, char *out);
  uint8_t loraCheckMsg(const char *in, int len, char *out);
  uint8_t loraCheckMsgMaster(const char *in, int len, char *out);
  RegRec lastMsgHeader();
  uint8_t lastSendId();
  const char* descCheckMsg(int ret);
  void execMsgModeCfg(String sMsg);
  boolean loopLora();
  int lastRssi();
  String lastMsg();
  uint8_t lastIdRec();
  void sendState(String sState, bool ret);
  void loopBtnLed();
  boolean isBtnActive();
  void clearBtnActive();

  // LF_LoRaClass Private
  // --------------
private:

  // ## Methods
  LF_LORA_EXEC_MSG_MODE_LOOP;

  uint8_t loraCheckMsgIni(const char *in, int len, uint8_t &de, uint8_t &para, char *out);
  void addRegRec(uint8_t de, uint8_t para, uint8_t id);
  void removeRegRec(int index);
  void clearRegRecs();
  int findRegRec(uint8_t de, uint8_t para);
  void sendNegotiation(String sRet);
  void btnCheck();
  int64_t getDeltaMillis(unsigned long lastTime);

  // ## Variáveis
  const uint8_t lf_key[128] = { //
    0x20, 0x0B, 0x4C, 0x49, 0x10, 0x07, 0x48, 0x30,
    0x3B, 0x10, 0x17, 0x31, 0x42, 0x48, 0x41, 0x43,
    0x0E, 0x08, 0x08, 0x2E, 0x28, 0x3A, 0x0D, 0x35,
    0x2A, 0x15, 0x29, 0x0B, 0x10, 0x29, 0x1B, 0x0C,
    0x2E, 0x46, 0x14, 0x04, 0x06, 0x3C, 0x01, 0x42,
    0x1C, 0x1D, 0x29, 0x4D, 0x04, 0x46, 0x00, 0x01,
    0x45, 0x3A, 0x09, 0x45, 0x27, 0x12, 0x05, 0x2F,
    0x0C, 0x0D, 0x0D, 0x45, 0x03, 0x40, 0x26, 0x44,
    0x2B, 0x41, 0x28, 0x2C, 0x4A, 0x29, 0x31, 0x32,
    0x1D, 0x12, 0x1F, 0x21, 0x47, 0x1C, 0x2B, 0x23,
    0x33, 0x0B, 0x1B, 0x33, 0x1D, 0x2B, 0x29, 0x2D,
    0x33, 0x43, 0x20, 0x41, 0x46, 0x49, 0x02, 0x0A,
    0x39, 0x18, 0x3B, 0x4D, 0x08, 0x08, 0x15, 0x14,
    0x2A, 0x36, 0x33, 0x47, 0x4E, 0x38, 0x25, 0x4B,
    0x1E, 0x12, 0x47, 0x09, 0x26, 0x16, 0x32, 0x4D,
    0x22, 0x1A, 0x4E, 0x30, 0x19, 0x4C, 0x32, 0x38
  };

  const char *descCheckMsgLora[5] = {
    "OK",
    "Não é do Master",
    "Não é para mim",
    "Msg repetida",
    "Erro na Msg"
  };

  //########## Para Preferences
  Preferences pref;

  boolean _debugEnabeld = false;

  String _sMac;
  String _sLast6Mac;
  String _sModel;

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
  uint8_t _ledPin;
  boolean _btnInverted;
  boolean _ledInverted;
  unsigned long _lastBtnDebounceTime = 0;
  unsigned long _lastBtnConfigTime = 0;
  unsigned long _lastLedTime = 0;
  bool _btnState = false;
  bool _lastBtnState = false;
  bool _btnSet = false;
  bool _btnCfg = false;
  bool _btnUsrActive = false;
  bool _btnCfgActive = false;
  boolean _lastModoOp = LORA_OP_MODE_CFG;

};

/* Informo que a variável Global LF_LoRa foi criada em LF_LoRa.ccp
   e não será necessário definir no .ino
   Isso é util quando só é necessário uma instância de LF_LoRaClass */
extern LF_LoRaClass LF_LoRa;

#endif
