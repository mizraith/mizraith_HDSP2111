/*************************************************** 
  This is a library for controlling an HDSP2111 through
  an MCP23017 port expander using 2 wire i2C.

  Author:  Red Byer      www.redstoyland.com
  Date:    9/3/2013
  
  https://github.com/mizraith/mizraith_HDSP2111
 ****************************************************/

#ifndef _mizraith_HDSP2111_H_
#define _mizraith_HDSP2111_H_

//#include <Wire.h>
#include <avr/pgmspace.h>
#include "Adafruit_MCP23017.h"

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif


        
class mizraith_HDSP2111 {
    Adafruit_MCP23017 mcp_display;
    uint8_t mcp_display_addr;
    
    char * BLANK_STRING;

    const static uint8_t NUMBER_OF_DISPLAYS = 2;

    /* Structure containing state function and data */
    struct display_data  {
	    unsigned long LAST_UPDATE;
	    char         *TEXT;
	    uint8_t       TEXT_LENGTH;      //auto-calculated
	    uint8_t       SCROLL_POSITION;  //[0:stringlength-1]
	    uint16_t      SCROLL_DELAY;
	    bool          SCROLL_COMPLETE;  //  (sets to 1 at end of string and stops operation)
	    bool   	      TEXT_CHANGED;
    } DISPLAY_DATA[NUMBER_OF_DISPLAYS];

	
  public:
      mizraith_HDSP2111(void);
      void setup(uint8_t mcpaddr);
      void resetDisplays(); 
	  void resetDisplay(uint8_t displaynum);
	  
	  //is the scroll flag complete on this
	  bool isScrollComplete(uint8_t displaynum);
	  void setScrollCompleteFlag(bool flag, uint8_t displaynum);
	  void setScrollPosition(uint8_t pos, uint8_t displaynum);
	  void automaticallyResetScrollFlagAndPosition(uint8_t displaynum);
	  void automaticallyResetScrollFlagAndPositions(void);
	  
	  
	  //set the delay between scroll steps. Default is 150ms
	  void setScrollDelay(uint16_t delayms, uint8_t displaynum);
	  
	  
	  
	  //useful for tweaking the same display string if scrolling
	  void setDisplayString(char *words, uint8_t displaynum);
	  
	  //RECOMMENDED #1
	  //set display string and start over as if new
	  void setDisplayStringAsNew(char *words, uint8_t displaynum);
	  
	  char * getDisplayString(uint8_t displaynum);
	    
  
	  //PRIMARY CONVENIENCE ALL-IN-ONE METHOD TO BE CALLED EVERY LOOP
	  // wraps up the automatic scroll flag reset with the 
	  //update displays method.
	  void GoDogGo(void);       
	  
	  
	  //RECOMMENDED METHOD #2
	  //convenience method for updating both displays, whether
	  //they be scrolling or static. This method should be
	  //called more frequently than the desired scroll rate...
	  // This method will not update if:
	  //    (a) short enough (therefore static) and unchanged
	  //    (b) not yet reached scroll delay since last update  
	  //       (calls updateDisplayScroll)
	  void updateDisplays();
	  
	 //these could be private
	  void writeDisplay(char *input, uint8_t displaynum); 
	  void updateDisplayScroll(uint8_t displaynum);
	  
	  void DEBUG_PrintDisplayData( void );
	  
	  
  private:
      bool stringLengthChanged( uint8_t displaynum );
 
};

//--------- MCP23017 --> HDSP2111 HOOK UPS --------------- 
//On the MCP23017 there is PORT A and PORT B
//The library numbers these pins (for convenience) 0:15
//the following are for the display MCP23017
#define HDSP_A0   0
#define HDSP_A1   1
#define HDSP_A2   2
#define HDSP_A3   3
// NOTE:  A4 is always held HIGH for this implementation
#define HDSP_RD   4
#define HDSP_WR   5
#define HDSP_CE1  6
#define HDSP_CE2  7

#define HDSP_D0   8
#define HDSP_D1   9
#define HDSP_D2   10
#define HDSP_D3   11
#define HDSP_D4   12
#define HDSP_D5   13
#define HDSP_D6   14
#define HDSP_D7   15


#endif
