/*************************************************** 
  This is a library for controlling an HDSP2111 through
  an MCP23017 port expander using 2 wire i2C.

  Author:  Red Byer      www.redstoyland.com
  Date:    9/3/2013
  
  https://github.com/mizraith/mizraith_HDSP2111
 ****************************************************/
#include "mizraith_HDSP2111.h"


////////////////////////////////////////////////////////////////////////////////
char mizraith_HDSP2111::BLANK_STRING[9] = "        ";

mizraith_HDSP2111::mizraith_HDSP2111(void) {
    DISPLAY1_LAST_UPDATE = 0;
    DISPLAY1_STRING_LENGTH = 0;    //auto-calculated
    DISPLAY1_SCROLL_POSITION = 0;   //[0:stringlength-1]
    DISPLAY1_SCROLL_DELAY = 150;
    DISPLAY1_SCROLL_COMPLETE = false;  //  (sets to 1 at end of string and stops operation)
    DISPLAY1_STRING_CHANGED = false;
    
    DISPLAY2_LAST_UPDATE = 0;
    DISPLAY2_STRING_LENGTH = 0;     //auto-calculated
    DISPLAY2_SCROLL_POSITION = 0;   //[0:stringlength-1]
    DISPLAY2_SCROLL_DELAY = 150;
    DISPLAY2_SCROLL_COMPLETE = false;  //  (sets to 1 at end of string and stops operation)
    DISPLAY2_STRING_CHANGED = false;
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
  
  DISPLAY1_LAST_UPDATE = millis();         //set up our timers
  DISPLAY2_LAST_UPDATE = millis();
}


//SUPER EASY CONVENIENCE METHOD  
//Intended to be called once per loop() to keep scrolling and updating going
void mizraith_HDSP2111::GoDogGo(void) {
    automaticallyResetScrollFlagAndPositions();
    updateDisplays();
} 




/**
 * Routine for clearing out both displays.
 */
void mizraith_HDSP2111::resetDisplays() {
  resetDisplay(1);
  resetDisplay(2);
}

/**
 * Routine for clearing out a display and it's associated
 * variables.  Displaynum should
 * be 1/2 (matching the CE lines).  Updates the 
 * DISPLAYx_LAST_UPDATE counter, too.
 *
 */
void mizraith_HDSP2111::resetDisplay(uint8_t displaynum) {
  switch(displaynum) {
  	case 1:
  	    DISPLAY1_LAST_UPDATE = millis();
  	    DISPLAY1_STRING = BLANK_STRING;
  		DISPLAY1_STRING_LENGTH = 8;
  		DISPLAY1_SCROLL_POSITION = 0;
  		DISPLAY1_SCROLL_COMPLETE = false;
  		DISPLAY1_STRING_CHANGED = false;
  		writeDisplay(BLANK_STRING, 1);
  		break;
  	case 2:
  		DISPLAY2_LAST_UPDATE = millis();
  		DISPLAY2_STRING = BLANK_STRING;
  		DISPLAY2_LAST_UPDATE = millis();
  		DISPLAY2_STRING_LENGTH = 8;
  		DISPLAY2_SCROLL_POSITION = 0;
  		DISPLAY2_SCROLL_COMPLETE = false;
  		DISPLAY2_STRING_CHANGED = false;
  		writeDisplay(BLANK_STRING, 2);
  		break;
  }
}






bool mizraith_HDSP2111::isScrollComplete(uint8_t displaynum) {
  switch(displaynum) {
  	case 1:
  		return DISPLAY1_SCROLL_COMPLETE;
  		break;
  	case 2:
  		return DISPLAY2_SCROLL_COMPLETE;
  		break;
  	default:
  	    return true;
  }
}



void mizraith_HDSP2111::setScrollCompleteFlag(bool flag, uint8_t displaynum) {
  switch(displaynum) {
  	case 1:
  		DISPLAY1_SCROLL_COMPLETE = flag;
  		break;
  	case 2:
  		DISPLAY2_SCROLL_COMPLETE = flag;
  		break;
  }
}



