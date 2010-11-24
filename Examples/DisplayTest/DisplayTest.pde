#include <Display.h>

Peggy2Display Display;

float rate = 60.0;

uint8_t InputReg;
uint8_t InputRegOld;

uint8_t newframebuffer[100] = {
													 0b11111110, 0b00000000, 0b11111100, 0b00000000
													,0b00111100, 0b00000001, 0b11111111, 0b00000000
													,0b00111100, 0b00000011, 0b11100001, 0b00000000
													,0b00111100, 0b00000111, 0b11000001, 0b10000000
													,0b00111100, 0b00000111, 0b10000011, 0b10000000
													,0b00111100, 0b00000111, 0b00000111, 0b10000000
													,0b00111100, 0b00001110, 0b00000111, 0b10000000
													,0b00111100, 0b00011110, 0b00001111, 0b00000000
													,0b00111100, 0b00011110, 0b00011111, 0b00000000
													,0b00111100, 0b00110110, 0b00111110, 0b00000000
													,0b00111100, 0b01110011, 0b11111000, 0b00000000
													,0b11111111, 0b11110001, 0b11110000, 0b00000000
													,0b11111110, 0b01111111, 0b11111111, 0b10000000
													,0b11111100, 0b01110001, 0b11100111, 0b10000000
													,0b00111100, 0b01100001, 0b11100001, 0b10000000
													,0b00111100, 0b01100001, 0b11100000, 0b10000000
													,0b00011110, 0b01100001, 0b11100010, 0b10000000
													,0b00011110, 0b11000001, 0b11100110, 0b00000000
													,0b00011110, 0b11000001, 0b11111110, 0b00000000
													,0b00001111, 0b11000001, 0b11100110, 0b00000000
													,0b00001111, 0b10000001, 0b11100010, 0b10000000
													,0b00001111, 0b10000001, 0b11100000, 0b10000000
													,0b00000111, 0b00000001, 0b11100001, 0b10000000
													,0b00000111, 0b00000001, 0b11100011, 0b10000000
													,0b00000111, 0b00000111, 0b11111111, 0b10000000};

void setup()
{
	PORTB = 0b00000001;	// Pull up on ("OFF/SELECT" button)
	PORTC = 0b00011111;	// Pull-ups on C
	DDRB = 0b11111110;	// B0 is an input ("OFF/SELECT" button)
	DDRC = 0b11100000;	//All inputs

	InputRegOld = (PINC & 0b00011111) | ((PINB & 1)<<5);

	Display = Peggy2Display();
	Display.SetRefreshRate(rate);
	Display.SetCallback(DisplayCallback);
	Fill();

	for (int x = 15; x < 21; x++)
	{
		WritePoint(x, x);
	}
}

void loop()
{
	Display.framebufferptr[0] = Display.ticks / 60;
	
	InputReg = (PINC & 0b00011111) | ((PINB & 1)<<5);	// Input reg measures OFF ( bit 5)
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
					Display.SetFrameBuffer(newframebuffer);
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
					Display.SetFrameBuffer(DEFAULT_FRAMEBUFFER);
		}
		if (InputRegOld & 32)	// s2 "Off/Select" button is pressed... 
		{
			Display.SetCallback(NO_FRAMEBUFFER_CALLBACK);
		}	
	}
	InputRegOld = InputReg;
	Display.framebufferptr[98] = InputReg;
 
	delay(100);
}

// Turn point on or off logically
void WritePoint(uint8_t xPos, uint8_t yPos)
{
	Display.framebufferptr[(yPos*4)+(xPos/8)] ^= (uint8_t) 0x80 >> xPos%8;
}

void Clear() 
{
	for (uint8_t j=0;j<100;j++)
		Display.framebufferptr[j] = 0;
}

void Fill() 
{
	for (uint8_t j=0;j<100;j++)
		Display.framebufferptr[j] = 0xff;
}

void DisplayCallback(void)
{
	Display.framebufferptr[2] = Display.ticks;
}

