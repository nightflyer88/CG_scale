/*
  -----------------------------------------------------------
            Defaults
  -----------------------------------------------------------
  It is recommended not to change these default values and parameters.
*/

// distance
enum {
  X1,
  X2,
  X3
};

// loadcells
enum {
  LC1,
  LC2,
  LC3
};

// resistors
enum {
  R1,
  R2
};


// serial menu
enum {
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
  MENU_BATTERY_CELLS,
  MENU_SHOW_ACTUAL,
#if defined(ESP8266)
  MENU_WIFI_INFO,
#endif
  MENU_RESET_DEFAULT
};


// console msg type
enum {
  T_BOOT,
  T_RUN,
  T_ERROR,
  T_WIFI,
  T_UPDATE,
  T_HTTPS
};


#if defined(ESP8266)
// https update
enum {
  PROBE_UPDATE,
  UPDATE_FIRMWARE,
  UPDATE_SPIFFS
};
#endif


// EEprom parameter addresses
enum {
  P_NUMBER_LOADCELLS =                  1,
  P_DISTANCE_X1 =                       2,
  P_DISTANCE_X2 =                       P_DISTANCE_X1 + sizeof(float),
  P_DISTANCE_X3 =                       P_DISTANCE_X2 + sizeof(float),
  P_LOADCELL1_CALIBRATION_FACTOR =      P_DISTANCE_X3 + sizeof(float),
  P_LOADCELL2_CALIBRATION_FACTOR =      P_LOADCELL1_CALIBRATION_FACTOR + sizeof(float),
  P_LOADCELL3_CALIBRATION_FACTOR =      P_LOADCELL2_CALIBRATION_FACTOR + sizeof(float),
  P_BAT_TYPE =                          P_LOADCELL3_CALIBRATION_FACTOR + sizeof(float),
  P_BATT_CELLS =                        P_BAT_TYPE + 1,
  P_REF_WEIGHT =                        P_BATT_CELLS + 3,
  P_REF_CG =                            P_REF_WEIGHT + sizeof(float),
  P_RESISTOR_R1 =                       P_REF_CG + sizeof(float),
  P_RESISTOR_R2 =                       P_RESISTOR_R1 + sizeof(float),
#if defined(__AVR__)
  EEPROM_SIZE =                         P_RESISTOR_R2 + sizeof(float)
#elif defined(ESP8266)
  P_SSID_STA =                          P_RESISTOR_R2 + sizeof(float),
  P_PASSWORD_STA =                      P_SSID_STA + MAX_SSID_PW_LENGHT + 1,
  P_SSID_AP =                           P_PASSWORD_STA + MAX_SSID_PW_LENGHT + 1,
  P_PASSWORD_AP =                       P_SSID_AP + MAX_SSID_PW_LENGHT + 1,
  P_MODELNAME =                         P_PASSWORD_AP + MAX_SSID_PW_LENGHT + 1,
  P_ENABLE_UPDATE =                     P_MODELNAME + MAX_MODELNAME_LENGHT + 1,
  P_ENABLE_OTA =                        P_ENABLE_UPDATE + 1,
  EEPROM_SIZE =                         P_ENABLE_OTA + 1
#endif
};



// battery image 12x6
const unsigned char batteryImage[] U8X8_PROGMEM = {
  0xff, 0x03, 0x01, 0x0e, 0x01, 0x0e, 0x01, 0x0e, 0x01, 0x0e, 0xff, 0x03
};

// weight image 18x18
const unsigned char weightImage[] U8X8_PROGMEM = {
  0x00, 0x00, 0xfc, 0x00, 0x03, 0xfc, 0x80, 0x04, 0xfc, 0x80, 0x04, 0xfc,
  0x80, 0x07, 0xfc, 0xf8, 0x7f, 0xfc, 0x08, 0x40, 0xfc, 0x08, 0x40, 0xfc,
  0x08, 0x47, 0xfc, 0x84, 0x84, 0xfc, 0x84, 0x84, 0xfc, 0x04, 0x87, 0xfc,
  0x04, 0x84, 0xfc, 0x02, 0x03, 0xfd, 0x02, 0x00, 0xfd, 0x02, 0x00, 0xfd,
  0xfe, 0xff, 0xfd, 0x00, 0x00, 0xfc
};

