/*
  Display.cpp - Peggy 2.0 LED Matrix library for Arduino
  Version 1.1 - 06/12/2008
  Copyright (c) 2008 Arthur J. Dahm III.  All right reserved.
  Email: art@mindlessdiversions.com
  Web: mindlessdiversions.com/peggy2

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

/******************************************************************************
 * Includes
 ******************************************************************************/

extern "C" {
#include <inttypes.h>
#include <binary.h>
#include <avr/interrupt.h>
#include <avr/io.h>
}

#include "Display.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

#define TIMER_CLOCK_FREQ 15625.0	// 62.5kHz for /256 prescale from 16MHz

// Initialize Class Variables //////////////////////////////////////////////////

// Default frame buffer: Modify this array to change display.
uint8_t Peggy2Display::framebuffer[100] = {
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000
													};

// Array of values to send to PORTD to turn on rows in sequence
const uint8_t Peggy2Display::framerows[25] = {
														B00000001, B00000010, B00000011, B00000100, B00000101,
														B00000110, B00000111, B00001000, B00001001, B00001010,
														B00001011, B00001100, B00001101, B00001110, B00001111,
														B00010000, B00100000, B00110000, B01000000, B01010000,
														B01100000, B01110000, B10000000, B10010000, B10100000
														};

uint8_t* Peggy2Display::framebufferptr;	// Pointer to frame buffer to use for display
int8_t Peggy2Display::framerow;					// Current row to display
uint8_t Peggy2Display::framerefreshrate;	// Value to load into TCNT2
uint32_t Peggy2Display::ticks;						// Counts display refreshes
framebuffercallbackptr_t Peggy2Display::framebuffercallbackptr;	// Pointer to callback routine or 0

/******************************************************************************
 * Constructor
 ******************************************************************************/

Peggy2Display::Peggy2Display(void)
{
	// Hardware Initialization
	PORTD = 0;
	DDRD = 0xff;
	
	// SET MOSI, SCK Output, all other SPI as input 
	DDRB |= _BV(5) | _BV(3) | _BV(2) | _BV(1);
	
	// ENABLE SPI, MASTER, CLOCK RATE fck/4
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(DORD);
	
	// Clear SPI LED drivers
	SPDR = 0;
	while (!(SPSR & _BV(SPIF)));
	SPDR = 0;
	while (!(SPSR & _BV(SPIF)));
	SPDR = 0;
	while (!(SPSR & _BV(SPIF)));
	SPDR = 0;
	while (!(SPSR & _BV(SPIF)));

	PORTB |= _BV(1);		//Latch Pulse 
	PORTB &= ~( _BV(1));

	// Initailize variables
	framebufferptr = framebuffer;	// Point to default frame buffer
	framerow = 0;
	ticks = 0;
	framebuffercallbackptr = 0;

  //Start the timer and get the timer reload value.
  framerefreshrate=SetupTimer2();
}

/******************************************************************************
 * User API
 ******************************************************************************/

// Set display refresh rate
// rate - refresh rate in Hz
// return - nothing
void Peggy2Display::SetRefreshRate(float rate)
{
	framerefreshrate = (uint8_t)((256 - (TIMER_CLOCK_FREQ / 25 / rate)) + 0.5);
}

// Set display frame buffer
// bufptr - pointer to a buffer of 100 bytes
// return - nothing
void Peggy2Display::SetFrameBuffer(uint8_t *bufptr)
{
	if (bufptr == DEFAULT_FRAMEBUFFER)
		bufptr = framebuffer;
		
	framebufferptr = bufptr;
}

// Set callback routine to call at the end of each interrupt
// callback - pointer to a void fn(void) or 0 for no callback
// return - nothing
void Peggy2Display::SetCallback(framebuffercallbackptr_t callback)
{
	framebuffercallbackptr = callback;
}

// Private Methods /////////////////////////////////////////////////////////////

// Set up Timer2
//
// Configures the ATMega168 8-Bit Timer2 to generate an interrupt
// at the specified frequency.
//
// Returns the timer load value which must be loaded into TCNT2
// inside the ISR routine.
uint8_t Peggy2Display::SetupTimer2(void)
{
  uint8_t result; //The timer load value.

  TCCR2A = 0;
  TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);

  // Timer2 Overflow Interrupt Enable
  TIMSK2 = _BV(TOIE2);

  // Load the timer for its first cycle
	result = (uint8_t)((256 - (TIMER_CLOCK_FREQ / 25 / DEFAULT_REFRESHRATE)) + 0.5);
  TCNT2 = result;

  return(result);
}

// Timer2 overflow interrupt vector handler
ISR(TIMER2_OVF_vect, ISR_NOBLOCK) {

	uint8_t* framebuffercurrptr;

	// Point to the row after the current row to display
	framebuffercurrptr = &Peggy2Display::framebufferptr[(Peggy2Display::framerow+1)<<2];
	
	SPDR = *--framebuffercurrptr;	// Decrement the byte to display and move it to SPDR
	while (!(SPSR & _BV(SPIF)));	// Wait for transmission to complete

	SPDR = *--framebuffercurrptr;	// Decrement the byte to display and move it to SPDR
	while (!(SPSR & _BV(SPIF)));	// Wait for transmission to complete

	SPDR = *--framebuffercurrptr;	// Decrement the byte to display and move it to SPDR
	while (!(SPSR & _BV(SPIF)));	// Wait for transmission to complete

	SPDR = *--framebuffercurrptr;	// Decrement the byte to display and move it to SPDR
	while (!(SPSR & _BV(SPIF)));	// Wait for transmission to complete
	
	PORTD = 0;										// Turn row off

	PORTB |= _BV(1);							// Send latch pulse 
	PORTB &= ~( _BV(1));

	PORTD = Peggy2Display::framerows[Peggy2Display::framerow++];	// Turn correct row on 
	
	// End of frame: reset and increment ticks
	if (Peggy2Display::framerow > 24)
	{
		Peggy2Display::framerow = 0;
		Peggy2Display::ticks++;
	}
	
	// If there's a callback routine, call it
	if (Peggy2Display::framebuffercallbackptr)
		Peggy2Display::framebuffercallbackptr();

  // Reload the timer and correct for latency.
  uint8_t latency = TCNT2;
  TCNT2 = latency + Peggy2Display::framerefreshrate;
}
