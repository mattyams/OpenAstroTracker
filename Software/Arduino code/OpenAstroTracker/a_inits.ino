// If you really want to look through this code, i apologise for my terrible coding
//#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <AccelStepper.h>
#include <LiquidCrystal.h>

#include "Utility.h"
#include "DayTime.hpp"
#include "Mount.hpp"
#include "Globals.h"


#define HALFSTEP 8
#define FULLSTEP 4

//SoftwareSerial BT(10,11);

// The radius of the surface that the belt runs on (in V1 of the ring) was 168.24mm.
// Belt moves 40mm for one stepper revolution (2mm pitch, 20 teeth) or 32mm for one stepper revolution (2mm pitch, 16 teeth)
// RA wheel is 2 x PI x 168.24mm (V2:180mm) circumference = 1057.1mm (V2:1131mm)
// One RA revolution needs 26.43 (1057.1mm / 40mm) or 33.03 (1057.1mm / 32mm) stepper revolutions (V2: 28.27 (1131mm/40mm)or 35.34 (1131mm/32mm))
// Which means 108245 steps (26.43 x 4096) or 135291 steps (33.03 x 4096) moves 360 degrees (V2: 115812 steps (28.27 x 4096) or 144753 steps (35.34 x 4096))
// So there are 300.1 (108245 / 360) or 375.8 (135291 / 360) steps/degree  (V2: 322 (115812 / 360) or 402.1 (144753 / 360))
// Theoretically correct RA tracking speed is 1.246586 (300 x 14.95903 / 3600) or 1.562388 (376 x 14.95903 / 3600) (V2 : 1.333800 (322 x 14.95903 / 3600) or 
// 1.670425 (402 x 14.95903 / 3600) steps/sec.

// Belt moves 40mm for one stepper revolution (2mm pitch, 20 teeth) or 32mm for one stepper revolution (2mm pitch, 16 teeth).
// DEC wheel is 2 x PI x 90mm circumference which is 565.5mm
// One DEC revolution needs 14.13 (565.5mm/40mm) or 17.67 (565.5mm/32mm) stepper revolutions
// Which means 57907 steps (14.14 x 4096) or 72384 (17.67 x 4096) moves 360 degrees
// So there are 160.85 (57907/360) or 201 (72384 / 360) steps/degree

// Steps per Degree based off gear size
#if (RA_GEAR_SIZE == 20 && DEC_GEAR_SIZE == 20 && RING_VERSION == 1)
  int RAStepsPerDegree 300;
  int DECStepsPerDegree 161
#elif (RA_GEAR_SIZE == 20 && DEC_GEAR_SIZE == 20 && RING_VERSION == 2)
  int RAStepsPerDegree 322;
  int DECStepsPerDegree 161
#elif (RA_GEAR_SIZE == 16 && DEC_GEAR_SIZE == 16 && RING_VERSION == 1)
  int RAStepsPerDegree 376;
  int DECStepsPerDegree 201
#elif (RA_GEAR_SIZE == 16 && DEC_GEAR_SIZE == 16 && RING_VERSION == 2)
  int RAStepsPerDegree 402;
  int DECStepsPerDegree 201
#else
  #error "Please Check Your Selections and Ensure Correct Values are entered (20 or 16 for gear size and 1 or 2 for ring version).  Mismatched gear sizes (ex. RA_GEAR_SIZE 20, DEC_GEAR_SIZE 16) are not supported at this time"
#endif

// RA Motor pins
#ifdef INVERT_RA_DIR
  #define RAmotorPin1  12    // IN1 auf ULN2003 driver 1    //  2 / 22
  #define RAmotorPin3  11    // IN2 auf ULN2003 driver 1    //  3 / 24
  #define RAmotorPin2  3     // IN3 auf ULN2003 driver 1    // 11 / 26
  #define RAmotorPin4  2     // IN4 auf ULN2003 driver 1    // 12 / 28
#else
  #define RAmotorPin1  2     // IN1 auf ULN2003 driver 1    //  2 / 22
  #define RAmotorPin3  3     // IN2 auf ULN2003 driver 1    //  3 / 24
  #define RAmotorPin2  11    // IN3 auf ULN2003 driver 1    // 11 / 26
  #define RAmotorPin4  12    // IN4 auf ULN2003 driver 1    // 12 / 28
#endif

// DEC Motor pins
#ifdef INVERT_DEC_DIR
  #define DECmotorPin1  18    // IN1 auf ULN2003 driver 2
  #define DECmotorPin2  16    // IN2 auf ULN2003 driver 2
  #define DECmotorPin3  17    // IN3 auf ULN2003 driver 2
  #define DECmotorPin4  15    // IN4 auf ULN2003 driver 2
#else
  #define DECmotorPin1  15    // IN1 auf ULN2003 driver 2
  #define DECmotorPin2  17    // IN2 auf ULN2003 driver 2
  #define DECmotorPin3  16    // IN3 auf ULN2003 driver 2
  #define DECmotorPin4  18    // IN4 auf ULN2003 driver 2
#endif

// Menu IDs
#define RA_Menu 0
#define DEC_Menu 1
#define HA_Menu 2
#define Heat_Menu 4
#define Calibration_Menu 5
#define Control_Menu 6
#define Home_Menu 7
#define POI_Menu 8
#define Status_Menu 9

// How many menu items at most?
#define MAXMENUITEMS 10

#ifdef SUPPORT_GUIDED_STARTUP
bool inStartup = true;        // Start with a guided startup
#else
bool inStartup = false;        // Start with a guided startup
#endif

// Serial control variables
bool inSerialControl = false; // When the serial port is in control
bool quitSerialOnNextButtonRelease = false; // Used to detect SELECT button to quit Serial mode.

// Calibration variables
float inputcal;              // calibration variable set form as integer. Added to speed after dividing by 10000
int calDelay = 150;          // The current delay in ms when changing calibration value. The longer a button is depressed, the smaller this gets.

//// Variables for use in the CONTROL menu
bool inControlMode  = false;  // Is manual control enabled

// Global variables
bool isUnreachable = false;

// RA variables
int RAselect;

// DEC variables
int DECselect;

// HA variables
int HAselect;

#ifdef SUPPORT_HEATING
// HEAT menu variables
int heatselect;   // Which stepper are we changing?
int RAheat = 0;   // Are we heating the RA stepper?
int DECheat = 0;  // Are we heating the DEC stepper?
#endif

//debugging
String inBT;
