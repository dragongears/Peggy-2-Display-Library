#include <Display.h>

Peggy2Display Display;

float rate = 60.0;

uint8_t InputReg;
uint8_t InputRegOld;

uint8_t framebuffer1[100] = {
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000011, B00000000, B11000000, B00000000,
													B00000011, B00000000, B11000000, B00000000,
													B00110000, B11000011, B00001100, B00000000,
													B00110000, B11000011, B00001100, B00000000,
													B00110011, B11111111, B11001100, B00000000,
													B00110011, B11111111, B11001100, B00000000,
													B00111111, B00111100, B11111100, B00000000,
													B00111111, B00111100, B11111100, B00000000,
													B00111111, B11111111, B11111100, B00000000,
													B00111111, B11111111, B11111100, B00000000,
													B00001111, B11111111, B11110000, B00000000,
													B00001111, B11111111, B11110000, B00000000,
													B00000011, B00000000, B11000000, B00000000,
													B00000011, B00000000, B11000000, B00000000,
													B00001100, B00000000, B00110000, B00000000,
													B00001100, B00000000, B00110000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B10100101, B01001001, B10011011, B00000000,
													B10110101, B01010101, B01010010, B10000000,
													B10101101, B01011101, B01011011, B00000000,
													B10100101, B01010101, B01010010, B10000000,
													B10100100, B10010101, B10011010, B10000000};

uint8_t framebuffer2[100] = {
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000011, B00000000, B11000000, B00000000,
													B00000011, B00000000, B11000000, B00000000,
													B00000000, B11000011, B00000000, B00000000,
													B00000000, B11000011, B00000000, B00000000,
													B00000011, B11111111, B11000000, B00000000,
													B00000011, B11111111, B11000000, B00000000,
													B00001111, B00111100, B11110000, B00000000,
													B00001111, B00111100, B11110000, B00000000,
													B00111111, B11111111, B11111100, B00000000,
													B00111111, B11111111, B11111100, B00000000,
													B00111111, B11111111, B11111100, B00000000,
													B00111111, B11111111, B11111100, B00000000,
													B00110011, B00000000, B11001100, B00000000,
													B00110011, B00000000, B11001100, B00000000,
													B00000000, B11000011, B00000000, B00000000,
													B00000000, B11000011, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B00000000, B00000000, B00000000, B00000000,
													B10100101, B01001001, B10011011, B00000000,
													B10110101, B01010101, B01010010, B10000000,
													B10101101, B01011101, B01011011, B00000000,
													B10100101, B01010101, B01010010, B10000000,
													B10100100, B10010101, B10011010, B10000000};

void setup()
{
	PORTB = B00000001;	// Pull up on ("OFF/SELECT" button)
	PORTC = B00011111;	// Pull-ups on C
	DDRB = B11111110;	// B0 is an input ("OFF/SELECT" button)
	DDRC = B11100000;	//All inputs

	InputRegOld = (PINC & B00011111) | ((PINB & 1)<<5);

	Display = Peggy2Display();
	Display.SetRefreshRate(rate);
	Display.SetCallback(DisplayCallback);
}

void loop()
{
	InputReg = (PINC & B00011111) | ((PINB & 1)<<5);	// Input reg measures OFF ( bit 5)
																										// and b1-b5 (bits 0-4).
	InputRegOld ^= InputReg;		// InputRegOld is now nonzero if there has been a change.

	if (InputRegOld)
	{
		InputRegOld &= InputReg;	// InputRegOld is now nonzero if the change was to the
															// button-released (not-pulled-down) state.
															// i.e., the bit that was high after the XOR corresponds
															// to a bit that is presently high.
															// The checks that follow will handle MULTIPLE buttons
															// being pressed and unpressed at the same time.
		if (InputRegOld & 1)	 // b1 "ANY" button is pressed
		{
		}	
		if (InputRegOld & 2)	 //b2 "left" button is pressed
		{
		}	
		if (InputRegOld & 4)	 //b3 "down" button is pressed	
		{
			if (rate > 0)
			{
				rate = rate - 1;
				Display.SetRefreshRate(rate);
			}
		}				
		if (InputRegOld & 8)	 //b4 "up" button is pressed		
		{
			if (rate < 150)
			{
				rate = rate + 1;
				Display.SetRefreshRate(rate);
			}
		}	
		if (InputRegOld & 16)	 //b5 "right" button is pressed
		{
		}
		if (InputRegOld & 32)	// s2 "Off/Select" button is pressed... 
		{
		}	
	}
	InputRegOld = InputReg;

	delay(100);
}

void DisplayCallback(void)
{
	if ((Display.ticks/50) % 2)
		Display.SetFrameBuffer(&framebuffer1[0]);
	else
		Display.SetFrameBuffer(&framebuffer2[0]);
}
