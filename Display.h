/*
  Display.h - Peggy 2.0 LED Matrix library for Arduino
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


#ifndef Display_h
#define Display_h

#include <inttypes.h>

#define DEFAULT_FRAMEBUFFER 0			// Use in SetFrameBuffer() to restore default
#define DEFAULT_REFRESHRATE 30.0	// Default display refreshes per second
#define NO_FRAMEBUFFER_CALLBACK 0	// Disable callback

typedef void (*framebuffercallbackptr_t)(void);

class Peggy2Display
{
	public:
		// Constructor
		// Initializes hardware and frame buffer and starts display
		Peggy2Display(void);

		// Set display refresh rate
		// rate - refresh rate in Hz
		// return - nothing
		void SetRefreshRate(float rate);

		// Set display frame buffer
		// bufptr - pointer to a buffer of 100 bytes or DEFAULT_FRAMEBUFFER
		// return - nothing
		void SetFrameBuffer(uint8_t *bufptr);

		// Set callback routine to call at the end of each interrupt
		// callback - pointer to a void fn(void) or NO_FRAMEBUFFER_CALLBACK for no callback
		// return - nothing
		void SetCallback(framebuffercallbackptr_t callback);

		static uint8_t *framebufferptr;				// Pointer to current frame buffer

		static uint8_t framebuffer[100]; 			// Default frame buffer

		static uint32_t ticks; 								// Increments each time display is refreshed

		// Public for interrupts *** DO NOT USE ***
		static const uint8_t framerows[25];
		static uint8_t framerefreshrate;
		static int8_t framerow;
		static framebuffercallbackptr_t framebuffercallbackptr;

	private:
		// Display uses Timer2. Do not use Timer2 in your application.
		static uint8_t SetupTimer2(void);
};

#endif

