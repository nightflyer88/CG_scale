/*
  -----------------------------------------------------------
            Settings for WIFI KIT 8 (ESP8266) board
  -----------------------------------------------------------
  General settings for the CG scale.

  Wifi Kit 8 (https://heltec.org/project/wifi-kit-8/) 
  is a ESP8266 based board, with integrated OLED and battery management
*/

// **** Loadcell hardware settings ****

#define NUMBER_LOADCELLS                  3  // if set to 2, the parameters of loadcell 3 are ignored

#define DISTANCE_X1                       30   // mm
#define DISTANCE_X2                       350  // mm
#define DISTANCE_X3                       220  // mm

#define LOADCELL1_CALIBRATION_FACTOR      900  // user set calibration factor
#define LOADCELL2_CALIBRATION_FACTOR      900  // user set calibration factor
#define LOADCELL3_CALIBRATION_FACTOR      900  // user set calibration factor

/*
CG scale with 2 Loadcells:


<-         ||=== Loadcell 1 ========== Loadcell 2
                
            |        |                     |
            |---X1---|---------X2----------|



CG scale with 3 Loadcells:

       --   ||   Loadcell 1
       |    ||      ||
       |    ||      ||
       |    ||      ||
<-    X3    ||      ||================ Loadcell 2
       |    ||      ||
       |    ||      ||
       |    ||      ||
       --   ||   Loadcell 3 
                
            |        |                     |
            |---X1---|---------X2----------|
*/



#define PIN_LOADCELL1_DOUT         D6
#define PIN_LOADCELL1_PD_SCK       D7

#define PIN_LOADCELL2_DOUT         D3
#define PIN_LOADCELL2_PD_SCK       D8

#define PIN_LOADCELL3_DOUT         D3
#define PIN_LOADCELL3_PD_SCK       D8

// D3 can be used in parallel to the load cell with Wifi Kit 8
#define PIN_TARE_BUTTON            D3



// **** Measurement settings ****

#define STABILISINGTIME            3000  // ms

#define UPDATE_INTERVAL_OLED_MENU  500  // ms
#define UPDATE_INTERVAL_LOADCELL   100   // ms

#define SMOOTHING_LOADCELL         0.4  // IIR filter: smoothing value from 0.00-1.00

#define MINIMAL_CG_WEIGHT          10    // g     if lower, no CG is displayed (0mm)
#define MINIMAL_TOTAL_WEIGHT       1  // g     if lower, weight = 0 is displayed



// **** Calibration settings ****

#define REF_WEIGHT                 1500  // g
#define REF_CG                     100       // mm



// **** Display settings ****

// Wifi Kit 8 has a fixed wired 128x32 display
U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C oledDisplay(U8G2_R0, /* reset=*/16, /* clock=*/5, /* data=*/4);

#define DISPLAY_WIDTH              128
#define DISPLAY_HIGHT              32

// **** Voltage measurement settings ****

// analog input pin
#define VOLTAGE_PIN                A0

// supply voltage
#define V_REF                      3300  // set supply voltage from 1800 to 5500mV

// voltage divider
#define RESISTOR_R1                20000  // ohm
#define RESISTOR_R2                10000  // ohm

/*
                  voltage input
                     |
                     |
                    | |
                    | |  R1
                    | |
                     |
  analog Pin  <------+
                     |
                    | |
                    | |  R2
                    | |
                     |
                     |
                    GND
*/

// calculate voltage to percent
#define ENABLE_PERCENTLIST         true

// Battery type
#define BAT_TYPE                   B_VOLT

// Battery cells
#define BAT_CELLS                  2



// **** Wifi settings ****

#define MAX_SSID_PW_LENGHT         64

// Station mode: connect to available network
#define SSID_STA                   "myWiFi"
#define PASSWORD_STA               ""
#define TIMEOUT_CONNECT            25000  //ms

// Access point mode: create own network
#define SSID_AP                    "CG scale"
#define PASSWORD_AP                ""
const char ip[4] =                 {1,2,3,4};  // default IP address

#define ENABLE_MDNS                true  // enable mDNS to reach the webpage with hostname.local
#define ENABLE_OTA                 true   // enable over the air update



// **** https update settings ****

#define ENABLE_UPDATE              true
#define HTTPS_PORT                 443
#define HOST                       "github.com"
#define URL                        "/nightflyer88/CG_scale/releases/latest"



// **** Model memory settings ****

#define MAX_MODELNAME_LENGHT       32    // max chars
#define DEFAULT_NAME               "Model"       // default model name
#define MODEL_FILE                 "/models.json"  // file to store models



// **** virtual weight settings ****

#define MAX_VIRTUAL_WEIGHT         10



// **** end of settings ****
#warning WIFI KIT 8 settings have been loaded
