/*
  =======================================================================================================================================

    Version 1.6

    1. Connect your Arduino, under tools choose "Arduino Uno", set the right Port and set "Arduino ISP" as the Programmer.
    2. Hit upload (Ctrl-U)

    You might need to download the "AccelStepper" library by Mike McCauley

    Authors: /u/intercipere
             /u/clutchplate
             /u/EorEquis

  =======================================================================================================================================
*/
#include "Globals.h"

String version = "V1.6.33";

///////////////////////////////////////////////////////////////////////////
// Please see the Globals.h file for configuration of the firmware.
///////////////////////////////////////////////////////////////////////////

// See NORTHERN_HEMISPHERE in Globals.h if you not in the northern hemisphere

// This is how many steps your 28BYJ-48 stepper needs for a full rotation. It is almost always 4096.
// This code drives the steppers in halfstep mode for TRK and DEC, and full step for RA
float StepsPerRevolution = 4096;

float speed = 1.000;    // Use this value to slightly increase or decrese tracking speed. The values from the "CAL" menu will be added to this.

int RAspeed = 400;          // You can change the speed and acceleration of the steppers here. Max. Speed = 600. High speeds tend to make
int RAacceleration = 600;   // these cheap steppers unprecice
int DECspeed = 800;
int DECacceleration = 400;

// Define some stepper limits to prevent physical damage to the tracker. This assumes that the home
// point (zero point) has been correctly set to be pointing at the celestial pole.
// Note: these are currently not used
float RAStepperLimit = 15500;         // Going much more than this each direction will make the ring fall off the bearings.

// These are for 47N, so they will need adjustment if you're a lot away from that.
// You can use the CTRL menu to find the limits and place them here. I moved it
// down until my lens was horizontal. Note the DEC number. Then move it up until
// the lens is horizontal and note that number. Put those here. Always watch your
// tracker and hit RESET if it approaches a dangerous area.
// Note: these are currently not used
float DECStepperDownLimit = 10000;    // Going much more than this will make the lens collide with the ring
float DECStepperUpLimit = -22000;     // Going much more than this is going below the horizon.

// These values are needed to calculate the current position during initial alignment.
int PolarisRAHour = 2;
int PolarisRAMinute = 58;
int PolarisRASecond = 0;
// Use something like Stellarium to look up the RA of Polaris in JNow (on date) variant.
// This changes slightly over weeks, so adjust every couple of months.
// This value is from 18.Apr.2020, next adjustment suggested at end 2020
// The same could be done for the DEC coordinates but they dont change significantly for the next 5 years
