/*
  ------------------------------------------------------------------
                            CG scale
                      (c) 2019-2020 by M. Lehmann
  ------------------------------------------------------------------
*/
#define CGSCALE_VERSION "2.22"
/*

  ******************************************************************
  history:
  V2.22   28.11.20     fixed RAM problems with JSON
  V2.21   27.11.20     bug fixed: recompiled, binary file incorrect
  V2.2    01.11.20     Virtual weights built in
  V2.12   07.10.20     bug fixed: LR value was displayed in the wrong display position
                       Voltage for specified battery types deleted
  V2.11   18.08.20     code is now compatible with standard OLED displays
                       and original code base (default pw length = 32)
  V2.1    18.07.20     added support for ESP8266 based Wifi Kit 8
  (by Pulsar07/           (https://heltec.org/project/wifi-kit-8/)
   R.Stransky             is a ESP8266 with
                            a build in OLED 128x32
                            battery connector with charging management
                            reset and GPIO0 button
                        support for a tare button (PIN_TARE_BUTTON)
                        bug fixed: wifi password now with up to 64 chars
                        bug fixed: wifi data (ssid/passwd) with special
                          character (e.g. +) is now supported
                        for specified battery type, voltage is displayed
                        using uncompressed html files makes WEB GUI much faster
  V2.01   29.01.20      small bug fixes with AVR
  V2.0    26.01.20      Webpage rewritten, no bootstrap framework needed
                        add translation to webpage (en, de)
                        optimized for measuring with landinggears
                        updated to ArduinoJson V6
                        firmware update over web interface
  V1.2.1  31.03.19      small bug fixed
                        values in model database are rounded
                        mDNS and OTA did not work in AP mode
  V1.2    23.02.19      Add OTA (over the air update)
                        mDNS default enabled
                        add percentlists for many battery types
                        memory optimization
  V1.1    02.02.19      Supports ESP8266, webpage integrated, STA and AP mode
  V1.0    12.01.19      first release


  ******************************************************************

  Software License Agreement (BSD License)

  Copyright (c) 2019-2020, Michael Lehmann
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  3. Neither the name of the copyright holders nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// **** Please UNCOMMENT to choose special hardware *****************

//#define WIFI_KIT_8    //is a ESP8266 based board, with integrated OLED and battery management

// ******************************************************************

// Required libraries, can be installed from the library manager
#include <HX711_ADC.h>      // library for the HX711 24-bit ADC for weight scales (https://github.com/olkal/HX711_ADC)
#include <U8g2lib.h>        // Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

// built-in libraries
#include <EEPROM.h>
#include <Wire.h>

// libraries for ESP8266
#if defined(ESP8266)
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ElegantOTA.h>
#include <ArduinoJson.h>
#endif

// load settings
#if defined(__AVR__)
#include "settings_AVR.h"
#elif defined(ESP8266)
  #ifdef WIFI_KIT_8
    #include "settings_WIFI_KIT_8.h"
  #else
    #include "settings_ESP8266.h"
  #endif
#endif

// HX711 constructor array (dout pin, sck pint):
HX711_ADC LoadCell[] {HX711_ADC(PIN_LOADCELL1_DOUT, PIN_LOADCELL1_PD_SCK), HX711_ADC(PIN_LOADCELL2_DOUT, PIN_LOADCELL2_PD_SCK), HX711_ADC(PIN_LOADCELL3_DOUT, PIN_LOADCELL3_PD_SCK)};

// webserver constructor
#if defined(ESP8266)
ESP8266WebServer server(80);
IPAddress apIP(ip[0], ip[1], ip[2], ip[3]);
WiFiClientSecure httpsClient;
File fsUploadFile;              // a File object to temporarily store the received file
#endif

#include "defaults.h"

struct VirtualWeight {
  String name;
  float cg;
  float weight;
  bool enabled = false;
};

struct Model {
  float distance[3] = {DISTANCE_X1, DISTANCE_X2, DISTANCE_X3};
#if defined(ESP8266)
  char name[MAX_MODELNAME_LENGHT + 1] = "";
  float targetCGmin = 0;
  float targetCGmax = 0;
  uint8_t mechanicsType = 0;
  VirtualWeight virtualWeight[MAX_VIRTUAL_WEIGHT];
#endif
};

Model model;

// load default values
uint8_t nLoadcells = NUMBER_LOADCELLS;
float calFactorLoadcell[] = {LOADCELL1_CALIBRATION_FACTOR, LOADCELL2_CALIBRATION_FACTOR, LOADCELL3_CALIBRATION_FACTOR};
float resistor[] = {RESISTOR_R1, RESISTOR_R2};
uint8_t batType = BAT_TYPE;
uint8_t batCells = BAT_CELLS;
float refWeight = REF_WEIGHT;
float refCG = REF_CG;
#if defined(ESP8266)
char ssid_STA[MAX_SSID_PW_LENGHT + 1] = SSID_STA;
char password_STA[MAX_SSID_PW_LENGHT + 1] = PASSWORD_STA;
char ssid_AP[MAX_SSID_PW_LENGHT + 1] = SSID_AP;
char password_AP[MAX_SSID_PW_LENGHT + 1] = PASSWORD_AP;
bool enableUpdate = ENABLE_UPDATE;
bool enableOTA = ENABLE_OTA;
#endif

// declare variables
float weightLoadCell[] = {0, 0, 0};
float lastWeightLoadCell[] = {0, 0, 0};
float weightTotal = 0;
float CG_length = 0;
float CG_trans = 0;
float batVolt = 0;
unsigned long lastTimeMenu = 0;
unsigned long lastTimeLoadcell = 0;
bool updateMenu = true;
int menuPage = 0;
String errMsg[5];
int errMsgCnt = 0;
int oledDisplayHeight;
int oledDisplayWidth;
const uint8_t *oledFontLarge;
const uint8_t *oledFontNormal;
const uint8_t *oledFontSmall;
const uint8_t *oledFontTiny;
#if defined(ESP8266)
String updateMsg = "";
bool wifiSTAmode = true;
float gitVersion = -1;
#endif


// Restart CPU
#if defined(__AVR__)
void(* resetCPU) (void) = 0;
#elif defined(ESP8266)
void resetCPU() {}
#endif


// convert time to string
char * TimeToString(unsigned long t)
{
  static char str[13];
  int h = t / 3600000;
  t = t % 3600000;
  int m = t / 60000;
  t = t % 60000;
  int s = t / 1000;
  int ms = t - (s * 1000);
  sprintf(str, "%02ld:%02d:%02d.%03d", h, m, s, ms);
  return str;
}


// Count percentage from cell voltage
int percentBat(float cellVoltage) {

  int result = 0;
  int elementCount = DATAPOINTS_PERCENTLIST;
  byte batTypeArray = batType - 2;

  for (int i = 0; i < elementCount; i++) {
    if (pgm_read_float( &percentList[batTypeArray][i][1]) == 100 ) {
      elementCount = i;
      break;
    }
  }

  float cellempty = pgm_read_float( &percentList[batTypeArray][0][0]);
  float cellfull = pgm_read_float( &percentList[batTypeArray][elementCount][0]);

  if (cellVoltage >= cellfull) {
    result = 100;
  } else if (cellVoltage <= cellempty) {
    result = 0;
  } else {
    for (int i = 0; i <= elementCount; i++) {
      float curVolt = pgm_read_float(&percentList[batTypeArray][i][0]);
      if (curVolt >= cellVoltage && i > 0) {
        float lastVolt = pgm_read_float(&percentList[batTypeArray][i - 1][0]);
        float curPercent = pgm_read_float(&percentList[batTypeArray][i][1]);
        float lastPercent = pgm_read_float(&percentList[batTypeArray][i - 1][1]);
        result = float((cellVoltage - lastVolt) / (curVolt - lastVolt)) * (curPercent - lastPercent) + lastPercent;
        break;
      }
    }
  }

  return result;
}


void printConsole(int t, String msg) {
  Serial.print(TimeToString(millis()));
  Serial.print(" [");
  switch (t) {
    case T_BOOT:
      Serial.print("BOOT");
      break;
    case T_RUN:
      Serial.print("RUN");
      break;
    case T_ERROR:
      Serial.print("ERROR");
      break;
    case T_WIFI:
      Serial.print("WIFI");
      break;
    case T_UPDATE:
      Serial.print("UPDATE");
      break;
    case T_HTTPS:
      Serial.print("HTTPS");
      break;
  }
  Serial.print("] ");
  Serial.println(msg);
}


void initOLED() {
  oledDisplay.begin();
  oledDisplayHeight = oledDisplay.getDisplayHeight();
  oledDisplayWidth = oledDisplay.getDisplayWidth();
  printConsole(T_BOOT, "init OLED display: " + String(oledDisplayWidth) + String("x") + String(oledDisplayHeight));


  oledFontLarge  = u8g2_font_helvR12_tr;
  oledFontNormal = u8g2_font_helvR10_tr;
  oledFontSmall  = u8g2_font_5x7_tr;
  oledFontTiny   = u8g2_font_4x6_tr;

  if (oledDisplayHeight <= 32) {
    oledFontLarge  = u8g2_font_helvR10_tr;
    oledFontNormal = u8g2_font_6x12_tr;
  }
  int ylineHeight = oledDisplayHeight / 3;

  oledDisplay.setFont(oledFontNormal);

  oledDisplay.firstPage();
  do {
    oledDisplay.setFont(oledFontLarge);
    if (oledDisplayHeight <= 32) {
      oledDisplay.drawXBMP(5, 0, 18, 18, CGImage);
    } else {
      oledDisplay.drawXBMP(20, 12, 18, 18, CGImage);
    }
    oledDisplay.setFont(oledFontLarge);

    if (oledDisplayHeight <= 32) {
      oledDisplay.setCursor(30, 12);
    } else {
      oledDisplay.setCursor(45, 28);
    }
    oledDisplay.print(F("CG scale"));

    oledDisplay.setFont(oledFontSmall);
    if (oledDisplayHeight <= 32) {
      oledDisplay.setCursor(30, 22);
    } else {
      oledDisplay.setCursor(35, 55);
    }
    oledDisplay.print(F("Version: "));
    oledDisplay.print(CGSCALE_VERSION);
    if (oledDisplayHeight <= 32) {
      oledDisplay.setCursor(5, 31);
    } else {
      oledDisplay.setCursor(20, 64);
    }
    oledDisplay.print(F("(c) 2020 M.Lehmann"));

  } while ( oledDisplay.nextPage() );
}

//void printOLED(String aLine1, String aLine2, String aLine3 = String(""));

void printOLED(String aLine1, String aLine2, String aLine3) {
  int ylineHeight = oledDisplayHeight / 3;

  oledDisplay.firstPage();
  do {
    oledDisplay.setFont(oledFontNormal);
    oledDisplay.setCursor(0, ylineHeight * 1);
    oledDisplay.print(aLine1);
    oledDisplay.setCursor(0, ylineHeight * 2);
    oledDisplay.print(aLine2);
    oledDisplay.setCursor(0, oledDisplayHeight);
    oledDisplay.print(aLine3);
  } while ( oledDisplay.nextPage() );
}

void printScaleOLED() {
  // print to display
  char buff[8];
  int pos_weightTotal = 7;
  int pos_CG_length = 28;
  if (nLoadcells == 2) {
    pos_weightTotal = 17;
    pos_CG_length = 45;
    if (batType == 0) {
      pos_weightTotal = 12;
      pos_CG_length = 40;
    }
  }

  oledDisplay.firstPage();
  do {
    if (errMsgCnt == 0) {
      // print battery
      if (batType > B_OFF) {
        oledDisplay.drawXBMP(88, 1, 12, 6, batteryImage);
        if (batType == B_VOLT) {
          dtostrf(batVolt, 2, 2, buff);
        } else {
          dtostrf(batVolt, 3, 0, buff);
          oledDisplay.drawBox(89, 2, (batVolt / (100 / 8)), 4);
        }
        oledDisplay.setFont(oledFontSmall);
        oledDisplay.setCursor(123 - oledDisplay.getStrWidth(buff), 7);
        oledDisplay.print(buff);
        if (batType == B_VOLT) {
          oledDisplay.print(F("V"));
        } else {
          oledDisplay.print(F("%"));
        }
      }
 
      // print total weight
      oledDisplay.setFont(oledFontNormal);
      dtostrf(weightTotal, 5, 1, buff);
      if (oledDisplayHeight <= 32) {
        oledDisplay.setCursor(1, 18);
        oledDisplay.print(F("M  = "));
      } else {
        oledDisplay.drawXBMP(2, pos_weightTotal, 18, 18, weightImage);
        oledDisplay.setCursor(93 - oledDisplay.getStrWidth(buff), pos_weightTotal + 17);
      }
      oledDisplay.print(buff);
      oledDisplay.print(F(" g"));

      // print CG longitudinal axis
      dtostrf(CG_length, 5, 1, buff);
      if (oledDisplayHeight <= 32) {
        oledDisplay.setCursor(1, 32);
        oledDisplay.print(F("CG = "));
      } else {
        oledDisplay.drawXBMP(2, pos_CG_length, 18, 18, CGImage);
        oledDisplay.setCursor(93 - oledDisplay.getStrWidth(buff), pos_CG_length + 16);
      }
      oledDisplay.print(buff);
      oledDisplay.print(F(" mm"));

      // print CG transverse axis
      if (nLoadcells == 3) {
        if (oledDisplayHeight <= 32) {
          oledDisplay.setCursor(78, 32);
          oledDisplay.print(F("LR="));
          dtostrf(CG_trans, 3, 0, buff);
        } else {
          dtostrf(CG_trans, 5, 1, buff);
          oledDisplay.drawXBMP(2, 47, 18, 18, CGtransImage);
          oledDisplay.setCursor(93 - oledDisplay.getStrWidth(buff), 64);
        }
        oledDisplay.print(buff);
        oledDisplay.print(F(" mm"));
      }
    } else {
      oledDisplay.setFont(oledFontSmall);
      for (int i = 1; i <= errMsgCnt; i++) {
        oledDisplay.setCursor(0, 7 * i);
        oledDisplay.print(errMsg[i]);
      }
    }

  } while ( oledDisplay.nextPage() );
}


#ifdef PIN_TARE_BUTTON
void handleTareBtn() {
  static unsigned long lastTaraBtn = 0;
  if ((millis() - lastTaraBtn) > 20) {
    lastTaraBtn = millis();
    static int tareBtnCnt = 0;
    if (digitalRead(PIN_TARE_BUTTON)) {
      tareBtnCnt = 0;
    } else {
      tareBtnCnt++;
      if (tareBtnCnt > 10) {
        printOLED("TARE ==>", "  tare load cells ...");
        // avoid keybounce
        tareBtnCnt = -1000;
        tareLoadcells();
        delay(2000);
      }
    }
  }
}
#endif

// save calibration factor
void saveCalFactor(int nLC) {
  LoadCell[nLC].setCalFactor(calFactorLoadcell[nLC]);
  EEPROM.put(P_LOADCELL1_CALIBRATION_FACTOR + (nLC * sizeof(float)), calFactorLoadcell[nLC]);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void updateLoadcells() {
  for (int i = LC1; i <= LC3; i++) {
    if (i < nLoadcells) {
      LoadCell[i].update();
    }
  }
}


void tareLoadcells() {
  for (int i = LC1; i <= LC3; i++) {
    if (i < nLoadcells) {
      LoadCell[i].tare();
    }
  }
}


void printNewValueText() {
  Serial.print(F("Set new value:"));
}


// run auto calibration
bool runAutoCalibrate() {
  Serial.print(F("\nAutocalibration is running"));
  for (int i = 0; i <= 20; i++) {
    Serial.print(F("."));
    delay(100);
  }
  // calculate weight
  float toWeightLoadCell[] = {0, 0, 0};
  toWeightLoadCell[LC2] = ((refCG - model.distance[X1]) * refWeight) / model.distance[X2];
  toWeightLoadCell[LC1] = refWeight - toWeightLoadCell[LC2];
  if (nLoadcells == 3) {
    toWeightLoadCell[LC1] = toWeightLoadCell[LC1] / 2;
    toWeightLoadCell[LC3] = toWeightLoadCell[LC1];
  }
  // calculate calibration factors
  for (int i = LC1; i <= LC3; i++) {
    calFactorLoadcell[i] = calFactorLoadcell[i] / (toWeightLoadCell[i] / weightLoadCell[i]);
    saveCalFactor(i);
  }

  // finish
  Serial.println(F("done"));
}


// check if a loadcell has error
bool getLoadcellError() {
  bool err = false;

  for (int i = LC1; i <= LC3; i++) {
    if (i < nLoadcells) {
      if (LoadCell[i].getTareTimeoutFlag()) {
        String msg = "ERROR: Timeout TARE Lc" + String(i + 1);
        errMsg[++errMsgCnt] = msg + "\n";
#if defined(ESP8266)
        printConsole(T_ERROR, msg);
#endif
        err = true;
      }
    }
  }

  return err;
}


#if defined(ESP8266)

void writeModelData(JsonObject object) {
  char buff[8];
  String stringBuff;

  dtostrf(weightTotal, 5, 1, buff);
  stringBuff = buff;
  stringBuff.trim();
  object["wt"] = stringBuff;
  dtostrf(CG_length, 5, 1, buff);
  stringBuff = buff;
  stringBuff.trim();
  object["cg"] = stringBuff;
  dtostrf(CG_trans, 5, 1, buff);
  stringBuff = buff;
  stringBuff.trim();
  object["cglr"] = stringBuff;
  object["x1"] = model.distance[X1];
  object["x2"] = model.distance[X2];
  object["x3"] = model.distance[X3];
  object["cgmin"] = model.targetCGmin;
  object["cgmax"] = model.targetCGmax;
  object["mType"] = model.mechanicsType;

  JsonArray virtw = object.createNestedArray("virtual");
  for (int i=0; i < MAX_VIRTUAL_WEIGHT; i++){
    JsonArray virtWeight = virtw.createNestedArray();
    virtWeight.add(model.virtualWeight[i].name);
    virtWeight.add(model.virtualWeight[i].cg);
    virtWeight.add(model.virtualWeight[i].weight);
    virtWeight.add(model.virtualWeight[i].enabled);
  }
}


// save model to json file
bool saveModelJson(String modelName) {

  if (modelName.length() > MAX_MODELNAME_LENGHT) {
    return false;
  }

  DynamicJsonDocument jsonDoc(JSONDOC_SIZE);

  if (SPIFFS.exists(MODEL_FILE)) {
    // read json file
    File f = SPIFFS.open(MODEL_FILE, "r");
    auto error = deserializeJson(jsonDoc, f);
    f.close();
    if (error) {
      printConsole(T_ERROR, "save JSON: " + String(error.c_str()));
      return false;
    }
    // check if model exists
    if (jsonDoc.containsKey(modelName)) {
      writeModelData(jsonDoc[modelName]);
    } else {
      // otherwise create new
      writeModelData(jsonDoc.createNestedObject(modelName));
    }
    // write to file
    if (!error) {
      f = SPIFFS.open(MODEL_FILE, "w");
      serializeJson(jsonDoc, f);
      f.close();
    } else {
      printConsole(T_ERROR, "save JSON: " + String(error.c_str()));
      return false;
    }
  } else {
    // creat new json
    writeModelData(jsonDoc.createNestedObject(modelName));
    // write to file
    if (!jsonDoc.isNull()) {
      File f = SPIFFS.open(MODEL_FILE, "w");
      serializeJson(jsonDoc, f);
      f.close();
    } else {
      printConsole(T_ERROR, "JSON is null ");
      return false;
    }
  }

  return true;
}


// read model data from json file
bool openModelJson(String modelName) {

  DynamicJsonDocument jsonDoc(JSONDOC_SIZE);

  if (SPIFFS.exists(MODEL_FILE)) {
    // read json file
    File f = SPIFFS.open(MODEL_FILE, "r");
    auto error = deserializeJson(jsonDoc, f);
    f.close();
    if (error) {
      printConsole(T_ERROR, "open JSON: " + String(error.c_str()));
      return false;
    }
    // check if model exists
    if (jsonDoc.containsKey(modelName)) {
      // load parameters from model
      model.distance[X1] = jsonDoc[modelName]["x1"];
      model.distance[X2] = jsonDoc[modelName]["x2"];
      model.distance[X3] = jsonDoc[modelName]["x3"];
      model.targetCGmin = jsonDoc[modelName]["cgmin"];
      model.targetCGmax = jsonDoc[modelName]["cgmax"];
      model.mechanicsType = jsonDoc[modelName]["mType"];

      JsonArray virtw = jsonDoc[modelName]["virtual"];
      if(virtw){
        for (int i=0; i < MAX_VIRTUAL_WEIGHT; i++){
          model.virtualWeight[i].name = virtw[i][0].as<String>();
          model.virtualWeight[i].cg = virtw[i][1].as<int>();
          model.virtualWeight[i].weight = virtw[i][2].as<int>();
          model.virtualWeight[i].enabled = virtw[i][3].as<bool>();
        }
      }
    } else {
      printConsole(T_ERROR, "Model name not found");
      return false;
    }

    // save current model name to eeprom
    modelName.toCharArray(model.name, MAX_MODELNAME_LENGHT + 1);
    EEPROM.put(P_MODELNAME, model.name);
    EEPROM.commit();

    return true;
  }

  printConsole(T_ERROR, "Modelfile not exists");
  return false;
}


// delete model from json file
bool deleteModelJson(String modelName) {

  DynamicJsonDocument jsonDoc(JSONDOC_SIZE);

  if (SPIFFS.exists(MODEL_FILE)) {
    // read json file
    File f = SPIFFS.open(MODEL_FILE, "r");
    auto error = deserializeJson(jsonDoc, f);
    f.close();
    if (error) {
      printConsole(T_ERROR, "delete JSON: " + String(error.c_str()));
      return false;
    }
    // check if model exists
    if (jsonDoc.containsKey(modelName)) {
      jsonDoc.remove(modelName);
    } else {
      printConsole(T_ERROR, "Model name not found");
      return false;
    }
    // if no models in json, kill it
    if (jsonDoc.size() == 0) {
      SPIFFS.remove(MODEL_FILE);
    } else {
      // write to file
      if (!jsonDoc.isNull()) {
        File f = SPIFFS.open(MODEL_FILE, "w");
        serializeJson(jsonDoc, f);
        f.close();
      } else {
        printConsole(T_ERROR, "JSON is null ");
        return false;
      }
    }
    return true;
  }

  printConsole(T_ERROR, "Modelfile not exists");
  return false;
}


// send headvalues to client
void getHead() {
  String response = ssid_AP;
  response += "&";
  for (int i = 1; i <= errMsgCnt; i++) {
    response += errMsg[i];
  }
  response += "&";
  response += CGSCALE_VERSION;
  response += "&";
  response += gitVersion;
  server.send(200, "text/html", response);
}


// send values to client
void getValue() {
  char buff[8];
  String response = "";
  dtostrf(weightTotal, 5, 1, buff);
  response += buff;
  response += "g&";
  dtostrf(CG_length, 5, 1, buff);
  response += buff;
  response += "mm&";
  dtostrf(CG_trans, 5, 1, buff);
  response += buff;
  response += "mm&";
  if (batType == B_VOLT) {
    dtostrf(batVolt, 5, 2, buff);
    response += buff;
    response += "V";
  } else {
    dtostrf(batVolt, 5, 0, buff);
    response += buff;
    response += "%";
  }
  server.send(200, "text/html", response);
}


// send raw values to client
void getRawValue() {
  char buff[8];
  String response = "";
  dtostrf(weightLoadCell[LC1], 5, 1, buff);
  response += buff;
  response += "g&";
  dtostrf(weightLoadCell[LC2], 5, 1, buff);
  response += buff;
  response += "g&";
  dtostrf(weightLoadCell[LC3], 5, 1, buff);
  response += buff;
  response += "g&";
  if (batType == B_VOLT) {
    dtostrf(batVolt, 5, 2, buff);
    response += buff;
    response += "V";
  } else {
    dtostrf(batVolt, 5, 0, buff);
    response += buff;
    response += "%";
  }
  server.send(200, "text/html", response);
}


// send parameters to client
void getParameter() {
  char buff[8];
  String response = "";
  float weightTotal_saved = 0;
  float CG_length_saved = 0;
  float CG_trans_saved = 0;
  model.targetCGmin = 0;
  model.targetCGmax = 0;

  //StaticJsonDocument<JSONDOC_SIZE> jsonDoc;
  DynamicJsonDocument jsonDoc(JSONDOC_SIZE);

  if (SPIFFS.exists(MODEL_FILE)) {
    // read json file
    File f = SPIFFS.open(MODEL_FILE, "r");
    auto error = deserializeJson(jsonDoc, f);
    f.close();
    // check if model exists
    if (!error && jsonDoc.containsKey(model.name)) {
      weightTotal_saved = jsonDoc[model.name]["wt"];
      CG_length_saved = jsonDoc[model.name]["cg"];
      CG_trans_saved = jsonDoc[model.name]["cglr"];
      model.targetCGmin = jsonDoc[model.name]["cgmin"];
      model.targetCGmax = jsonDoc[model.name]["cgmax"];
      model.mechanicsType = jsonDoc[model.name]["mType"];
    }
  }

  // parameter list
  response += nLoadcells;
  response += "&";
  for (int i = X1; i <= X3; i++) {
    response += model.distance[i];
    response += "&";
  }
  response += refWeight;
  response += "&";
  response += refCG;
  response += "&";
  for (int i = LC1; i <= LC3; i++) {
    response += calFactorLoadcell[i];
    response += "&";
  }
  for (int i = R1; i <= R2; i++) {
    response += resistor[i];
    response += "&";
  }
  response += batType;
  response += "&";
  response += batCells;
  response += "&";
  response += ssid_STA;
  response += "&";
  response += password_STA;
  response += "&";
  response += ssid_AP;
  response += "&";
  response += password_AP;
  response += "&";
  response += model.name;
  response += "&";
  dtostrf(weightTotal_saved, 5, 1, buff);
  response += buff;
  response += "g&";
  dtostrf(CG_length_saved, 5, 1, buff);
  response += buff;
  response += "mm&";
  dtostrf(CG_trans_saved, 5, 1, buff);
  response += buff;
  response += "mm&";
  response += model.targetCGmin;
  response += "&";
  response += model.targetCGmax;
  response += "&";
  response += model.mechanicsType;
  response += "&";
  response += enableUpdate;
  response += "&";
  response += enableOTA;
  server.send(200, "text/html", response);
}


// send virtual weights to client
void getVirtualWeight() {
  String response = "";
  //StaticJsonDocument<JSONDOC_SIZE> jsonDoc;
  DynamicJsonDocument jsonDoc(JSONDOC_SIZE);

  JsonArray virtw = jsonDoc.createNestedArray("virtual");
  for (int i=0; i < MAX_VIRTUAL_WEIGHT; i++){
    JsonArray virtWeight = virtw.createNestedArray();
    virtWeight.add(model.virtualWeight[i].name);
    virtWeight.add(model.virtualWeight[i].cg);
    virtWeight.add(model.virtualWeight[i].weight);
    virtWeight.add(model.virtualWeight[i].enabled);
  }

  serializeJson(jsonDoc["virtual"], response);
  server.send(200, "text/html", response);
}


// send available WiFi networks to client
void getWiFiNetworks() {
  bool ssidSTAavailable = false;
  String response = "";
  int n = WiFi.scanNetworks();

  if (n > 0) {
    for (int i = 0; i < n; ++i) {
      response += WiFi.SSID(i);
      if (WiFi.SSID(i) == ssid_STA) ssidSTAavailable = true;
      if (i < n - 1) response += "&";
    }
    if (!ssidSTAavailable) {
      response += "&";
      response += ssid_STA;
    }
  }
  server.send(200, "text/html", response);
}


// save parameters
void saveParameter() {
  if (server.hasArg("nLoadcells")) nLoadcells = server.arg("nLoadcells").toInt();
  if (server.hasArg("distanceX1")) model.distance[X1] = server.arg("distanceX1").toFloat();
  if (server.hasArg("distanceX2")) model.distance[X2] = server.arg("distanceX2").toFloat();
  if (server.hasArg("distanceX3")) model.distance[X3] = server.arg("distanceX3").toFloat();
  if (server.hasArg("refWeight")) refWeight = server.arg("refWeight").toFloat();
  if (server.hasArg("refCG")) refCG = server.arg("refCG").toFloat();
  if (server.hasArg("calFactorLoadcell1")) calFactorLoadcell[LC1] = server.arg("calFactorLoadcell1").toFloat();
  if (server.hasArg("calFactorLoadcell2")) calFactorLoadcell[LC2] = server.arg("calFactorLoadcell2").toFloat();
  if (server.hasArg("calFactorLoadcell3")) calFactorLoadcell[LC3] = server.arg("calFactorLoadcell3").toFloat();
  if (server.hasArg("resistorR1")) resistor[R1] = server.arg("resistorR1").toFloat();
  if (server.hasArg("resistorR2")) resistor[R2] = server.arg("resistorR2").toFloat();
  if (server.hasArg("batType")) batType = server.arg("batType").toInt();
  if (server.hasArg("batCells")) batCells = server.arg("batCells").toInt();
  if (server.hasArg("ssid_STA")) server.arg("ssid_STA").toCharArray(ssid_STA, MAX_SSID_PW_LENGHT + 1);
  if (server.hasArg("password_STA")) server.arg("password_STA").toCharArray(password_STA, MAX_SSID_PW_LENGHT + 1);
  if (server.hasArg("ssid_AP")) server.arg("ssid_AP").toCharArray(ssid_AP, MAX_SSID_PW_LENGHT + 1);
  if (server.hasArg("password_AP")) server.arg("password_AP").toCharArray(password_AP, MAX_SSID_PW_LENGHT + 1);
  if (server.hasArg("mechanicsType")) model.mechanicsType = server.arg("mechanicsType").toInt();
  if (server.hasArg("enableUpdate")) enableUpdate = server.arg("enableUpdate").toInt();
  if (server.hasArg("enableOTA")) enableOTA = server.arg("enableOTA").toInt();

  EEPROM.put(P_NUMBER_LOADCELLS, nLoadcells);
  for (int i = LC1; i <= LC3; i++) {
    EEPROM.put(P_DISTANCE_X1 + (i * sizeof(float)), model.distance[i]);
    saveCalFactor(i);
  }
  EEPROM.put(P_REF_WEIGHT, refWeight);
  EEPROM.put(P_REF_CG, refCG);
  for (int i = R1; i <= R2; i++) {
    EEPROM.put(P_RESISTOR_R1 + (i * sizeof(float)), resistor[i]);
  }
  EEPROM.put(P_BAT_TYPE, batType);
  EEPROM.put(P_BATT_CELLS, batCells);
  EEPROM.put(P_SSID_STA, ssid_STA);
  EEPROM.put(P_PASSWORD_STA, password_STA);
  EEPROM.put(P_SSID_AP, ssid_AP);
  EEPROM.put(P_PASSWORD_AP, password_AP);
  EEPROM.put(P_ENABLE_UPDATE, enableUpdate);
  EEPROM.put(P_ENABLE_OTA, enableOTA);
  EEPROM.commit();

  if (model.name != "") {
    saveModelJson(model.name);
  }

  server.send(200, "text/plain", "saved");
}


// calibrate cg scale
void autoCalibrate() {
  while (!runAutoCalibrate());
  server.send(200, "text/plain", "Calibration successful");
}


// tare cg scale
void runTare() {
  tareLoadcells();
  if (!getLoadcellError()) {
    server.send(200, "text/plain", "Tare completed");
    return;
  }
  server.send(404, "text/plain", "404: Tare failed !");
}


// save model
void saveModel() {
  if (server.hasArg("modelname")) {
    if (server.hasArg("targetCGmin")) model.targetCGmin = server.arg("targetCGmin").toFloat();
    if (server.hasArg("targetCGmax")) model.targetCGmax = server.arg("targetCGmax").toFloat();
    if (server.hasArg("distanceX1")) model.distance[X1] = server.arg("distanceX1").toFloat();
    if (server.hasArg("distanceX2")) model.distance[X2] = server.arg("distanceX2").toFloat();
    if (server.hasArg("distanceX3")) model.distance[X3] = server.arg("distanceX3").toFloat();
    if (server.hasArg("mechanicsType")) model.mechanicsType = server.arg("mechanicsType").toInt();
    if(server.hasArg("virtualWeight")){    
      //StaticJsonDocument<JSONDOC_SIZE> jsonDoc;
      DynamicJsonDocument jsonDoc(JSONDOC_SIZE);
      String json = server.arg("virtualWeight");
      json.replace("%22", "\"");
      deserializeJson(jsonDoc, json);
      JsonArray virtw = jsonDoc["virtual"];
      if(virtw){
        for (int i=0; i < MAX_VIRTUAL_WEIGHT; i++){
          model.virtualWeight[i].name = virtw[i][0].as<String>();
          model.virtualWeight[i].weight = virtw[i][1].as<int>();
          model.virtualWeight[i].cg = virtw[i][2].as<int>();
          model.virtualWeight[i].enabled = virtw[i][3].as<bool>();
        }
      }
    }
      
    if (saveModelJson(server.arg("modelname"))) {
      server.send(200, "text/plain", "saved");
      return;
    }
  }
  server.send(404, "text/plain", "404: Save model failed !");
}


// open model
void openModel() {
  if (server.hasArg("modelname")) {
    if (openModelJson(server.arg("modelname"))) {
      server.send(200, "text/plain", "opened");
      return;
    }
  }
  server.send(404, "text/plain", "404: Open model failed !");
}


// delete model
void deleteModel() {
  if (server.hasArg("modelname")) {
    if (deleteModelJson(server.arg("modelname"))) {
      server.send(200, "text/plain", "deleted");
      return;
    }
  }
  server.send(404, "text/plain", "404: Delete model failed !");
}


// convert the file extension to the MIME type
String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".png")) return "text/css";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".map")) return "application/json";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}


// send file to the client (if it exists)
bool handleFileRead(String path) {
  // If a folder is requested, send the index file
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";

  // If the file exists, either as a compressed archive, or normal
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }

  return false;
}


// upload a new file to the SPIFFS
void handleFileUpload() {

  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    if (filename != MODEL_FILE ) server.send(500, "text/plain", "wrong file !");
    // Open the file for writing in SPIFFS (create if it doesn't exist)
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Write the received bytes to the file
    fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    // If the file was successfully created
    if (fsUploadFile) {
      fsUploadFile.close();
      // Redirect the client to the success page
      server.sendHeader("Location", "/settings.html");
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }

}


// print update progress screen
void printUpdateProgress(unsigned int progress, unsigned int total) {
  printConsole(T_UPDATE, updateMsg);

  oledDisplay.firstPage();
  do {
    oledDisplay.setFont(oledFontSmall);
    oledDisplay.setCursor(0, 12);
    oledDisplay.print(updateMsg);

    oledDisplay.setCursor(107, 35);
    oledDisplay.printf("%u%%\r", (progress / (total / 100)));

    oledDisplay.drawFrame(0, 40, 128, 10);
    oledDisplay.drawBox(0, 40, (progress / (total / 128)), 10);

  } while ( oledDisplay.nextPage() );
}


// https update
bool httpsUpdate(uint8_t command) {
  if (!httpsClient.connect(HOST, HTTPS_PORT)) {
    printConsole(T_ERROR, "Wifi: connection to GIT failed");
    return false;
  }

  const char * headerKeys[] = {"Location"} ;
  const size_t numberOfHeaders = 1;

  HTTPClient https;
  https.setUserAgent("cgscale");
  https.setRedirectLimit(0);
  https.setFollowRedirects(true);

  String url = "https://" + String(HOST) + String(URL);
  if (https.begin(httpsClient, url)) {
    https.collectHeaders(headerKeys, numberOfHeaders);

    printConsole(T_HTTPS, "GET: " + url);
    int httpCode = https.GET();
    if (httpCode > 0) {
      // response
      if (httpCode == HTTP_CODE_FOUND) {
        String newUrl = https.header("Location");
        gitVersion = newUrl.substring(newUrl.lastIndexOf('/') + 2).toFloat();
        if (gitVersion > String(CGSCALE_VERSION).toFloat()) {
          printConsole(T_UPDATE, "Firmware update available: V" + String(gitVersion));
        } else {
          printConsole(T_UPDATE, "Firmware version found on GitHub: V" + String(gitVersion) + " - current firmware is up to date");
        }
      } else if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        //Serial.println(https.getString());
      } else {
        printConsole(T_ERROR, "HTTPS: GET... failed, " + https.errorToString(httpCode));
        https.end();
        return false;
      }
    } else {
      return false;
    }
    https.end();
  } else {
    printConsole(T_ERROR, "Wifi: Unable to connect");
    return false;
  }

  return true;
}

#endif


void setup() {

  // init serial
  Serial.begin(115200);
  Serial.println();
  delay(1000);

#if defined(ESP8266)
  printConsole(T_BOOT, "startup CG scale V" + String(CGSCALE_VERSION));

  // init filesystem
  SPIFFS.begin();
  EEPROM.begin(EEPROM_SIZE);
  printConsole(T_BOOT, "init filesystem");
#endif

  // read settings from eeprom
  if (EEPROM.read(P_NUMBER_LOADCELLS) != 0xFF) {
    nLoadcells = EEPROM.read(P_NUMBER_LOADCELLS);
  }

  for (int i = LC1; i <= LC3; i++) {
    if (EEPROM.read(P_DISTANCE_X1 + (i * sizeof(float))) != 0xFF) {
      EEPROM.get(P_DISTANCE_X1 + (i * sizeof(float)), model.distance[i]);
    }

    if (EEPROM.read(P_LOADCELL1_CALIBRATION_FACTOR + (i * sizeof(float))) != 0xFF) {
      EEPROM.get(P_LOADCELL1_CALIBRATION_FACTOR + (i * sizeof(float)), calFactorLoadcell[i]);
    }
  }

  if (EEPROM.read(P_BAT_TYPE) != 0xFF) {
    batType = EEPROM.read(P_BAT_TYPE);
  }

  if (EEPROM.read(P_BATT_CELLS) != 0xFF) {
    batCells = EEPROM.read(P_BATT_CELLS);
  }

  if (EEPROM.read(P_REF_WEIGHT) != 0xFF) {
    EEPROM.get(P_REF_WEIGHT, refWeight);
  }

  if (EEPROM.read(P_REF_CG) != 0xFF) {
    EEPROM.get(P_REF_CG, refCG);
  }

  for (int i = R1; i <= R2; i++) {
    if (EEPROM.read(P_RESISTOR_R1 + (i * sizeof(float))) != 0xFF) {
      EEPROM.get(P_RESISTOR_R1 + (i * sizeof(float)), resistor[i]);
    }
  }

#if defined(ESP8266)
  if (EEPROM.read(P_SSID_STA) != 0xFF) {
    EEPROM.get(P_SSID_STA, ssid_STA);
  }

  if (EEPROM.read(P_PASSWORD_STA) != 0xFF) {
    EEPROM.get(P_PASSWORD_STA, password_STA);
  }

  if (EEPROM.read(P_SSID_AP) != 0xFF) {
    EEPROM.get(P_SSID_AP, ssid_AP);
  }

  if (EEPROM.read(P_PASSWORD_AP) != 0xFF) {
    EEPROM.get(P_PASSWORD_AP, password_AP);
  }

  if (EEPROM.read(P_MODELNAME) != 0xFF) {
    EEPROM.get(P_MODELNAME, model.name);
  }

  if (EEPROM.read(P_ENABLE_UPDATE) != 0xFF) {
    EEPROM.get(P_ENABLE_UPDATE, enableUpdate);
  }

  if (EEPROM.read(P_ENABLE_OTA) != 0xFF) {
    EEPROM.get(P_ENABLE_OTA, enableOTA);
  }

  // load current model
  printConsole(T_BOOT, "open last model");
  if (!openModelJson(model.name)) {
    saveModelJson(DEFAULT_NAME);
    openModelJson(DEFAULT_NAME);
  }

#endif

  // init OLED display
  initOLED();

  // init & tare Loadcells
  for (int i = LC1; i <= LC3; i++) {
    if (i < nLoadcells) {
      LoadCell[i].begin();
      LoadCell[i].setCalFactor(calFactorLoadcell[i]);
#if defined(ESP8266)
      printConsole(T_BOOT, "init Loadcell " + String(i + 1));
#endif
    }
  }

  // stabilize scale values
  while (millis() < STABILISINGTIME) {
    updateLoadcells();
  }

  tareLoadcells();

  getLoadcellError();

#if defined(ESP8266)

  printConsole(T_BOOT, "Wifi: STA mode - connecing with: " + String(ssid_STA));

  // Start by connecting to a WiFi network
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_STA, password_STA);


  long timeoutWiFi = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (WiFi.status() == WL_NO_SSID_AVAIL) {
      printConsole(T_ERROR, "\nWifi: No SSID available");
      break;
    } else if (WiFi.status() == WL_CONNECT_FAILED) {
      printConsole(T_ERROR, "\nWifi: Connection failed");
      break;
    } else if ((millis() - timeoutWiFi) > TIMEOUT_CONNECT) {
      printConsole(T_ERROR, "\nWifi: Timeout");
      break;
    }
  }


  if (WiFi.status() != WL_CONNECTED) {
    // if WiFi not connected, switch to access point mode
    wifiSTAmode = false;
    printConsole(T_BOOT, "Wifi: AP mode - create access point: " + String(ssid_AP));
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid_AP, password_AP);
    printConsole(T_RUN, "Wifi: Connected, IP: " + String(WiFi.softAPIP().toString()));
  } else {
    printConsole(T_RUN, "Wifi: Connected, IP: " + String(WiFi.localIP().toString()));
  }


  // Set Hostname
  String hostname = "disabled";
#if ENABLE_MDNS
  hostname = ssid_AP;
  hostname.replace(" ", "");
  hostname.toLowerCase();
  if (!MDNS.begin(hostname, WiFi.localIP())) {
    hostname = "mDNS failed";
    printConsole(T_ERROR, "Wifi: " + hostname);
  } else {
    hostname += ".local";
    printConsole(T_RUN, "Wifi hostname: " + hostname);
  }
#endif

  if (wifiSTAmode) {
    printOLED("WiFi: " + String(ssid_STA),
              "Host: " + String(hostname),
              "IP: " + WiFi.localIP().toString());
  } else {
    printOLED("WiFi: " + String(ssid_AP),
              "Host: " + String(hostname),
              "IP: " + WiFi.softAPIP().toString());
  }

  delay(3000);

  // When the client requests data
  server.on("/getHead", getHead);
  server.on("/getValue", getValue);
  server.on("/getRawValue", getRawValue);
  server.on("/getParameter", getParameter);
  server.on("/getWiFiNetworks", getWiFiNetworks);
  server.on("/getVirtualWeight", getVirtualWeight);
  server.on("/saveParameter", saveParameter);
  server.on("/autoCalibrate", autoCalibrate);
  server.on("/tare", runTare);
  server.on("/saveModel", saveModel);
  server.on("/openModel", openModel);
  server.on("/deleteModel", deleteModel);

  // When the client upload file
  server.on("/settings.html", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  // If the client requests any URI
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "CGscale Error: 404\n File or URL not Found !");
  });

  // init ElegantOTA
  ElegantOTA.begin(&server);

  // init webserver
  server.begin();
  printConsole(T_RUN, "Webserver is up and running");

  // init OTA (over the air update)
  if (enableOTA) {
    ArduinoOTA.setHostname(ssid_AP);
    ArduinoOTA.setPassword(password_AP);

    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "firmware";
      } else { // U_SPIFFS
        type = "SPIFFS";
      }
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      updateMsg = "Updating " + type;
      printConsole(T_UPDATE, type);
    });

    ArduinoOTA.onEnd([]() {
      updateMsg = "successful..";
      printUpdateProgress(100, 100);
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      printUpdateProgress(progress, total);
    });

    ArduinoOTA.onError([](ota_error_t error) {
      if (error == OTA_AUTH_ERROR) {
        updateMsg = "Auth Failed";
      } else if (error == OTA_BEGIN_ERROR) {
        updateMsg = "Begin Failed";
      } else if (error == OTA_CONNECT_ERROR) {
        updateMsg = "Connect Failed";
      } else if (error == OTA_RECEIVE_ERROR) {
        updateMsg = "Receive Failed";
      } else if (error == OTA_END_ERROR) {
        updateMsg = "End Failed";
      }
      printUpdateProgress(0, 100);
    });

    ArduinoOTA.begin();
    printConsole(T_RUN, "OTA is up and running");
  }

  // https update
  httpsClient.setInsecure();
  if (enableUpdate) {
    // check for update
    httpsUpdate(PROBE_UPDATE);
  }

#endif

}


void loop() {

#if defined(ESP8266)

#if ENABLE_MDNS
  MDNS.update();
#endif

  if (enableOTA) {
    ArduinoOTA.handle();
  }
  server.handleClient();
#endif

#ifdef PIN_TARE_BUTTON
  handleTareBtn();
#endif

  updateLoadcells();

  // update loadcell values
  if ((millis() - lastTimeLoadcell) > UPDATE_INTERVAL_LOADCELL) {
    lastTimeLoadcell = millis();

    // get Loadcell weights
    for (int i = LC1; i <= LC3; i++) {
      if (i < nLoadcells) {
        weightLoadCell[i] = LoadCell[i].getData();
        // IIR filter
        weightLoadCell[i] = weightLoadCell[i] + SMOOTHING_LOADCELL * (lastWeightLoadCell[i] - weightLoadCell[i]);
        lastWeightLoadCell[i] = weightLoadCell[i];
      }
    }
  }


  // update display and serial menu
  if ((millis() - lastTimeMenu) > UPDATE_INTERVAL_OLED_MENU) {

    lastTimeMenu = millis();

    // total model weight
    weightTotal = weightLoadCell[LC1] + weightLoadCell[LC2] + weightLoadCell[LC3];
    if (weightTotal < MINIMAL_TOTAL_WEIGHT && weightTotal > MINIMAL_TOTAL_WEIGHT * -1) {
      weightTotal = 0;
    }

    if (weightTotal > MINIMAL_CG_WEIGHT) {
      // CG longitudinal axis
      CG_length = ((weightLoadCell[LC2] * model.distance[X2]) / weightTotal) + model.distance[X1];

#if defined(ESP8266)
      if (model.mechanicsType == 2) {
        CG_length = ((weightLoadCell[LC2] * model.distance[X2]) / weightTotal) - model.distance[X1];
      } else if (model.mechanicsType == 3) {
        CG_length = ((weightLoadCell[LC2] * model.distance[X2]) / weightTotal) * -1 + model.distance[X1];
      }

      /* Virtual weights 

      m = weight
      d = cg
      d_new=(m1*d1+m2*d2)/(m1+m2)
      */

      for (int i=0; i < MAX_VIRTUAL_WEIGHT; i++){
        if(model.virtualWeight[i].enabled == true){
          CG_length = (weightTotal * CG_length + model.virtualWeight[i].weight * model.virtualWeight[i].cg) / (weightTotal + model.virtualWeight[i].weight);
        }
      }

      for (int i=0; i < MAX_VIRTUAL_WEIGHT; i++){
        if(model.virtualWeight[i].enabled == true){
          weightTotal +=  model.virtualWeight[i].weight;
        }
      }

      
      
