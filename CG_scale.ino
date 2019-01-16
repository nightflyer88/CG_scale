/*
  ------------------------------------------------------------------
                            CG scale
                      (c) 2019 by M. Lehmann
  ------------------------------------------------------------------
*/
#define CGSCALE_VERSION "1.0.51"
/*

  ******************************************************************
  history:
  V1.1    beta          ESP8266
  V1.0    12.01.19      first release


  ******************************************************************

  Software License Agreement (BSD License)

  Copyright (c) 2019, Michael Lehmann
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
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "settings_ESP8266.h"
#endif

// settings for AVR
#if defined(__AVR__)
#include "settings_AVR.h"
#endif

// HX711 constructor (dout pin, sck pint):
HX711_ADC LoadCell_1(PIN_LOADCELL1_DOUT, PIN_LOADCELL1_PD_SCK);
HX711_ADC LoadCell_2(PIN_LOADCELL2_DOUT, PIN_LOADCELL2_PD_SCK);
HX711_ADC LoadCell_3(PIN_LOADCELL3_DOUT, PIN_LOADCELL3_PD_SCK);

// webserver constructor
#if defined(ESP8266)
ESP8266WebServer server(80);
IPAddress apIP(ip[0], ip[1], ip[2], ip[3]);
#endif

// serial menu
enum
{
  MENU_HOME,
  MENU_LOADCELLS,
  MENU_DISTANCE_X1,
  MENU_DISTANCE_X2,
  MENU_DISTANCE_X3,
  MENU_REF_WEIGHT,
  MENU_REF_CG,
  MENU_AUTO_CALIBRATE,
  MENU_LOADCELL1_CALIBRATION_FACTOR,
  MENU_LOADCELL2_CALIBRATION_FACTOR,
  MENU_LOADCELL3_CALIBRATION_FACTOR,
  MENU_RESISTOR_R1,
  MENU_RESISTOR_R2,
  MENU_BATTERY_MEASUREMENT,
  MENU_SHOW_ACTUAL,
  MENU_RESET_DEFAULT
};

// EEprom parameter addresses
#define EEPROM_SIZE 120
enum
{
  P_NUMBER_LOADCELLS =                  1,
  P_DISTANCE_X1 =                       2,
  P_DISTANCE_X2 =                       P_DISTANCE_X1 + sizeof(float),
  P_DISTANCE_X3 =                       P_DISTANCE_X2 + sizeof(float),
  P_LOADCELL1_CALIBRATION_FACTOR =      P_DISTANCE_X3 + sizeof(float),
  P_LOADCELL2_CALIBRATION_FACTOR =      P_LOADCELL1_CALIBRATION_FACTOR + sizeof(float),
  P_LOADCELL3_CALIBRATION_FACTOR =      P_LOADCELL2_CALIBRATION_FACTOR + sizeof(float),
  P_ENABLE_BATVOLT =                    P_LOADCELL3_CALIBRATION_FACTOR + sizeof(float),
  P_REF_WEIGHT =                        P_ENABLE_BATVOLT + sizeof(float),
  P_REF_CG =                            P_REF_WEIGHT + sizeof(float),
  P_RESISTOR_R1 =                       P_REF_CG + sizeof(float),
  P_RESISTOR_R2 =                       P_RESISTOR_R1 + sizeof(float)
};

// battery image 12x6
static const unsigned char batteryImage[] U8X8_PROGMEM = {
  0xfc, 0xff, 0x07, 0xf8, 0x01, 0xf8, 0x01, 0xf8, 0x07, 0xf8, 0xfc, 0xff
};

// weight image 18x18
static const unsigned char weightImage[] U8X8_PROGMEM = {
  0x00, 0x00, 0xfc, 0x00, 0x03, 0xfc, 0x80, 0x04, 0xfc, 0x80, 0x04, 0xfc,
  0x80, 0x07, 0xfc, 0xf8, 0x7f, 0xfc, 0x08, 0x40, 0xfc, 0x08, 0x40, 0xfc,
  0x08, 0x47, 0xfc, 0x84, 0x84, 0xfc, 0x84, 0x84, 0xfc, 0x04, 0x87, 0xfc,
  0x04, 0x84, 0xfc, 0x02, 0x03, 0xfd, 0x02, 0x00, 0xfd, 0x02, 0x00, 0xfd,
  0xfe, 0xff, 0xfd, 0x00, 0x00, 0xfc
};

// CG image 18x18
static const unsigned char CGImage[] U8X8_PROGMEM = {
  0x00, 0x02, 0xfc, 0xc0, 0x1f, 0xfc, 0x30, 0x7e, 0xfc, 0x08, 0xfe, 0xfc,
  0x04, 0xfe, 0xfc, 0x04, 0xfe, 0xfd, 0x02, 0xfe, 0xfd, 0x02, 0xfe, 0xfd,
  0x02, 0xfe, 0xff, 0xff, 0x01, 0xfd, 0xfe, 0x01, 0xfd, 0xfe, 0x01, 0xfd,
  0xfe, 0x81, 0xfc, 0xfc, 0x81, 0xfc, 0xfc, 0x41, 0xfc, 0xf8, 0x31, 0xfc,
  0xe0, 0x0f, 0xfc, 0x00, 0x01, 0xfc
};

// CG transverse axis image 18x18
static const unsigned char CGtransImage[] U8X8_PROGMEM = {
  0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x04, 0x70, 0xfc,
  0x04, 0x90, 0xfc, 0x04, 0x90, 0xfc, 0x04, 0x70, 0xfc, 0x04, 0x50, 0xfc,
  0x04, 0x90, 0xfc, 0x3c, 0x90, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc,
  0x08, 0x40, 0xfc, 0x04, 0x80, 0xfc, 0x7e, 0xf8, 0xfd, 0x04, 0x80, 0xfc,
  0x08, 0x40, 0xfc, 0x00, 0x00, 0xfc
};

// set default text
static const String newValueText = "Set new value:";

// load default values
uint8_t nLoadcells = NUMBER_LOADCELLS;
float distanceX1 = DISTANCE_X1;
float distanceX2 = DISTANCE_X2;
float distanceX3 = DISTANCE_X3;
float calFactorLoadcell1 = LOADCELL1_CALIBRATION_FACTOR;
float calFactorLoadcell2 = LOADCELL2_CALIBRATION_FACTOR;
float calFactorLoadcell3 = LOADCELL3_CALIBRATION_FACTOR;
float resistorR1 = RESISTOR_R1;
float resistorR2 = RESISTOR_R2;
bool enableBatVolt = ENABLE_VOLTAGE;
float refWeight = REF_WEIGHT;
float refCG = REF_CG;

// declare variables
float weightLoadCell1 = 0;
float weightLoadCell2 = 0;
float weightLoadCell3 = 0;
float lastWeightLoadCell1 = 0;
float lastWeightLoadCell2 = 0;
float lastWeightLoadCell3 = 0;
float weightTotal = 0;
float CG_length = 0;
float CG_trans = 0;
float batVolt = 0;
unsigned long lastTimeMenu = 0;
unsigned long lastTimeLoadcell = 0;
bool updateMenu = true;
int menuPage = 0;


// Restart CPU
void(* resetCPU) (void) = 0;


// save values to eeprom
void saveLoadcells() {
  EEPROM.put(P_NUMBER_LOADCELLS, nLoadcells);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveDistanceX1() {
  EEPROM.put(P_DISTANCE_X1, distanceX1);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveDistanceX2() {
  EEPROM.put(P_DISTANCE_X2, distanceX2);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveDistanceX3() {
  EEPROM.put(P_DISTANCE_X3, distanceX3);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveRefWeight() {
  EEPROM.put(P_REF_WEIGHT, refWeight);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveRefCG() {
  EEPROM.put(P_REF_CG, refCG);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveCalFactor1() {
  LoadCell_1.setCalFactor(calFactorLoadcell1);
  EEPROM.put(P_LOADCELL1_CALIBRATION_FACTOR, calFactorLoadcell1);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveCalFactor2() {
  LoadCell_2.setCalFactor(calFactorLoadcell2);
  EEPROM.put(P_LOADCELL2_CALIBRATION_FACTOR, calFactorLoadcell2);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveCalFactor3() {
  LoadCell_3.setCalFactor(calFactorLoadcell3);
  EEPROM.put(P_LOADCELL3_CALIBRATION_FACTOR, calFactorLoadcell3);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveResistorR1() {
  EEPROM.put(P_RESISTOR_R1, resistorR1);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}


void saveResistorR2() {
  EEPROM.put(P_RESISTOR_R2, resistorR2);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}

void saveEnableBatVolt() {
  EEPROM.put(P_ENABLE_BATVOLT, enableBatVolt);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}

void auto_calibrate() {
  Serial.print(F("Autocalibration is running"));
  for (int i = 0; i <= 20; i++) {
    Serial.print(F("."));
    delay(100);
  }
  // calculate weight
  float toWeightLoadCell2 = ((refCG - distanceX1) * refWeight) / distanceX2;
  float toWeightLoadCell1 = refWeight - toWeightLoadCell2;
  float toWeightLoadCell3 = 0;
  if (nLoadcells > 2) {
    toWeightLoadCell1 = toWeightLoadCell1 / 2;
    toWeightLoadCell3 = toWeightLoadCell1;
  }
  // calculate calibration factors
  calFactorLoadcell1 = calFactorLoadcell1 / (toWeightLoadCell1 / weightLoadCell1);
  calFactorLoadcell2 = calFactorLoadcell2 / (toWeightLoadCell2 / weightLoadCell2);
  if (nLoadcells > 2) {
    calFactorLoadcell3 = calFactorLoadcell3 / (toWeightLoadCell3 / weightLoadCell3);
  }
  saveCalFactor1();
  saveCalFactor2();
  saveCalFactor3();
  // finish
  Serial.println(F("done"));
}


void setup() {

#if defined(ESP8266)
  // init webserver
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);

  server.begin();
  server.on("/", main_page);
  server.on("/index.html", main_page);
  server.on("/settings", settings_page);
  server.on("/settings.png", settingsImg);
  server.on("/weight.png", weightImg);
  server.on("/cg.png", cgImg);
  server.on("/cglr.png", cgLRimg);
  server.on("/battery.png", batteryImg);
  server.on("/CG_scale_mechanics.png", mechanicsImg);
  server.on("/bootstrap.min.css", bootstrap);
  server.on("bootstrap.min.css", bootstrap);
  server.on("/popper.min.js", popper);
  server.on("/bootstrap.min.js", bootstrapmin);
  server.on("bootstrap.min.js", bootstrapmin);

  SPIFFS.begin();
  EEPROM.begin(EEPROM_SIZE);
#endif

  // init OLED display
  oledDisplay.begin();
  oledDisplay.firstPage();
  do {
    oledDisplay.drawXBMP(20, 12, 18, 18, CGImage);
    oledDisplay.setFont(u8g2_font_helvR12_tr);
    oledDisplay.setCursor(45, 28);
    oledDisplay.print(F("CG scale"));

    oledDisplay.setFont(u8g2_font_5x7_tr);
    oledDisplay.setCursor(35, 55);
    oledDisplay.print(F("Version: "));
    oledDisplay.print(CGSCALE_VERSION);
    oledDisplay.setCursor(20, 64);
    oledDisplay.print(F("(c) 2019 M. Lehmann"));
  } while ( oledDisplay.nextPage() );

  // read settings from eeprom
  if (EEPROM.read(P_NUMBER_LOADCELLS) != 0xFF) {
    nLoadcells = EEPROM.read(P_NUMBER_LOADCELLS);
  }

  if (EEPROM.read(P_DISTANCE_X1) != 0xFF) {
    EEPROM.get(P_DISTANCE_X1, distanceX1);
  }

  if (EEPROM.read(P_DISTANCE_X2) != 0xFF) {
    EEPROM.get(P_DISTANCE_X2, distanceX2);
  }

  if (EEPROM.read(P_DISTANCE_X3) != 0xFF) {
    EEPROM.get(P_DISTANCE_X3, distanceX3);
  }

  if (EEPROM.read(P_LOADCELL1_CALIBRATION_FACTOR) != 0xFF) {
    EEPROM.get(P_LOADCELL1_CALIBRATION_FACTOR, calFactorLoadcell1);
  }

  if (EEPROM.read(P_LOADCELL2_CALIBRATION_FACTOR) != 0xFF) {
    EEPROM.get(P_LOADCELL2_CALIBRATION_FACTOR, calFactorLoadcell2);
  }

  if (EEPROM.read(P_LOADCELL3_CALIBRATION_FACTOR) != 0xFF) {
    EEPROM.get(P_LOADCELL3_CALIBRATION_FACTOR, calFactorLoadcell3);
  }

  if (EEPROM.read(P_ENABLE_BATVOLT) != 0xFF) {
    EEPROM.get(P_ENABLE_BATVOLT, enableBatVolt);
  }

  if (EEPROM.read(P_REF_WEIGHT) != 0xFF) {
    EEPROM.get(P_REF_WEIGHT, refWeight);
  }

  if (EEPROM.read(P_REF_CG) != 0xFF) {
    EEPROM.get(P_REF_CG, refCG);
  }

  if (EEPROM.read(P_RESISTOR_R1) != 0xFF) {
    EEPROM.get(P_RESISTOR_R1, resistorR1);
  }

  if (EEPROM.read(P_RESISTOR_R2) != 0xFF) {
    EEPROM.get(P_RESISTOR_R2, resistorR2);
  }

  // init Loadcells
  LoadCell_1.begin();
  LoadCell_2.begin();
  if (nLoadcells > 2) {
    LoadCell_3.begin();
  }

  // tare
  if (nLoadcells > 2) {
    while (!LoadCell_1.startMultiple(STABILISINGTIME) && !LoadCell_2.startMultiple(STABILISINGTIME) && !LoadCell_3.startMultiple(STABILISINGTIME)) {
    }
  } else {
    while (!LoadCell_1.startMultiple(STABILISINGTIME) && !LoadCell_2.startMultiple(STABILISINGTIME)) {
    }
  }

  // set calibration factor
  LoadCell_1.setCalFactor(calFactorLoadcell1);
  LoadCell_2.setCalFactor(calFactorLoadcell2);
  if (nLoadcells > 2) {
    LoadCell_3.setCalFactor(calFactorLoadcell3);
  }

  // stabilize scale values
  for (int i = 0; i <= 5; i++) {
    LoadCell_1.update();
    LoadCell_2.update();
    if (nLoadcells > 2) {
      LoadCell_3.update();
    }
    delay(200);
  }

  // init serial
  Serial.begin(9600);

}


void loop() {

  LoadCell_1.update();
  LoadCell_2.update();
  if (nLoadcells > 2) {
    LoadCell_3.update();
  }


  // update loadcell values
  if ((millis() - lastTimeLoadcell) > UPDATE_INTERVAL_LOADCELL) {
    lastTimeLoadcell = millis();

    // get Loadcell weights
    weightLoadCell1 = LoadCell_1.getData();
    weightLoadCell2 = LoadCell_2.getData();
    if (nLoadcells > 2) {
      weightLoadCell3 = LoadCell_3.getData();
    }

    // IIR filter
    weightLoadCell1 = weightLoadCell1 + SMOOTHING_LOADCELL1 * (lastWeightLoadCell1 - weightLoadCell1);
    lastWeightLoadCell1 = weightLoadCell1;

    weightLoadCell2 = weightLoadCell2 + SMOOTHING_LOADCELL2 * (lastWeightLoadCell2 - weightLoadCell2);
    lastWeightLoadCell2 = weightLoadCell2;

    weightLoadCell3 = weightLoadCell3 + SMOOTHING_LOADCELL3 * (lastWeightLoadCell3 - weightLoadCell3);
    lastWeightLoadCell3 = weightLoadCell3;
  }


  // update display and serial menu
  if ((millis() - lastTimeMenu) > UPDATE_INTERVAL_OLED_MENU) {
    lastTimeMenu = millis();

    // total model weight
    weightTotal = weightLoadCell1 + weightLoadCell2 + weightLoadCell3;
    if (weightTotal < MINIMAL_TOTAL_WEIGHT && weightTotal > MINIMAL_TOTAL_WEIGHT * -1) {
      weightTotal = 0;
    }

    if (weightTotal > MINIMAL_CG_WEIGHT) {
      // CG longitudinal axis
      CG_length = ((weightLoadCell2 * distanceX2) / weightTotal) + distanceX1;

      // CG transverse axis
      if (nLoadcells > 2) {
        CG_trans = (distanceX3 / 2) - (((weightLoadCell1 + weightLoadCell2 / 2) * distanceX3) / weightTotal);
      }
    }else{
      CG_length = 0;
      CG_trans = 0;
    }

    // read battery voltage
    if (enableBatVolt) {
      batVolt = (analogRead(VOLTAGE_PIN) / 1024.0) * V_REF * ((resistorR1 + resistorR2) / resistorR2) / 1000.0;
    }

    // print to display
    char buff[8];
    int pos_weightTotal = 7;
    int pos_CG_length = 28;
    if (nLoadcells < 3) {
      pos_weightTotal = 17;
      pos_CG_length = 45;
      if (!enableBatVolt) {
        pos_weightTotal = 12;
        pos_CG_length = 40;
      }
    }


    oledDisplay.firstPage();
    do {
      // print battery
      if (enableBatVolt) {
        oledDisplay.drawXBMP(88, 1, 12, 6, batteryImage);
        dtostrf(batVolt, 2, 2, buff);
        oledDisplay.setFont(u8g2_font_5x7_tr);
        oledDisplay.setCursor(123 - oledDisplay.getStrWidth(buff), 7);
        oledDisplay.print(buff);
        oledDisplay.print(F("V"));
      }

      // print total weight
      oledDisplay.drawXBMP(2, pos_weightTotal, 18, 18, weightImage);
      dtostrf(weightTotal, 5, 1, buff);
      oledDisplay.setFont(u8g2_font_helvR12_tr);
      oledDisplay.setCursor(93 - oledDisplay.getStrWidth(buff), pos_weightTotal + 17);
      oledDisplay.print(buff);
      oledDisplay.print(F(" g"));

      // print CG longitudinal axis
      oledDisplay.drawXBMP(2, pos_CG_length, 18, 18, CGImage);
      dtostrf(CG_length, 5, 1, buff);
      oledDisplay.setCursor(93 - oledDisplay.getStrWidth(buff), pos_CG_length + 16);
      oledDisplay.print(buff);
      oledDisplay.print(F(" mm"));

      // print CG transverse axis
      if (nLoadcells > 2) {
        oledDisplay.drawXBMP(2, 47, 18, 18, CGtransImage);
        dtostrf(CG_trans, 5, 1, buff);
        oledDisplay.setCursor(93 - oledDisplay.getStrWidth(buff), 64);
        oledDisplay.print(buff);
        oledDisplay.print(F(" mm"));
      }

    } while ( oledDisplay.nextPage() );

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
            saveLoadcells();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_DISTANCE_X1:
            distanceX1 = Serial.parseFloat();
            saveDistanceX1();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_DISTANCE_X2:
            distanceX2 = Serial.parseFloat();
            saveDistanceX2();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_DISTANCE_X3:
            distanceX3 = Serial.parseFloat();
            saveDistanceX3();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_REF_WEIGHT:
            refWeight = Serial.parseFloat();
            saveRefWeight();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_REF_CG:
            refCG = Serial.parseFloat();
            saveRefCG();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_AUTO_CALIBRATE:
            if (Serial.read() == 'J') {
              auto_calibrate();
              menuPage = 0;
              updateMenu = true;
            }
            break;
          case MENU_LOADCELL1_CALIBRATION_FACTOR:
            calFactorLoadcell1 = Serial.parseFloat();
            saveCalFactor1();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_LOADCELL2_CALIBRATION_FACTOR:
            calFactorLoadcell2 = Serial.parseFloat();
            saveCalFactor2();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_LOADCELL3_CALIBRATION_FACTOR:
            calFactorLoadcell3 = Serial.parseFloat();
            saveCalFactor3();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_RESISTOR_R1:
            resistorR1 = Serial.parseFloat();
            saveResistorR1();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_RESISTOR_R2:
            resistorR2 = Serial.parseFloat();
            saveResistorR2();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_BATTERY_MEASUREMENT:
            if (Serial.read() == 'J') {
              enableBatVolt = true;
            } else {
              enableBatVolt = false;
            }
            saveEnableBatVolt();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_SHOW_ACTUAL:
            Serial.readString();
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_RESET_DEFAULT:
            //chr = Serial.read();
            if (Serial.read() == 'J') {
              // reset eeprom
              for (int i = 0; i < EEPROM_SIZE; i++) {
                EEPROM.write(i, 0xFF);
              }
              Serial.end();
#if defined(ESP8266)
              EEPROM.commit();
#endif
              resetCPU();
            }
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
        case MENU_HOME:
          Serial.print(F("\n\n********************************************\nCG scale by M.Lehmann - V"));
          Serial.print(CGSCALE_VERSION);
          Serial.print(F("\n\n1  - Set number of load cells ("));
          Serial.print(nLoadcells);
          Serial.print(F(")\n2  - Set distance X1 ("));
          Serial.print(distanceX1);
          Serial.print(F("mm)\n3  - Set distance X2 ("));
          Serial.print(distanceX2);
          Serial.print(F("mm)\n4  - Set distance X3 ("));
          Serial.print(distanceX3);
          Serial.print(F("mm)\n5  - Set reference weight ("));
          Serial.print(refWeight);
          Serial.print(F("g)\n6  - Set reference CG ("));
          Serial.print(refCG);
          Serial.print(F("mm)\n7  - Start autocalibration\n8  - Set calibration factor of load cell 1 ("));
          Serial.print(calFactorLoadcell1);
          Serial.print(F(")\n9  - Set calibration factor of load cell 2 ("));
          Serial.print(calFactorLoadcell2);
          Serial.print(F(")\n10 - Set calibration factor of load cell 3 ("));
          Serial.print(calFactorLoadcell3);
          Serial.print(F(")\n11 - Set value of resistor R1 ("));
          Serial.print(resistorR1);
          Serial.print(F("ohm)\n12 - Set value of resistor R2 ("));
          Serial.print(resistorR2);
          Serial.print(F("ohm)\n13 - Enable battery voltage measurement ("));
          if (enableBatVolt) {
            Serial.print(F("enabled)\n"));
          } else {
            Serial.print(F("disabled)\n"));
          }
          Serial.print(F("14 - Show actual values\n15 - Reset to factory defaults\n\n"));
          Serial.print(F("Please choose the menu number:"));
          updateMenu = false;
          break;
        case MENU_LOADCELLS:
          Serial.print(F("\n\nNumber of load cells: "));
          Serial.println(nLoadcells);
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_DISTANCE_X1:
          Serial.print(F("\n\nDistance X1: "));
          Serial.print(distanceX1);
          Serial.print(F("mm\n"));
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_DISTANCE_X2:
          Serial.print(F("\n\nDistance X2: "));
          Serial.print(distanceX2);
          Serial.print(F("mm\n"));
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_DISTANCE_X3:
          Serial.print(F("\n\nDistance X3: "));
          Serial.print(distanceX3);
          Serial.print(F("mm\n"));
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_REF_WEIGHT:
          Serial.print(F("\n\nReference weight: "));
          Serial.print(refWeight);
          Serial.print(F("g\n"));
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_REF_CG:
          Serial.print(F("\n\nReference CG: "));
          Serial.print(refCG);
          Serial.print(F("mm\n"));
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_AUTO_CALIBRATE:
          Serial.print(F("\n\nPlease put the reference weight on the scale.\nStart auto calibration (J/N)?\n"));
          updateMenu = false;
          break;
        case MENU_LOADCELL1_CALIBRATION_FACTOR:
          Serial.print(F("\n\nCalibration factor of load cell 1: "));
          Serial.println(calFactorLoadcell1);
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_LOADCELL2_CALIBRATION_FACTOR:
          Serial.print(F("\n\nCalibration factor of load cell 2: "));
          Serial.println(calFactorLoadcell2);
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_LOADCELL3_CALIBRATION_FACTOR:
          Serial.print(F("\n\nCalibration factor of load cell 3: "));
          Serial.println(calFactorLoadcell3);
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_RESISTOR_R1:
          Serial.print(F("\n\nValue of resistor R1: "));
          Serial.println(resistorR1);
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_RESISTOR_R2:
          Serial.print(F("\n\nValue of resistor R2: "));
          Serial.println(resistorR2);
          Serial.print(newValueText);
          updateMenu = false;
          break;
        case MENU_BATTERY_MEASUREMENT:
          Serial.print(F("\n\nEnable battery voltage measurement (J/N)?\n"));
          updateMenu = false;
          break;
        case MENU_SHOW_ACTUAL:
          Serial.print(F("Lc1: "));
          Serial.print(weightLoadCell1);
          Serial.print(F("g  Lc2: "));
          Serial.print(weightLoadCell2);
          if (nLoadcells > 2) {
            Serial.print(F("g  Lc3: "));
            Serial.print(weightLoadCell3);
          }
          Serial.print(F("g  Total weight: "));
          Serial.print(weightTotal);
          Serial.print(F("g  CG length: "));
          Serial.print(CG_length);
          if (nLoadcells > 2) {
            Serial.print(F("mm  CG trans: "));
            Serial.print(CG_trans);
            Serial.print(F("mm"));
          }
          if (enableBatVolt) {
            Serial.print(F("  Battery:"));
            Serial.print(batVolt);
            Serial.print(F("V"));
          }
          Serial.println();
          break;
        case MENU_RESET_DEFAULT:
          Serial.print(F("\n\nReset to factory defaults (J/N)?\n"));
          updateMenu = false;
          break;
      }

    } else {
      updateMenu = true;
    }
  }

#if defined(ESP8266)
  server.handleClient();
#endif

}


#if defined(ESP8266)
void main_page()
{
  char buff[8];
  String webPage = "<!doctype html>";
  webPage += "<html lang=\"en\">";
  webPage += "<head>";
  webPage += "<meta charset=\"utf-8\">";
  webPage += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">";
  webPage += "<meta name=\"description\" content=\"\">";
  webPage += "<meta name=\"author\" content=\"\">";
  webPage += "<link rel=\"icon\" href=\"/favicon.ico\">";
  webPage += "<title>CG scale by M. Lehmann</title>";
  webPage += "<link href=\"/bootstrap.min.css\" rel=\"stylesheet\">";
  webPage += "<link href=\"navbar-top-fixed.css\" rel=\"stylesheet\">";
  webPage += "<meta http-equiv=\"refresh\" content=\"";
  webPage += REFRESH_TIME;
  webPage += "\">";
  webPage += "</head>";
  webPage += "<body>";
  webPage += "<nav class=\"navbar navbar-dark fixed-top bg-dark\">";
  webPage += "<div class=\"container-fluid\">";
  webPage += "<div class=\"navbar-header\">";
  webPage += "<a class=\"navbar-brand\" href=\"#\">";
  webPage += ssid;
  webPage += "</a>";
  webPage += "</div>";
  webPage += "<ul class=\"nav navbar-nav navbar-right\">";
  webPage += "<button type=\"button\" onclick=\"location.href = '/settings'\" class=\"btn btn-danger\">";
  webPage += "<img src=\"settings.png\" alt=\"\" style=\"width:auto;height:30px\">";
  webPage += "</button>";
  webPage += "</ul>";
  webPage += "</div>";
  webPage += "</nav>";
  webPage += "<br><br><br>";
  webPage += "<main role=\"main\" class=\"container\">";
  webPage += "<div class=\"jumbotron\">";

  // print weight
  webPage += "<div class=\"container\">";
  webPage += "<div class=\"row mt-3\">";
  webPage += "<div class=\"col-xs-6\"><img src=\"weight.png\" class=\"pull-left mr-4\" alt=\"weight\" style=\"width:auto;height:50px\"></div>";
  webPage += "<div class=\"col-xs-6 d-flex align-items-center\"><font size=\"6\"> ";
  dtostrf(weightTotal, 5, 1, buff);
  webPage += buff;
  webPage += "g</div>";
  webPage += "</div>";
  webPage += "</div>";

  // print cg
  webPage += "<div class=\"container\">";
  webPage += "<div class=\"row mt-3\">";
  webPage += "<div class=\"col-xs-6\"><img src=\"cg.png\" class=\"pull-left mr-4\" alt=\"weight\" style=\"width:auto;height:50px\"></div>";
  webPage += "<div class=\"col-xs-6 d-flex align-items-center\"><font size=\"6\"> ";
  dtostrf(CG_length, 5, 1, buff);
  webPage += buff;
  webPage += "mm</div>";
  webPage += "</div>";
  webPage += "</div>";

  // print cg trans
  if (nLoadcells > 2) {
    //webPage += "<br>";
    webPage += "<div class=\"container\">";
    webPage += "<div class=\"row mt-3\">";
    webPage += "<div class=\"col-xs-6\"><img src=\"cglr.png\" class=\"pull-left mr-4\" alt=\"weight\" style=\"width:auto;height:50px\"></div>";
    webPage += "<div class=\"col-xs-6 d-flex align-items-center\"><font size=\"6\"> ";
    dtostrf(CG_trans, 5, 1, buff);
    webPage += buff;
    webPage += "mm</div>";
    webPage += "</div>";
    webPage += "</div>";

  }

  // print battery
  if (enableBatVolt) {
    //webPage += "<br>";
    webPage += "<div class=\"container\">";
    webPage += "<div class=\"row mt-3\">";
    webPage += "<div class=\"col-xs-6\"><img src=\"battery.png\" class=\"pull-left mr-4\" alt=\"weight\" style=\"width:auto;height:50px\"></div>";
    webPage += "<div class=\"col-xs-6 d-flex align-items-center\"><font size=\"6\"> ";
    webPage += batVolt;
    webPage += "V</div>";
    webPage += "</div>";
    webPage += "</div>";
  }
  webPage += "</div>";
  webPage += "</main>";
  webPage += "<p><font size=\"2\"><center>(c) 2019 M. Lehmann - Version: ";
  webPage += CGSCALE_VERSION;
  webPage += "</center></font></p>";
  webPage += "<script src=\"/bootstrap.min.js\"></script>";
  webPage += "</body>";
  webPage += "</html>";

  server.send(200, "text/html", webPage);

}

void settings_page()
{
  if ( server.hasArg("nLoadcells")) {
    nLoadcells = server.arg("nLoadcells").toFloat();
    saveLoadcells();
  }
  if ( server.hasArg("distanceX1")) {
    distanceX1 = server.arg("distanceX1").toFloat();
    saveDistanceX1();
  }
  if ( server.hasArg("distanceX2")) {
    distanceX2 = server.arg("distanceX2").toFloat();
    saveDistanceX2();
  }
  if ( server.hasArg("distanceX3")) {
    distanceX3 = server.arg("distanceX3").toFloat();
    saveDistanceX3();
  }
  if ( server.hasArg("refWeight")) {
    refWeight = server.arg("refWeight").toFloat();
    saveRefWeight();
  }
  if ( server.hasArg("refCG")) {
    refCG = server.arg("refCG").toFloat();
    saveRefCG();
  }
  if ( server.hasArg("calFactorLoadcell1")) {
    calFactorLoadcell1 = server.arg("calFactorLoadcell1").toFloat();
    saveCalFactor1();
  }
  if ( server.hasArg("calFactorLoadcell2")) {
    calFactorLoadcell2 = server.arg("calFactorLoadcell2").toFloat();
    saveCalFactor2();
  }
  if ( server.hasArg("calFactorLoadcell3")) {
    calFactorLoadcell3 = server.arg("calFactorLoadcell3").toFloat();
    saveCalFactor3();
  }
  if ( server.hasArg("resistorR1")) {
    resistorR1 = server.arg("resistorR1").toFloat();
    saveResistorR1();
  }
  if ( server.hasArg("resistorR2")) {
    resistorR2 = server.arg("resistorR2").toFloat();
    saveResistorR2();
  }
  if ( server.hasArg("enableBatVolt")) {
    if (server.arg("enableBatVolt") == "ON") {
      enableBatVolt = true;
    } else {
      enableBatVolt = false;
    }
  }
  if ( server.hasArg("calibrate")) {
    auto_calibrate();
  }


  String webPage = "<!doctype html>";
  webPage += "<html lang=\"en\">";
  webPage += "<head>";
  webPage += "<meta charset=\"utf-8\">";
  webPage += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">";
  webPage += "<meta name=\"description\" content=\"\">";
  webPage += "<meta name=\"author\" content=\"\">";
  webPage += "<link rel=\"icon\" href=\"/favicon.ico\">";
  webPage += "<title>CG scale by M. Lehmann</title>";
  webPage += "<link href=\"/bootstrap.min.css\" rel=\"stylesheet\">";
  webPage += "<link href=\"navbar-top-fixed.css\" rel=\"stylesheet\">";
  webPage += "</head>";
  webPage += "<body>";
  webPage += "<nav class=\"navbar navbar-dark fixed-top bg-dark\">";
  webPage += "<div class=\"container-fluid\">";
  webPage += "<div class=\"navbar-header\">";
  webPage += "<a class=\"navbar-brand\" href=\"#\">";
  webPage += ssid;
  webPage += "</a>";
  webPage += "</div>";
  webPage += "<ul class=\"nav navbar-nav navbar-right\">";
  webPage += "<button type=\"button\" onclick=\"location.href = '/'\" class=\"btn btn-danger\">Home</button>";
  webPage += "</ul>";
  webPage += "</div>";
  webPage += "</nav>";
  webPage += "<br><br><br>";
  webPage += "<main role=\"main\" class=\"container\">";

  webPage += "<form action=\"/settings\" id=\"saveForm\">";
  webPage += "<div class=\"form-group\">";
  webPage += "<label>Number of load cells:</label>";
  webPage += "<select class=\"form-control\" name=\"nLoadcells\">";
  if (nLoadcells == 2) {
    webPage += "<option selected>2</option>";
    webPage += "<option>3</option>";
  }
  if (nLoadcells == 3) {
    webPage += "<option>2</option>";
    webPage += "<option selected>3</option>";
  }
  webPage += "</select>";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Distance X1 [mm]:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"distanceX1\" value=\"";
  webPage += distanceX1;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Distance X2 [mm]:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"distanceX2\" value=\"";
  webPage += distanceX2;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Distance X3 [mm]:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"distanceX3\" value=\"";
  webPage += distanceX3;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Reference weight [g]:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"refWeight\" value=\"";
  webPage += refWeight;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Reference CG [mm]:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"refCG\" value=\"";
  webPage += refCG;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Calibration factor of load cell 1:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"calFactorLoadcell1\" value=\"";
  webPage += calFactorLoadcell1;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Calibration factor of load cell 2:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"calFactorLoadcell2\" value=\"";
  webPage += calFactorLoadcell2;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Calibration factor of load cell 3:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"calFactorLoadcell3\" value=\"";
  webPage += calFactorLoadcell3;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Value of resistor R1 [ohm]:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"resistorR1\" value=\"";
  webPage += resistorR1;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Value of resistor R2 [ohm]:</label>";
  webPage += "<input type=\"text\" class=\"form-control\" name=\"resistorR2\" value=\"";
  webPage += resistorR2;
  webPage += "\">";
  webPage += "</div>";

  webPage += "<div class=\"form-group\">";
  webPage += "<label>Voltage measurement:</label>";
  webPage += "<select class=\"form-control\" name=\"enableBatVolt\">";
  if (enableBatVolt) {
    webPage += "<option selected>ON</option>";
    webPage += "<option>OFF</option>";
  } else {
    webPage += "<option>ON</option>";
    webPage += "<option selected>OFF</option>";
  }
  webPage += "</select>";
  webPage += "</div>";

  webPage += "</form>";

  webPage += "<div class=\"btn-group btn-group-lg\">";
  webPage += "<button type=\"button submit\" class=\"btn btn-success btn-lg\" form=\"saveForm\">save parameters</button>";
  webPage += "<form action=\"/settings\" method=\"POST\"><button type=\"button submit\" class=\"btn btn-primary btn-lg\" name=\"calibrate\" value=\"1\">auto calibrate</button></form>";
  webPage += "</div>";

  webPage += "<img src=\"CG_scale_mechanics.png\" class=\"pull-left mr-4\" alt=\"mechanics\" style=\"width:100%\">";

  webPage += "</main>";
  webPage += "<p><font size=\"2\"><center>(c) 2019 M. Lehmann - Version: ";
  webPage += CGSCALE_VERSION;
  webPage += "</center></font></p>";
  webPage += "<script src=\"/bootstrap.min.js\"></script>";
  webPage += "</body>";
  webPage += "</html>";

  server.send(200, "text/html", webPage);
}

void settingsImg()
{
  File file = SPIFFS.open("/settings.png", "r");
  size_t sent = server.streamFile(file, "text/css");
}

void weightImg()
{
  File file = SPIFFS.open("/weight.png", "r");
  size_t sent = server.streamFile(file, "text/css");
}

void cgImg()
{
  File file = SPIFFS.open("/cg.png", "r");
  size_t sent = server.streamFile(file, "text/css");
}

void cgLRimg()
{
  File file = SPIFFS.open("/cglr.png", "r");
  size_t sent = server.streamFile(file, "text/css");
}

void batteryImg()
{
  File file = SPIFFS.open("/battery.png", "r");
  size_t sent = server.streamFile(file, "text/css");
}

void mechanicsImg()
{
  File file = SPIFFS.open("/CG_scale_mechanics.png", "r");
  size_t sent = server.streamFile(file, "text/css");
}

void bootstrap()
{
  File file = SPIFFS.open("/bootstrap.min.css.gz", "r");
  size_t sent = server.streamFile(file, "text/css");
}

void popper()
{
  File file = SPIFFS.open("/popper.min.js.gz", "r");
  size_t sent = server.streamFile(file, "application/javascript");
}

void bootstrapmin()
{
  File file = SPIFFS.open("/bootstrap.min.js.gz", "r");
  size_t sent = server.streamFile(file, "application/javascript");
}
#endif
