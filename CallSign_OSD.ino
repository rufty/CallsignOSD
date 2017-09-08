/*
Scarab NG OSD ...

 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version. see http://www.gnu.org/licenses/

This work is based on the following open source work :-
 Rushduino                 http://code.google.com/p/rushduino-osd/
 Rush OSD Development      https://code.google.com/p/rush-osd-development/
 Minim OSD                 https://code.google.com/p/arducam-osd/wiki/minimosd

 Its base is taken from "Rush OSD Development" R370

 All credit and full acknowledgement to the incredible work and hours from the many developers, contributors and testers that have helped along the way.
 Jean Gabriel Maurice. He started the revolution. He was the first....

 Please refer to credits.txt for list of individual contributions

 Exceptions:
 Where there are exceptions, these take precedence over the genereric licencing approach
 Elements of the code provided by Pawelsky (DJI specific) are not for commercial use. See headers in individual files for further details.
 Libraries used and typically provided by compilers may have licening terms stricter than that of GNU 3

*/


/*
This has been very substantially pruned for HamRadio use.
It now ONLY displays the callsign (and a throbber)
Upload to a MinimOSD board as "ArduinoPro" ATmega328P.
eMail: bugs@wbh.org <Bill Hill (M1BKF)>
*/


#include <math.h>
#include <stdio.h>
#include <string.h>


// The callsign to display.
#define CALLSIGN "m1bkf/p"
// What sort of signal: NTSC = 0 or PAL = 1
#define SIGNALTYPE 1


#define LO_SPEED_CYCLE   100
#define HI_SPEED_CYCLE   10
#define END_string       0xff
#define MAX7456ADD_VM0   0x00
#define MAX7456ADD_DMM   0x04
#define MAX7456ADD_DMAH  0x05
#define MAX7456ADD_DMAL  0x06
#define MAX7456ADD_DMDI  0x07
#define MAX7456ENABLE    PORTD&=B10111111 ;
#define MAX7456DISABLE   PORTD|=B01000000 ;
#define MAX7456HWRESET   PORTB&=B11111011;delay(100);PORTB|=B00000100 ;
#define MAX7456SETHARDWAREPORTS  DDRB|=B00101100;DDRB&=B11101111;DDRD|=B01000000;DDRD&=B11111011 ;


struct
  {
  uint8_t  tenthSec ;
  unsigned long loopcount ;
  unsigned long currentMillis ;
  unsigned long previous_millis_low ;
  unsigned long previous_millis_high ;
  }
timer ;

char screen[480] ;
uint16_t MAX_screen_size ;


uint8_t
spi_transfer ( uint8_t data )
  {
  SPDR = data ;
  while ( ! ( SPSR & ( 1 << SPIF ) ) ) ;
  return SPDR ;
  }


void
MAX7456_Send ( uint8_t add, uint8_t data )
  {
  spi_transfer ( add ) ;
  spi_transfer ( data ) ;
  }


void
MAX7456_WriteString ( const char* string, int addr )
  {
  char* screenp = &screen[addr] ;
  while ( *string ) *screenp++ = *string++ ;
  }


void
MAX7456_DrawScreen()
  {
  uint16_t xx ;
  MAX7456ENABLE ;
  MAX7456_Send ( MAX7456ADD_DMAH, 0 ) ;
  MAX7456_Send ( MAX7456ADD_DMAL, 0 ) ;
  MAX7456_Send ( MAX7456ADD_DMM, 1 ) ;
  for ( xx = 0; xx < MAX_screen_size; ++xx )
    {
    MAX7456_Send ( MAX7456ADD_DMDI, screen[xx] ) ;
    screen[xx] = ' ' ;
    }
  MAX7456_Send ( MAX7456ADD_DMDI, END_string ) ;
  MAX7456_Send ( MAX7456ADD_DMM, 0 ) ;
  MAX7456DISABLE ;
  }


void
MAX7456Setup ( void )
  {
  uint8_t MAX7456_reset = 0x0C ;
  uint8_t MAX_screen_rows ;
  MAX7456DISABLE ;
  MAX7456HWRESET ;
  SPCR = ( 1 << SPE ) | ( 1 << MSTR ) ;
  SPSR = ( 1 << SPI2X ) ;
  uint8_t spi_junk ;
  spi_junk = SPSR ;
  spi_junk = SPDR ;
  delay ( 10 ) ;
  MAX7456ENABLE ;
  // PAL
  if ( SIGNALTYPE )
    {
    MAX7456_reset = 0x4C ;
    MAX_screen_size = 480 ;
    MAX_screen_rows = 16 ;
    }
  // NTSC
  else
    {
    MAX_screen_size = 390 ;
    MAX_screen_rows = 13 ;
    }
  MAX7456_Send ( MAX7456ADD_VM0, MAX7456_reset ) ;
  MAX7456DISABLE ;
  }


void
setup()
  {
  MAX7456SETHARDWAREPORTS ;
  MAX7456Setup() ;
  timer.currentMillis = millis() ;
  timer.previous_millis_low = timer.currentMillis ;
  timer.previous_millis_high = timer.currentMillis ;
  }


void
loop()
  {
  timer.currentMillis = millis() ;
  timer.loopcount++ ;
  if ( ( timer.currentMillis - timer.previous_millis_low ) >= LO_SPEED_CYCLE )
    {
    timer.previous_millis_low += LO_SPEED_CYCLE ;
    timer.tenthSec++ ;
    timer.tenthSec %= 10 ;
    }
  if ( ( timer.currentMillis - timer.previous_millis_high ) >= HI_SPEED_CYCLE )
    {
    timer.previous_millis_high += HI_SPEED_CYCLE ;
    MAX7456_DrawScreen() ;
    MAX7456_WriteString ( CALLSIGN,  64 + ( 30 * 12 ) - 2 ) ;
    if ( timer.tenthSec > 4 ) MAX7456_WriteString ( "+", 64 + ( 30 * 12 ) + 23 ) ; else MAX7456_WriteString ( "-", 64 + ( 30 * 12 ) + 23 ) ;
    }
  }
