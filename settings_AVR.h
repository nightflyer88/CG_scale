/*
  -----------------------------------------------------------
            Settings for AVR based MCUs
  -----------------------------------------------------------
  General settings for the CG scale.
*/

// **** Loadcell hardware settings ****

#define NUMBER_LOADCELLS                3       // if set to 2, the parameters of loadcell 3 are ignored

#define DISTANCE_X1                     30      // mm
#define DISTANCE_X2                     350     // mm
#define DISTANCE_X3                     220     // mm

#define LOADCELL1_CALIBRATION_FACTOR    900     // user set calibration factor
#define LOADCELL2_CALIBRATION_FACTOR    900     // user set calibration factor
#define LOADCELL3_CALIBRATION_FACTOR    900     // user set calibration factor

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


#define PIN_LOADCELL1_DOUT            A2
#define PIN_LOADCELL1_PD_SCK          A3

#define PIN_LOADCELL2_DOUT            A0
#define PIN_LOADCELL2_PD_SCK          A1
  
#define PIN_LOADCELL3_DOUT            A0//A9
#define PIN_LOADCELL3_PD_SCK          A1//A10



// **** Measurement settings ****

#define STABILISINGTIME               3000     // ms

#define UPDATE_INTERVAL_OLED_MENU     500      // ms
#define UPDATE_INTERVAL_LOADCELL      100      // ms

#define SMOOTHING_LOADCELL            0.4     // IIR filter: smoothing value from 0.00-1.00

#define MINIMAL_CG_WEIGHT             10      // g     if lower, no CG is displayed (0mm)
#define MINIMAL_TOTAL_WEIGHT          1       // g     if lower, weight = 0 is displayed



// **** Calibration settings ****

#define REF_WEIGHT                    1500     // g
#define REF_CG                        100      // mm



// **** Display settings ****

// Please UNCOMMENT the display used

U8G2_SH1106_128X64_NONAME_1_HW_I2C oledDisplay(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C oledDisplay(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define DISPLAY_WIDTH                 128
#define DISPLAY_HIGHT                 64

// **** Voltage measurement settings ****

// analog input pin
#define VOLTAGE_PIN                   A7

// supply voltage
#define V_REF                         5000     // set supply voltage from 1800 to 5500mV

// voltage divider
#define RESISTOR_R1                   10000    // ohm
#define RESISTOR_R2                   10000    // ohm

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
#define ENABLE_PERCENTLIST            false

// Battery type
#define BAT_TYPE                      B_VOLT

// Battery cells
#define BAT_CELLS                     2



// **** end of settings ****
#warning AVR settings have been loaded