#endif

      // CG transverse axis
      if (nLoadcells == 3) {
        CG_trans = (model.distance[X3] / 2) - (((weightLoadCell[LC1] + weightLoadCell[LC2] / 2) * model.distance[X3]) / weightTotal);
      }

    } else {
      CG_length = 0;
      CG_trans = 0;
    }

    // read battery voltage
    if (batType > B_OFF) {
      batVolt = (analogRead(VOLTAGE_PIN) / 1024.0) * V_REF * ((resistor[R1] + resistor[R2]) / resistor[R2]) / 1000.0;
#if ENABLE_PERCENTLIST
      if (batType > B_VOLT) {
        batVolt = percentBat(batVolt / batCells);
      }
#endif
    }

    printScaleOLED();

    // serial connection
    if (Serial) {
      if (Serial.available() > 0) {

        switch (menuPage)
        {
          case MENU_HOME:
            menuPage = Serial.parseInt();
            updateMenu = true;
            break;
          case MENU_LOADCELLS:
            nLoadcells = Serial.parseInt();
            EEPROM.put(P_NUMBER_LOADCELLS, nLoadcells);
#if defined(ESP8266)
            EEPROM.commit();
#endif
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_DISTANCE_X1 ... MENU_DISTANCE_X3:
            model.distance[menuPage - MENU_DISTANCE_X1] = Serial.parseFloat();
            EEPROM.put(P_DISTANCE_X1 + ((menuPage - MENU_DISTANCE_X1) * sizeof(float)), model.distance[menuPage - MENU_DISTANCE_X1]);
#if defined(ESP8266)
            EEPROM.commit();
#endif
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_REF_WEIGHT:
            refWeight = Serial.parseFloat();
            EEPROM.put(P_REF_WEIGHT, refWeight);
#if defined(ESP8266)
            EEPROM.commit();
#endif
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_REF_CG:
            refCG = Serial.parseFloat();
            EEPROM.put(P_REF_CG, refCG);
#if defined(ESP8266)
            EEPROM.commit();
#endif
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_AUTO_CALIBRATE:
            if (Serial.read() == 'J') {
              runAutoCalibrate();
            }
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_LOADCELL1_CALIBRATION_FACTOR ... MENU_LOADCELL3_CALIBRATION_FACTOR:
            calFactorLoadcell[menuPage - MENU_LOADCELL1_CALIBRATION_FACTOR] = Serial.parseFloat();
            saveCalFactor(menuPage - MENU_LOADCELL1_CALIBRATION_FACTOR);
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_RESISTOR_R1 ... MENU_RESISTOR_R2:
            resistor[menuPage - MENU_RESISTOR_R1] = Serial.parseFloat();
            EEPROM.put(P_RESISTOR_R1 + ((menuPage - MENU_RESISTOR_R1) * sizeof(float)), resistor[menuPage - MENU_RESISTOR_R1]);
#if defined(ESP8266)
            EEPROM.commit();
#endif
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_BATTERY_MEASUREMENT:
            batType = Serial.parseInt();
            EEPROM.put(P_BAT_TYPE, batType);
#if defined(ESP8266)
            EEPROM.commit();
#endif
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_BATTERY_CELLS:
            batCells = Serial.parseInt();
            EEPROM.put(P_BATT_CELLS, batCells);
#if defined(ESP8266)
            EEPROM.commit();
#endif
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_RESET_DEFAULT:
            if (Serial.read() == 'J') {
              // reset eeprom
              for (int i = 0; i < EEPROM_SIZE; i++) {
                EEPROM.write(i, 0xFF);
              }
              Serial.end();
#if defined(ESP8266)
              EEPROM.commit();
              // delete json model file
              if (SPIFFS.exists(MODEL_FILE)) {
                SPIFFS.remove(MODEL_FILE);
              }
#endif
              resetCPU();
            }
            menuPage = 0;
            updateMenu = true;
            break;
          default:
            Serial.readString();
            menuPage = 0;
            updateMenu = true;
            break;
        }
        Serial.readString();

      }

      if (!updateMenu)
        return;

      switch (menuPage)
      {
        case MENU_HOME: {
            Serial.print(F("\n\n********************************************\nCG scale by M.Lehmann - V"));
            Serial.print(CGSCALE_VERSION);
            Serial.print(F("\n\n"));

            Serial.print(MENU_LOADCELLS);
            Serial.print(F("  - Set number of load cells ("));
            Serial.print(nLoadcells);
            Serial.print(F(")\n"));

            for (int i = X1; i <= X3; i++) {
              Serial.print(MENU_DISTANCE_X1 + i);
              Serial.print(F("  - Set distance X"));
              Serial.print(i + 1);
              Serial.print(F(" ("));
              Serial.print(model.distance[i]);
              Serial.print(F("mm)\n"));
            }

            Serial.print(MENU_REF_WEIGHT);
            Serial.print(F("  - Set reference weight ("));
            Serial.print(refWeight);
            Serial.print(F("g)\n"));

            Serial.print(MENU_REF_CG);
            Serial.print(F("  - Set reference CG ("));
            Serial.print(refCG);
            Serial.print(F("mm)\n"));

            Serial.print(MENU_AUTO_CALIBRATE);
            Serial.print(F("  - Start autocalibration\n"));

            for (int i = LC1; i <= LC3; i++) {
              Serial.print(MENU_LOADCELL1_CALIBRATION_FACTOR + i);
              if ((MENU_LOADCELL1_CALIBRATION_FACTOR + i) < 10) Serial.print(F(" "));
              Serial.print(F(" - Set calibration factor of load cell "));
              Serial.print(i + 1);
              Serial.print(F(" ("));
              Serial.print(calFactorLoadcell[i]);
              Serial.print(F(")\n"));
            }

            for (int i = R1; i <= R2; i++) {
              Serial.print(MENU_RESISTOR_R1 + i);
              Serial.print(F(" - Set value of resistor R"));
              Serial.print(i + 1);
              Serial.print(F(" ("));
              Serial.print(resistor[i]);
              Serial.print(F("ohm)\n"));
            }

            Serial.print(MENU_BATTERY_MEASUREMENT);
            Serial.print(F(" - Set battery type ("));
            Serial.print(battTypName[batType]);
            Serial.print(F(")\n"));

            Serial.print(MENU_BATTERY_CELLS);
            Serial.print(F(" - Set number of battery cells ("));
            Serial.print(batCells);
            Serial.print(F(")\n"));

            Serial.print(MENU_SHOW_ACTUAL);
            Serial.print(F(" - Show actual values\n"));

#if defined(ESP8266)
            Serial.print(MENU_WIFI_INFO);
            Serial.print(F(" - Show WiFi network info\n"));
#endif

            Serial.print(MENU_RESET_DEFAULT);
            Serial.print(F(" - Reset to factory defaults\n"));

            Serial.print(F("\n"));
            for (int i = 1; i <= errMsgCnt; i++) {
              Serial.print(errMsg[i]);
            }

            Serial.print(F("\nPlease choose the menu number:"));

            updateMenu = false;
            break;
          }
        case MENU_LOADCELLS:
          Serial.print(F("\n\nNumber of load cells: "));
          Serial.println(nLoadcells);
          printNewValueText();
          updateMenu = false;
          break;
        case MENU_DISTANCE_X1 ... MENU_DISTANCE_X3:
          Serial.print("\n\nDistance X");
          Serial.print(menuPage - MENU_DISTANCE_X1 + 1);
          Serial.print(F(": "));
          Serial.print(model.distance[menuPage - MENU_DISTANCE_X1]);
          Serial.print(F("mm\n"));
          printNewValueText();
          updateMenu = false;
          break;
        case MENU_REF_WEIGHT:
          Serial.print(F("\n\nReference weight: "));
          Serial.print(refWeight);
          Serial.print(F("g\n"));
          printNewValueText();
          updateMenu = false;
          break;
        case MENU_REF_CG:
          Serial.print(F("\n\nReference CG: "));
          Serial.print(refCG);
          Serial.print(F("mm\n"));
          printNewValueText();
          updateMenu = false;
          break;
        case MENU_AUTO_CALIBRATE:
          Serial.print(F("\n\nPlease put the reference weight on the scale.\nStart auto calibration (J/N)?\n"));
          updateMenu = false;
          break;
        case MENU_LOADCELL1_CALIBRATION_FACTOR ... MENU_LOADCELL3_CALIBRATION_FACTOR:
          Serial.print("\n\nCalibration factor of load cell ");
          Serial.print(menuPage - MENU_LOADCELL1_CALIBRATION_FACTOR + 1);
          Serial.print(F(": "));
          Serial.println(calFactorLoadcell[menuPage - MENU_LOADCELL1_CALIBRATION_FACTOR]);
          printNewValueText();
          updateMenu = false;
          break;
        case MENU_RESISTOR_R1 ... MENU_RESISTOR_R2:
          Serial.print(F("\n\nValue of resistor R"));
          Serial.print(menuPage - MENU_RESISTOR_R1 + 1);
          Serial.print(F(": "));
          Serial.println(resistor[menuPage - MENU_RESISTOR_R1]);
          printNewValueText();
          updateMenu = false;
          break;
        case MENU_BATTERY_MEASUREMENT: {
            Serial.print(F("\n\nBattery type: "));
            Serial.println(battTypName[batType]);
            for (int i = 0; i < NUMBER_BAT_TYPES; i++) {
              Serial.print(i);
              Serial.print(" = ");
              Serial.println(battTypName[i]);
            }
            printNewValueText();
            updateMenu = false;
            break;
          }
        case MENU_BATTERY_CELLS:
          Serial.print(F("\n\nBattery cells: "));
          Serial.println(batCells);
          printNewValueText();
          updateMenu = false;
          break;
        case MENU_SHOW_ACTUAL:
          for (int i = LC1; i <= LC3; i++) {
            if (i < nLoadcells) {
              Serial.print(F("Lc"));
              Serial.print(i + 1);
              Serial.print(F(": "));
              Serial.print(weightLoadCell[i]);
              Serial.print(F("g  "));
            }
          }
          Serial.print(F("Total weight: "));
          Serial.print(weightTotal);
          Serial.print(F("g  CG length: "));
          Serial.print(CG_length);
          if (nLoadcells == 3) {
            Serial.print(F("mm  CG trans: "));
            Serial.print(CG_trans);
            Serial.print(F("mm"));
          }
          if (batType > B_OFF) {
            Serial.print(F("  Battery:"));
            Serial.print(batVolt);
            if (batType == B_VOLT) {
              Serial.print(F("V"));
            } else {
              Serial.print(F("%"));
            }
          }
          Serial.println();
          break;
#if defined(ESP8266)
        case MENU_WIFI_INFO:
          {
            Serial.println("\n\n********************************************\nWiFi network information\n");

            Serial.println("# Current WiFi status:");
            WiFi.printDiag(Serial);
            if (wifiSTAmode == false) {
              Serial.print("Connected clients: ");
              Serial.println(WiFi.softAPgetStationNum());
            }

            Serial.println("\n# Available WiFi networks:");
            int wifiCnt = WiFi.scanNetworks();
            if (wifiCnt == 0) {
              Serial.println("no networks found");
            } else {
              for (int i = 0; i < wifiCnt; ++i) {
                // Print SSID and RSSI for each network found
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.print("dBm) ");
                switch (WiFi.encryptionType(i)) {
                  case ENC_TYPE_WEP:
                    Serial.print("WEP");
                    break;
                  case ENC_TYPE_TKIP:
                    Serial.print("WPA");
                    break;
                  case ENC_TYPE_CCMP:
                    Serial.print("WPA2");
                    break;
                  case ENC_TYPE_AUTO:
                    Serial.print("Auto");
                    break;
                }
                Serial.println("");
              }
            }
          }
          updateMenu = false;
          break;
#endif
        case MENU_RESET_DEFAULT:
          Serial.print(F("\n\nReset to factory defaults (J/N)?\n"));
          updateMenu = false;
          break;
      }

    } else {
      updateMenu = true;
    }
  }
}
