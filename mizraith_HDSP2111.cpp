/*************************************************** 
  This is a library for controlling an HDSP2111 through
  an MCP23017 port expander using 2 wire i2C.

  Author:  Red Byer      www.redstoyland.com
  Date:    9/3/2013
  
  https://github.com/mizraith/mizraith_HDSP2111
 ****************************************************/
#include "mizraith_HDSP2111.h"



mizraith_HDSP2111::mizraith_HDSP2111(void) {
    BLANK_STRING = "        ";     
  
    for (uint8_t i=0;  i < NUMBER_OF_DISPLAYS;  i++) {
        DISPLAY_DATA[i].LAST_UPDATE = 0;
        DISPLAY_DATA[i].TEXT = BLANK_STRING;
        DISPLAY_DATA[i].TEXT_LENGTH = 0;
        DISPLAY_DATA[i].SCROLL_POSITION = 0;
        DISPLAY_DATA[i].SCROLL_DELAY = 120;
        DISPLAY_DATA[i].SCROLL_COMPLETE = false;
        DISPLAY_DATA[i].TEXT_CHANGED = false;
    }
}


/**
 * Sets up the target MCP (at mcpaddr) so that all the ports
 * are outputs and ready to begin transmitting to HDSP2111's
 * See HDSP2111 datasheet for address A0,A1,A2 info.
 */
void mizraith_HDSP2111::setup(uint8_t mcpaddr) {
  mcp_display_addr = mcpaddr;
  
  if (mcpaddr > 7) {
    mcpaddr = 7;       //we have an issue!
  }
  
  //  ---------------------------------------------------------
  //   SETUP MCP23017 FOR THE DISPLAY
  //  ---------START MCP DISPLAY COMMUNICATIONS ---------------
//  Serial.println();
//  Serial.print("Setting up HDSP2111 MCP23017 at address: ");
//  Serial.print( MCP23017_ADDRESS | mcp_display_addr , BIN);
//  Serial.println();
  
  Adafruit_MCP23017 mcp_display;
  
  mcp_display.begin(mcp_display_addr);   // use i2C address for display
  mcp_display.setGPIOABMode(0x0000);     // 1=input 0=output  set all as outputs
  
  //HDSP2111 CODE TO "START" UP DISPLAY
  //hold CE and WR high for now -- to keep from inadvertantly writing
  mcp_display.writePin(HDSP_CE1, HIGH);
  mcp_display.writePin(HDSP_CE2, HIGH);
  mcp_display.writePin(HDSP_WR, HIGH);
  
  for(uint8_t i=0; i<NUMBER_OF_DISPLAYS;  i++ ) {
    DISPLAY_DATA[i].LAST_UPDATE = millis();
  }
}


//SUPER EASY CONVENIENCE METHOD  
//Intended to be called once per loop() to keep scrolling and updating going
void mizraith_HDSP2111::GoDogGo(void) {
    automaticallyResetScrollFlagAndPositions();
    updateDisplays();
} 




/**
 * Routine for clearing out ALL displays.
 */
void mizraith_HDSP2111::resetDisplays() {
  for(uint8_t displaynum=1; displaynum <= NUMBER_OF_DISPLAYS; displaynum++) {
      resetDisplay(displaynum);
  }
}

/**
 * Routine for clearing out a display and it's associated
 * variables.  Displaynum should
 * be 1/2 (matching the CE lines).  Updates the 
 * DISPLAYx_LAST_UPDATE counter, too.
 *
 */
void mizraith_HDSP2111::resetDisplay(uint8_t displaynum) {
    if( displaynum > NUMBER_OF_DISPLAYS) {
        return;
    } else {
       uint8_t displayindex = displaynum-1;
       
       DISPLAY_DATA[displayindex].LAST_UPDATE = millis();
       DISPLAY_DATA[displayindex].TEXT = BLANK_STRING;
       DISPLAY_DATA[displayindex].TEXT_LENGTH = 8;
       DISPLAY_DATA[displayindex].SCROLL_POSITION = 0;
       DISPLAY_DATA[displayindex].SCROLL_COMPLETE = false;
       DISPLAY_DATA[displayindex].TEXT_CHANGED = false;
       
       writeDisplay(DISPLAY_DATA[displayindex].TEXT, displaynum);
   }
}