void mizraith_HDSP2111::setScrollPosition(uint8_t pos, uint8_t displaynum) {
  switch(displaynum) {
  	case 1:
  		DISPLAY1_SCROLL_POSITION = pos;
  		break;
  	case 2:
  		DISPLAY2_SCROLL_POSITION = pos;
  		break;
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




// Automatically reset both scroll complete flag and scroll position for 
// both displays.
void mizraith_HDSP2111::automaticallyResetScrollFlagAndPositions(void) {
    automaticallyResetScrollFlagAndPosition(1);
    automaticallyResetScrollFlagAndPosition(2);
}


	  
//Set the delay in (ms) between scroll steps	  
void mizraith_HDSP2111::setScrollDelay(uint16_t delayms, uint8_t displaynum) {
  switch(displaynum) {
  	case 1:
  		DISPLAY1_SCROLL_DELAY = delayms;
  		break;
  	case 2:
  		DISPLAY2_SCROLL_DELAY = delayms;
  		break;
  }
}


//set the display string convenience method that does 2 things
// (1) calculates the string length
// (2.1) if the string length is the same, just slips it in.  useful
//      if we are just editing one character of the string.
// (3) OTHERWISE -- passes off to setDisplayStringAsNew moethod
void mizraith_HDSP2111::setDisplayString(char *words, uint8_t displaynum) {
    uint8_t newlength = strlen(words);
    uint8_t oldlength;
    
    switch(displaynum) {
  	case 1:
  		oldlength = DISPLAY1_STRING_LENGTH;
  		DISPLAY1_STRING_LENGTH = newlength;
  		if (newlength == oldlength) {
            DISPLAY1_STRING = words; 
            DISPLAY1_STRING_CHANGED = true;
        } else {
            //different length, need to restart anyway
            setDisplayStringAsNew(words, displaynum);
        }
  		break;
  	case 2:
  		oldlength = DISPLAY2_STRING_LENGTH;
  		DISPLAY2_STRING_LENGTH = newlength;
  		if (newlength == oldlength) {
            DISPLAY2_STRING = words;   
            DISPLAY1_STRING_CHANGED = true;
        } else {
            //different length, need to restart anyway
            setDisplayStringAsNew(words, displaynum);
        }
  		break;
  }
    
}

// This method takes in a string, and syncrhonizes all 
// the supporting variables.  Finally,
// it sets the DISPLAYx_STRING_CHANGED variable to true to refresh static displays
void mizraith_HDSP2111::setDisplayStringAsNew(char *words, uint8_t displaynum) {
  uint8_t newlength = strlen(words);
  
  switch(displaynum) {
  	case 1:
  		 DISPLAY1_STRING = words;
  		 DISPLAY1_STRING_LENGTH = newlength;
  		 DISPLAY1_SCROLL_POSITION = 0;   //this forces a restart during update
  		 DISPLAY1_SCROLL_COMPLETE = false;
  		 DISPLAY1_STRING_CHANGED = true;
  		break;
  	case 2:
  		 DISPLAY2_STRING = words;
  		 DISPLAY2_STRING_LENGTH = newlength;
  		 DISPLAY2_SCROLL_POSITION = 0;    //this forces a restart during update
  		 DISPLAY2_SCROLL_COMPLETE = false;
  		 DISPLAY2_STRING_CHANGED = true;
  		break;
  }
}



char * mizraith_HDSP2111::getDisplayString(uint8_t displaynum) {
  switch(displaynum) {
  	case 1:
  		return DISPLAY1_STRING;
  		break;
  	case 2:
  		return DISPLAY2_STRING;
  		break;
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
    
    for(int i=1 ; i<3 ; i++) {
        //did the display change
        switch(i) {
          case 1:   
             if( (DISPLAY1_STRING_LENGTH <=8) && (!DISPLAY1_STRING_CHANGED) ) {
                //do nothing, leave display static
             }    
             else if( (DISPLAY1_STRING_LENGTH <=8) && (DISPLAY1_STRING_CHANGED) ) {
                //refresh it 
                DISPLAY1_SCROLL_COMPLETE = false;
                writeDisplay(DISPLAY1_STRING, 1);
             }           
//              else if ( (DISPLAY1_STRING_LENGTH > 8) && (DISPLAY1_LENGTH_CHANGED) ) {
//                 //we have to start over due to new length
//                 DISPLAY1_SCROLL_POSITION = 0;
//                 DISPLAY1_SCROLL_COMPLETE = false;
//                 DISPLAY1_STRING_CHANGED = false;
//                 updateDisplayScroll(1);
//              }
             else if  (DISPLAY1_STRING_LENGTH > 8)  {
                //push it forward, let method handle complete flag
                updateDisplayScroll(1);
             }
             DISPLAY1_STRING_CHANGED = false;
            break;
          case 2:
             if( (DISPLAY2_STRING_LENGTH <=8) && (!DISPLAY2_STRING_CHANGED) ) {
                //do nothing, leave display satic
             } 
             else if( (DISPLAY2_STRING_LENGTH <=8) && (DISPLAY2_STRING_CHANGED) ) {
                //refresh it 
                DISPLAY2_SCROLL_COMPLETE = false;
                writeDisplay(DISPLAY1_STRING, 2);
             }           
//              else if ( (DISPLAY2_STRING_LENGTH > 8) && (DISPLAY2_LENGTH_CHANGED) ) {
//                 //we have to start over due to new length
//                 DISPLAY2_SCROLL_POSITION = 0;
//                 DISPLAY2_SCROLL_COMPLETE = false;
//                 DISPLAY2_STRING_CHANGED = false;
//                 updateDisplayScroll(2);
//              }
             else if  (DISPLAY2_STRING_LENGTH > 8)  {
                //push it forward, let method handle complete flag
                updateDisplayScroll(2);
             }
             DISPLAY2_STRING_CHANGED = false;
             break;
        }
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
  uint8_t i;
  
  
  if(  ((displaynum==1) && DISPLAY1_SCROLL_COMPLETE) || 
       ((displaynum==2) && DISPLAY2_SCROLL_COMPLETE) ){
    return;
  }
  
  //setup display specific values
  switch(displaynum) {
    case 1:
        i = DISPLAY1_SCROLL_POSITION;
        text = DISPLAY1_STRING;
        temp = millis() - DISPLAY1_LAST_UPDATE;
        if (temp < DISPLAY1_SCROLL_DELAY) {
          proceed = false;
        } else {
          proceed = true;
          DISPLAY1_LAST_UPDATE = millis();
        }
        break;
    case 2:
        i = DISPLAY2_SCROLL_POSITION;
        text = DISPLAY2_STRING;
        temp = millis() - DISPLAY2_LAST_UPDATE;
        if (temp < DISPLAY2_SCROLL_DELAY) {
          proceed = false;
        } else {
          proceed = true;
          DISPLAY2_LAST_UPDATE = millis();
        }
        break;
  }

   //check that it has been long enough since last update 
  if( !proceed ) {
    return;  
  } 
  
  
  //check if our start index just hit the end of the string
  if(text[i] != 0) {     
      boolean blank = false;
      
      //now fill up our buffer. If we are scrolling
      //past the end of the string, add blank
      //characters
      for (int j = 0; j<8; j++) {
          //seems like a bad habit to over-index the array, doesn't it?
          //but !blank prevents that from actually happening
          if ( !blank && text[i+j] == 0 ) {  
            blank = true;    //setting blank makes above tast pass through (not over-indexing array)
          }
          
          if ( blank ) {
            buffer[j] = ' ';   //add empty space in place of null
          } else {
            buffer[j] = text[i+j];  //add char to buffer
          }
      }
      
      buffer[8]=0;
      writeDisplay(buffer, displaynum);   //where '1' is the display number
      
      switch (displaynum) {
        case 1:
          DISPLAY1_SCROLL_POSITION++;
          break;
        case 2:
          DISPLAY2_SCROLL_POSITION++;
          break;
      }
            
   
   } else {
       //at end of string now write a fully blank line to push
       //that last character off the screen
       for(int j = 0; j<8; j++) {
         buffer[j] = ' ';
       }
       buffer[8]=0;
       writeDisplay(buffer, displaynum);
       
       //start index was at end of string, raise flag
       switch(displaynum) {
         case 1:
           DISPLAY1_SCROLL_COMPLETE = true;
           break;
         case 2:
           DISPLAY2_SCROLL_COMPLETE = true;
           break;
       }
   }
     
   return;

}






//###########################################################################
//###########################################################################

/**
 * Blocking method to scroll through more than 8 characters at a time
 * Uses displaynum = 1 or 2 at this time....
 */
void mizraith_HDSP2111::scrollDisplayBlocking(char *words, uint8_t displaynum) {
  char buffer[9];
  int i = 0;
  while(words[i] != 0) {
      boolean blank = false;
      
      for (int j = 0; j<8; j++) {
          if ( !blank && words[i+j] == 0 ) {
            blank = true;
          }
          
          if ( blank ) {
            buffer[j] = ' ';
          } else {
            buffer[j] = words[i+j];
          }
      }
      
      buffer[8]=0;
      writeDisplay(buffer, displaynum);  
      
      delay(100);
    
      i++;
   }

}




