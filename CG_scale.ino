/*
  ------------------------------------------------------------------
                            CG scale
                      (c) 2019 by M. Lehmann
  ------------------------------------------------------------------
*/
#define CGSCALE_VERSION "1.0"
/*

  ******************************************************************
  history:
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

// Settings in separate file
#include "settings.h"

// HX711 constructor (dout pin, sck pint):
HX711_ADC LoadCell_1(PIN_LOADCELL1_DOUT, PIN_LOADCELL1_PD_SCK);
HX711_ADC LoadCell_2(PIN_LOADCELL2_DOUT, PIN_LOADCELL2_PD_SCK);
HX711_ADC LoadCell_3(PIN_LOADCELL3_DOUT, PIN_LOADCELL3_PD_SCK);

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
  MENU_BATTERY_MEASUREMENT,
  MENU_SHOW_ACTUAL,
  MENU_RESET_DEFAULT
};

// EEprom parameter addresses
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
  P_REF_CG =                            P_REF_WEIGHT + sizeof(float)
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
static const String PROGMEM newValueText = "Set new value:";

// load default values
uint8_t nLoadcells = NUMBER_LOADCELLS;
float distanceX1 = DISTANCE_X1;
float distanceX2 = DISTANCE_X2;
float distanceX3 = DISTANCE_X3;
float calFactorLoadcell1 = LOADCELL1_CALIBRATION_FACTOR;
float calFactorLoadcell2 = LOADCELL2_CALIBRATION_FACTOR;
float calFactorLoadcell3 = LOADCELL3_CALIBRATION_FACTOR;
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
unsigned long lastTimeMenu = 0;
unsigned long lastTimeLoadcell = 0;
bool displayInit = false;
bool updateMenu = true;
int menuPage = 0;


// Restart CPU
void(* resetCPU) (void) = 0;


// save calibration factors
void saveCalFactor1() {
  LoadCell_1.setCalFactor(calFactorLoadcell1);
  EEPROM.put(P_LOADCELL1_CALIBRATION_FACTOR, calFactorLoadcell1);
}


void saveCalFactor2() {
  LoadCell_2.setCalFactor(calFactorLoadcell2);
  EEPROM.put(P_LOADCELL2_CALIBRATION_FACTOR, calFactorLoadcell2);
}


void saveCalFactor3() {
  LoadCell_3.setCalFactor(calFactorLoadcell3);
  EEPROM.put(P_LOADCELL3_CALIBRATION_FACTOR, calFactorLoadcell3);
}


void setup() {

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

  // init Loadcells
  LoadCell_1.begin();
  LoadCell_2.begin();
  LoadCell_3.begin();

  // tare
  while (!LoadCell_1.startMultiple(STABILISINGTIME) && !LoadCell_2.startMultiple(STABILISINGTIME) && !LoadCell_3.startMultiple(STABILISINGTIME)) {
  }

  // set calibration factor
  LoadCell_1.setCalFactor(calFactorLoadcell1);
  LoadCell_2.setCalFactor(calFactorLoadcell2);
  LoadCell_3.setCalFactor(calFactorLoadcell3);

  // stabilize scale values
  for (int i = 0; i <= 5; i++) {
    LoadCell_1.update();
    LoadCell_2.update();
    LoadCell_3.update();
    delay(200);
  }

  // init serial
  Serial.begin(9600);

}


void loop() {

  LoadCell_1.update();
  LoadCell_2.update();
  LoadCell_3.update();


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

    float weightTotal;
    float CG_length = 0;
    float CG_trans = 0;
    float batVolt = 0;

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
    }

    // read battery voltage
    if (enableBatVolt) {
      batVolt = (analogRead(VOLTAGE_PIN) / 1024.0) * V_REF * (float(RESISTOR_R1 + RESISTOR_R2) / RESISTOR_R2) / 1000.0;
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
            EEPROM.put(P_NUMBER_LOADCELLS, nLoadcells);
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_DISTANCE_X1:
            distanceX1 = Serial.parseFloat();
            EEPROM.put(P_DISTANCE_X1, distanceX1);
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_DISTANCE_X2:
            distanceX2 = Serial.parseFloat();
            EEPROM.put(P_DISTANCE_X2, distanceX2);
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_DISTANCE_X3:
            distanceX3 = Serial.parseFloat();
            EEPROM.put(P_DISTANCE_X3, distanceX3);
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_REF_WEIGHT:
            refWeight = Serial.parseFloat();
            EEPROM.put(P_REF_WEIGHT, refWeight);
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_REF_CG:
            refCG = Serial.parseFloat();
            EEPROM.put(P_REF_CG, refCG);
            menuPage = 0;
            updateMenu = true;
            break;
          case MENU_AUTO_CALIBRATE:
            if (Serial.read() == 'J') {
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
          case MENU_BATTERY_MEASUREMENT:
            if (Serial.read() == 'J') {
              enableBatVolt = true;
            } else {
              enableBatVolt = false;
            }
            EEPROM.put(P_ENABLE_BATVOLT, enableBatVolt);
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
              for (int i = 0; i < 100; i++) {
                EEPROM.write(i, 0xFF);
              }
              Serial.end();
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
          Serial.print(F(")\n11 - Enable battery voltage measurement ("));
          if (enableBatVolt) {
            Serial.print(F("enabled)\n"));
          } else {
            Serial.print(F("disabled)\n"));
          }
          Serial.print(F("12 - Show actual values\n13 - Reset to factory defaults\n\n"));
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
}