// CG image 18x18
const unsigned char CGImage[] U8X8_PROGMEM = {
  0x00, 0x02, 0xfc, 0xc0, 0x1f, 0xfc, 0x30, 0x7e, 0xfc, 0x08, 0xfe, 0xfc,
  0x04, 0xfe, 0xfc, 0x04, 0xfe, 0xfd, 0x02, 0xfe, 0xfd, 0x02, 0xfe, 0xfd,
  0x02, 0xfe, 0xff, 0xff, 0x01, 0xfd, 0xfe, 0x01, 0xfd, 0xfe, 0x01, 0xfd,
  0xfe, 0x81, 0xfc, 0xfc, 0x81, 0xfc, 0xfc, 0x41, 0xfc, 0xf8, 0x31, 0xfc,
  0xe0, 0x0f, 0xfc, 0x00, 0x01, 0xfc
};

// CG transverse axis image 18x18
const unsigned char CGtransImage[] U8X8_PROGMEM = {
  0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x04, 0x70, 0xfc,
  0x04, 0x90, 0xfc, 0x04, 0x90, 0xfc, 0x04, 0x70, 0xfc, 0x04, 0x50, 0xfc,
  0x04, 0x90, 0xfc, 0x3c, 0x90, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc,
  0x08, 0x40, 0xfc, 0x04, 0x80, 0xfc, 0x7e, 0xf8, 0xfd, 0x04, 0x80, 0xfc,
  0x08, 0x40, 0xfc, 0x00, 0x00, 0xfc
};



// battery types
#if ENABLE_PERCENTLIST
#define NUMBER_BAT_TYPES     6
#else
#define NUMBER_BAT_TYPES     2
#endif

enum {
  B_OFF,
  B_VOLT,
  B_LIPO,
  B_LIION,
  B_LIFEPO,
  B_NIXX
};

const String battTypName[NUMBER_BAT_TYPES] = {
#if ENABLE_PERCENTLIST
  "OFF",
  "Voltage",
  "LiPo",
  "Li-ion",
  "LiFePo",
  "Nixx"
#else
  "OFF",
  "Voltage"
#endif
};

// battery percent lists
#define DATAPOINTS_PERCENTLIST    21

const PROGMEM float percentList[][DATAPOINTS_PERCENTLIST][2] =
{
  {
    {3.000, 0},   // LiPo
    {3.250, 5},
    {3.500, 10},
    {3.675, 15},
    {3.696, 20},
    {3.718, 25},
    {3.737, 30},
    {3.753, 35},
    {3.772, 40},
    {3.789, 45},
    {3.807, 50},
    {3.827, 55},
    {3.850, 60},
    {3.881, 65},
    {3.916, 70},
    {3.948, 75},
    {3.987, 80},
    {4.042, 85},
    {4.085, 90},
    {4.115, 95},
    {4.150, 100}
  },
  {
    {3.250, 0},   // Li-ion
    {3.300, 5},
    {3.327, 10},
    {3.355, 15},
    {3.377, 20},
    {3.395, 25},
    {3.435, 30},
    {3.490, 40},
    {3.630, 60},
    {3.755, 75},
    {3.790, 80},
    {3.840, 85},
    {3.870, 90},
    {3.915, 95},
    {4.050, 100}
  },
  {
    {2.80, 0},    // LiFePo
    {3.06, 5},
    {3.14, 10},
    {3.17, 15},
    {3.19, 20},
    {3.20, 25},
    {3.21, 30},
    {3.22, 35},
    {3.23, 40},
    {3.24, 45},
    {3.25, 50},
    {3.25, 55},
    {3.26, 60},
    {3.26, 65},
    {3.27, 70},
    {3.28, 75},
    {3.28, 80},
    {3.29, 85},
    {3.29, 90},
    {3.29, 95},
    {3.30, 100}
  },
  {
    {0.900, 0},   // Nixx
    {0.970, 5},
    {1.040, 10},
    {1.090, 15},
    {1.120, 20},
    {1.140, 25},
    {1.155, 30},
    {1.175, 40},
    {1.205, 60},
    {1.220, 75},
    {1.230, 80},
    {1.250, 85},
    {1.280, 90},
    {1.330, 95},
    {1.420, 100}
  }
};
