/**************************************************************************
 * Demo for dual HDSP2111's.  You will need both my mizraith_MCP23017 and
 * the mizraith_HDSP2111 libraries (see github).   
 *
 * Be sure to check out license.txt and README.txt files
 * 
 * See schematic image in the top level for hookups.
 *
 * FUNCTIONALITY BASICS:
 *   (1) Drives an HDSP2111 through an MCP23017 port expander
 *
 * http://github.com/mizraith
 * 11/5/2013  Red Byer 
 *
 * 11/5/2013 
 * v0.1
 ************************************************************************* */
 
 /******************************************************************************
 DATA CONNECTIONS:   (you're on your own for power/ground connections) 
    Arduino A5  -->  SCL    (i2C clock)
    Arduino A4  -->  SDA    (i2C data)
    D13 - onboard LED (Arduino Nano).
    Be sure to set the address bits (A2A1A0) on your MCP23017  to match addressing below
    
 *******************************************************************************/
#include <Arduino.h>
#include <Wire.h>

#include "Adafruit_MCP23017.h"      // Be sure to use the one from my github library, Adafruit has not updated theirs yet.
#include "mizraith_HDSP2111.h"

 
//--------  ARDUINO PINS -----------------------------
#define LED_PIN         13
//#define INT_PIN         2    //INTERRUPT coming from the MCP

//---------MCP23017 #2 PINS------------------------- 
// For HDSP2111 connection, see the mizraith_HDSP2111.h file
//
//On the MCP23017 there is PORT A and PORT B
//The library numbers these pins (for convenience) 0:15
//the following are for other inputs for MCP23017
mizraith_HDSP2111 mcp_HDSP2111s;
const uint8_t mcp_display_addr = 0b00000000;   //i2C chip address for the display
  
/***************************************************
 *   SETUP
 ***************************************************/

void setup() {
    uint16_t tempwide;
    
    Serial.begin(57600);
    
    serialPrintHeaderString();
    
    checkRAMandExitIfLow(0);
    
    //---------------------------------------------------------
    // SETUP ARDUINO PIN MODES
    //--------------------------------------------------------- 
    //pinMode(INT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    mcp_HDSP2111s.setup(mcp_display_addr);
    mcp_HDSP2111s.resetDisplays();    
    
    mcp_HDSP2111s.DEBUG_PrintDisplayData();
     
    uint8_t k;
    k = (uint8_t) 1;
    mcp_HDSP2111s.setBrightnessForAllDisplays( k );
    k = (uint8_t) 3;
    mcp_HDSP2111s.setScrollSpeedForAllDisplays( k );
    //The folloiwng has the effect of setting the HDSP2111 text pointers
    //to the memory space of the state machine strings.  This gives
    //state machine control over those strings, removing need to constantly
    //call "setnewstring"
    mcp_HDSP2111s.setDisplayStringAsNew("Hello World!", 1);
    mcp_HDSP2111s.setDisplayStringAsNew("This is your HDSP211 Display #2", 2);
    
    mcp_HDSP2111s.DEBUG_PrintDisplayData();
    DEBUG_PrintHDSP1111Strings();
    checkRAMandExitIfLow(1);
}




/***************************************************
 *   SETUP HELPERS
 ***************************************************/

void serialPrintHeaderString() {
  Serial.println();
  Serial.println();
  Serial.println(F("#######################################"));
  Serial.println(F("HDSP2111 Demo Code"));
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Red Byer    www.redstoyland.com"));
  Serial.println(F("http://github.com/mizraith"));
  Serial.println(F("VERSION DATE: 11-05-2016"));
  Serial.println(F("#######################################"));
  Serial.println();
}


//##############################################################################
// #############################################################################
//    MAIN LOOP
//##############################################################################
//##############################################################################
void loop() {
    mcp_HDSP2111s.GoDogGo();   //continuous scrolling and updating of info
}

void DEBUG_PrintHDSP1111Strings( void ) {
    Serial.println(F("__HDSP2111 DISPLAY INFO__"));
    char *text = mcp_HDSP2111s.getDisplayString(1);
    Serial.print((uint16_t) &text,  DEC);
    Serial.print(F(":"));
    Serial.println(text);
    
    text = mcp_HDSP2111s.getDisplayString(2);
    Serial.print((uint16_t) &text,  DEC);
    Serial.print(F(":"));
    Serial.println(text);
    Serial.println();
}


void checkRAMandExitIfLow( uint8_t checkpointnum ) {
  int x = freeRam();
  if (x < 128) {
    Serial.print(F("!!! WARNING LOW SRAM !! FREE RAM at checkpoint "));
    Serial.print(checkpointnum, DEC);  
    Serial.print(F(" is: "));
    Serial.println(x, DEC);
    Serial.println();
    gotoEndLoop();
  } else {
    Serial.print(F("FREE RAM, at checkpoint "));
    Serial.print(checkpointnum, DEC);  
    Serial.print(F(" is: "));
    Serial.println(x, DEC);
  }
}


//Endpoint for program (really for debugging)
void gotoEndLoop( void ) {
  Serial.println(F("--->END<---"));
  while (1) {
    delay(100);   
  }
}


/**
 * Takes one byte and loads up bytestr[8] with the value
 *  eg  "8" loads bytestr with "00000111"
 */
void getBinaryString(uint8_t byteval, char bytestr[]) 
{
	uint8_t bitv;
	int i = 0;
	
    for (i = 0; i < 8; i++) {                    
           bitv = (byteval >> i) & 1;
           if (bitv == 0) {
               bytestr[7-i] = '0';
           } else {
               bytestr[7-i] = '1';
           }
    }
}


//Outside namespace definition.  The freeRam function doesn't seem to
//compile if it is within another namespace.
    /**
     * Extremely useful method for checking AVR RAM status
     * see: http://playground.arduino.cc/Code/AvailableMemory
     * and source:  http://www.controllerprojects.com/2011/05/23/determining-sram-usage-on-arduino/
     */
int freeRam () {
      extern int __heap_start, *__brkval; 
      int v; 
      return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