bool mizraith_HDSP2111::isScrollComplete(uint8_t displaynum) {
    if(displaynum > NUMBER_OF_DISPLAYS) {
        return false;
    } else {
        bool sc = DISPLAY_DATA[displaynum-1].SCROLL_COMPLETE;
        return sc;
    } 
}


void mizraith_HDSP2111::setScrollCompleteFlag(bool flag, uint8_t displaynum) {
    if( displaynum > NUMBER_OF_DISPLAYS) {
        return;
    } else {
        DISPLAY_DATA[displaynum-1].SCROLL_COMPLETE = flag;
    }
}


void mizraith_HDSP2111::setScrollPosition(uint8_t pos, uint8_t displaynum) {
    if( displaynum > NUMBER_OF_DISPLAYS) {
        return;
    } else {
        DISPLAY_DATA[displaynum-1].SCROLL_POSITION = pos;    
    }
}
	
// Automatically reset both scroll complete flag and scroll position for 
// both displays.
void mizraith_HDSP2111::automaticallyResetScrollFlagAndPositions(void) {
    for(uint8_t i=1; i <= NUMBER_OF_DISPLAYS; i++) {
        automaticallyResetScrollFlagAndPosition(i);
    }
}	
	 
//Reset scroll flag and scroll position automatically for display.
//Keep calling every loop to keep scrolling going.
void mizraith_HDSP2111::automaticallyResetScrollFlagAndPosition(uint8_t displaynum) {
    if(isScrollComplete(displaynum) ) {           
        setScrollCompleteFlag(false, displaynum);
        setScrollPosition(0, displaynum);
    }
}







	  
//Set the delay in (ms) between scroll steps	  
void mizraith_HDSP2111::setScrollDelay(uint16_t delayms, uint8_t displaynum) {
  if( displaynum-1 < NUMBER_OF_DISPLAYS) {
      DISPLAY_DATA[displaynum-1].SCROLL_DELAY = delayms;
  }
}


//set the display string convenience method that does 2 things
// (1) calculates the string length
// (2.1) if the string length is the same, just slips it in.  useful
//      if we are just editing one character of the string.
// (3) OTHERWISE -- passes off to setDisplayStringAsNew moethod
void mizraith_HDSP2111::setDisplayString(char *words, uint8_t displaynum) {
    if(displaynum > NUMBER_OF_DISPLAYS) {
        return;
    }
    uint8_t displayindex = displaynum - 1;
    
    DISPLAY_DATA[displayindex].TEXT = words;
    
    if (stringLengthChanged(displaynum)) {
        setDisplayStringAsNew(words, displaynum);  //different length, need to restart scroll
    } else {
        DISPLAY_DATA[displayindex].TEXT_CHANGED = true;
    }
} 
//     uint8_t newlength = strlen(words);
//     uint8_t oldlength;
//         
//     oldlength = DISPLAY_DATA[displayindex].TEXT_LENGTH;
//     DISPLAY_DATA[displayindex].TEXT_LENGTH = newlength;
//     
//     if (newlength == oldlength) {
//         DISPLAY_DATA[displayindex].TEXT = words; 
//         DISPLAY_DATA[displayindex].TEXT_CHANGED = true;
//     } else {
//         //different length, need to restart anyway
//         setDisplayStringAsNew(words, displaynum);
//     }
      
    

// This method takes in a string, and syncrhonizes all 
// the supporting variables.  Finally,
// it sets the DISPLAYx_STRING_CHANGED variable to true to refresh static displays
void mizraith_HDSP2111::setDisplayStringAsNew(char *words, uint8_t displaynum) {
    if(displaynum > NUMBER_OF_DISPLAYS) {
      return;
    }
    uint8_t displayindex = displaynum - 1;
    uint8_t newlength = strlen(words);

    DISPLAY_DATA[displayindex].TEXT = words;  
    DISPLAY_DATA[displayindex].TEXT_LENGTH = newlength;
    DISPLAY_DATA[displayindex].SCROLL_POSITION = 0;    
    DISPLAY_DATA[displayindex].SCROLL_COMPLETE = false;
    DISPLAY_DATA[displayindex].TEXT_CHANGED = true;
}



char * mizraith_HDSP2111::getDisplayString(uint8_t displaynum) {
  if(displaynum > NUMBER_OF_DISPLAYS) {
      return BLANK_STRING;
  } else {
      return DISPLAY_DATA[displaynum-1].TEXT;
  }
}





//RECOMMENDED METHOD #2
//convenience method for updating both displays, whether
//they be scrolling or static. This method should be
//called more frequently than the desired scroll rate...
//   In fact, intended to be called every tight "loop"
//
// This method will update if
//    (a) displaystring is short (<8 non-scrolling) AND marked as changed
//    (b) displaystirng is long (>8 scrolling) AND marked as changed
//    (c) displaystring is long (>8)....passed off to updateDisplayScroll
//        to handle the scrolling of the display.
void mizraith_HDSP2111::updateDisplays() {
    
    for(uint8_t i=0; i < NUMBER_OF_DISPLAYS; i++) {
        uint8_t displaynum = i+1;
        
        if(stringLengthChanged(displaynum)) {
            setDisplayStringAsNew(DISPLAY_DATA[i].TEXT , displaynum);
        }
        
        if( (DISPLAY_DATA[i].TEXT_LENGTH <=8) && (!DISPLAY_DATA[i].TEXT_CHANGED) ) {
            //NOTE:  The following lines are "optional", as they may add
            //unnecessary extra traffic. However, by leaving the following 
            //active, the display can 'auto-update' short strings without intervention
            DISPLAY_DATA[i].SCROLL_COMPLETE = false;
            writeDisplay(DISPLAY_DATA[i].TEXT, displaynum);
         }    
         else if( (DISPLAY_DATA[i].TEXT_LENGTH <=8) && (DISPLAY_DATA[i].TEXT_CHANGED) ) {
            //refresh it 
            DISPLAY_DATA[i].SCROLL_COMPLETE = false;
            writeDisplay(DISPLAY_DATA[i].TEXT, displaynum);
         }           
         else if  (DISPLAY_DATA[i].TEXT_LENGTH > 8)  {
            updateDisplayScroll(displaynum);
         }
         DISPLAY_DATA[i].TEXT_CHANGED = false;
    }
}




/**
 * Takes a string of 8 chars and writes it out
 * to the numbered display.  Currently displaynum
 * must = 1 or 2 right now.
 */
void mizraith_HDSP2111::writeDisplay(char *input, uint8_t displaynum) {
  uint8_t portA = 0;
  uint8_t portB = 0;
  
  uint8_t dispCE = 0;
  if (displaynum == 1) {
      dispCE = HDSP_CE1;
  } else if (displaynum==2) {
      dispCE = HDSP_CE2;
  } else {
      Serial.println(F("!!!! ERROR UNDEFINED DISPLAY ADDRESS (writeDisplay) !!!!!"));
  }

 for(int i=0; i<8; i++) {
      portA &= 0xF0;      //clear A0:2 bits before rebuilding
      portA |= 0xF8;      //set A3, RD, WR, CE1, CE2 to high
      
      portA |= i;         //set A0, A1, A2 bits
      
      portB = input[i];   //Cool!  the HDSP2111 uses ASCII mapping.

      //Put these out on the ports, then toggle write pins
      mcp_display.writeGPIOA(portA);
      mcp_display.writeGPIOB(portB);
      delay(1);
      //now toggle
      mcp_display.writePin(dispCE, LOW);
      delay(1);
      mcp_display.writePin(HDSP_WR, LOW);
      delay(1);
      mcp_display.writePin(dispCE, HIGH);
      delay(1);
      mcp_display.writePin(HDSP_WR, HIGH);
      delay(1);
  }

}





/**
 *  Given a preloaded string in DISPLAY1_STRING, 
 * check against a counter and update whenever its
 * been more than DISPLAY1_SCROLL_DELAY long (ms)
 *  Also uses DISPLAY1_SCROLL_POSITION as next
 * start point to display.   You can reset the display
 * by setting this to zero.
 *
 * At end of string will set flag DISPLAY1_SCROLL_COMPLETE
 *
 * Why?  So that we're not blocking operations like
 * the scrollDisplay() method.
 *
 * Params:
 *   uint8_t displaynum   either 1 or 2 for this purpose. 
 * References:
 *   char *DISPLAYx_STRING
 *   uint8_t DISPLAYx_SCROLL_POSITION  [0:stringlength-1]
 *   uint16_t DISPLAYx_SCROLL_DELAY
 *   boolean  DISPLAYx_SCROLL_COMPLETE  (sets to 1 at end of string and stops operation)
 */
void mizraith_HDSP2111::updateDisplayScroll(uint8_t displaynum) {
  char *text;
  char buffer[9];
  unsigned long temp;
  boolean proceed = true;
  uint8_t scrollindex;
  uint8_t displayindex = displaynum - 1 ;
  
  if( displaynum > NUMBER_OF_DISPLAYS) {
    return;
  }
  if (DISPLAY_DATA[displayindex].SCROLL_COMPLETE) {
    return;
  }

  //setup display specific values
  scrollindex = DISPLAY_DATA[displayindex].SCROLL_POSITION;
  text = DISPLAY_DATA[displayindex].TEXT;
  temp = millis() - DISPLAY_DATA[displayindex].LAST_UPDATE;
  if (temp < DISPLAY_DATA[displayindex].SCROLL_DELAY) {
      proceed = false;
  } else {
      proceed = true;
      DISPLAY_DATA[displayindex].LAST_UPDATE = millis();
  }

   //check that it has been long enough since last update 
  if( !proceed ) {
    return;  
  } 
  
  
  //check if our start index just hit the end of the string
  if(text[scrollindex] != 0) {     
      boolean blank = false;
      
      //now fill up our buffer. If we are scrolling
      //past the end of the string, add blank
      //characters
      for (int displaypos = 0; displaypos<8; displaypos++) {
          //seems like a bad habit to over-index the array, doesn't it?
          //but !blank prevents that from actually happening
          if ( !blank && text[scrollindex + displaypos] == 0 ) {  
            blank = true;    //setting blank makes above tast pass through (not over-indexing array)
          }
          
          if ( blank ) {
            buffer[displaypos] = ' ';   //add empty space in place of null
          } else {
            buffer[displaypos] = text[scrollindex+displaypos];  //add char to buffer
          }
      }
      
      buffer[8]=0;
      writeDisplay(buffer, displaynum);   //where '1' is the display number
      
      
      DISPLAY_DATA[displayindex].SCROLL_POSITION++;      
   
   } else {
       //at end of string now write a fully blank line to push
       //that last character off the screen
       for(int j = 0; j<8; j++) {
         buffer[j] = ' ';
       }
       buffer[8]=0;
       writeDisplay(buffer, displaynum);
       
       //start index was at end of string, raise flag
       DISPLAY_DATA[displayindex].SCROLL_COMPLETE = true;
   }
     
   return;

}



bool mizraith_HDSP2111::stringLengthChanged( uint8_t displaynum ) {
    if( displaynum > NUMBER_OF_DISPLAYS) {
        return false;
    } else {
        uint8_t displayindex = displaynum - 1;
        
        uint8_t currentlength = strlen(DISPLAY_DATA[displayindex].TEXT);
        uint8_t storedlength = DISPLAY_DATA[displayindex].TEXT_LENGTH;
        
        return (currentlength != storedlength);
    }
}
 






void mizraith_HDSP2111::DEBUG_PrintDisplayData( void ) {
    Serial.println(F("___HDSP2111_DISPLAY_DATA___"));
    for(uint8_t i=0; i < NUMBER_OF_DISPLAYS; i++) {
        uint16_t p = (uint16_t) &(DISPLAY_DATA[i]);
        Serial.print(F("___ADDR: "));
        Serial.print(p, DEC);
        Serial.print(F("  --->"));
        Serial.println(DISPLAY_DATA[i].TEXT);  
        Serial.print(F("_Update        : "));
        Serial.println(DISPLAY_DATA[i].LAST_UPDATE);
        Serial.print(F("_Length        : "));
        Serial.println(DISPLAY_DATA[i].TEXT_LENGTH);
        Serial.print(F("_ScrollPos     : "));
        Serial.println(DISPLAY_DATA[i].SCROLL_POSITION);
        Serial.print(F("_ScrollDelay   : "));
        Serial.println(DISPLAY_DATA[i].SCROLL_DELAY);
        Serial.print(F("_ScrollComplete: "));
        Serial.println(DISPLAY_DATA[i].SCROLL_COMPLETE );
        Serial.print(F("_TextChanged   : "));
        Serial.println(DISPLAY_DATA[i].TEXT_CHANGED);
    }
}



