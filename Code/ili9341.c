﻿
#include "ili9341.h"

unsigned int width;
unsigned int height;

int getHeight(){
	return height;
}
int getWidth(){
	return width;
}
void clkSet(){
	CCP = CCP_IOREG_gc;							//disable change protection
	OSC_CTRL |= 0x02;						//enable 32 Mhz oscillator
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));			//wait for oscillator to be stable
	//_delay_us(10);
	CCP = CCP_IOREG_gc;
	OSC_CTRL &= ~OSC_RC2MEN_bm;
	CLK_CTRL |= 0x01 ;						//select 32Mhz clock by making sclksel - 001
}

void setup32(){
	CCP = CCP_IOREG_gc;
	OSC_CTRL |= OSC_RC8MEN_bm;
	while(!(OSC_STATUS & OSC_RC8MRDY_bm));
	CCP = CCP_IOREG_gc;
	CLK_CTRL= CLK_SCLKSEL_RC2M_gc;
	//CCP = CCP_IOREG_gc
	OSC_PLLCTRL = OSC_PLLSRC_RC8M_gc;
	OSC_PLLCTRL= (OSC_PLLFAC4_bm)|(OSC_PLLFAC2_bm);//|(OSC_PLLFAC1_bm)|(OSC_PLLFAC0_bm);		//selecting PLL to run at 16* 8M hz = 128 Mhz.
	//_delay_ms(10);
	OSC_CTRL = OSC_PLLEN_bm;
	while(!(OSC_STATUS & OSC_PLLRDY_bm));
	
	CCP = CCP_IOREG_gc;
	CLK_CTRL= CLK_SCLKSEL_PLL_gc;
	//CLK_PSCTRL = CLK_PSADIV_2_gc;
}



void spi_init_hardware(void){
	SPIDDR |= (1<<DC)|(1<<RESET);		//set DC and RESET to output
	SPIPORT |= (1<<RESET);	//		//output high on RESET
	//clkSet();				//
	setup32();
}
void spi_init(void){
	SPIDDR |= (1<<CS)|(1<<MOSI)|(1<<SCK);		//set cs, mosi, sck as output
	SPIC_CTRL |= (1<<SPI_ENABLE_bp)|(1<<SPI_MASTER_bp);		//enable the SPI and set it as master. (confirm wheater it should be _bm or _bp
	SPIC_CTRL |= (1<<SPI_CLK2X_bp);						//double the spi speed
	//SPIC_CTRL |= (1<<SPI_PRESCALER1_bp);
	//SPIC_CTRL &= ~(1<<SPI_PRESCALER0_bp);
	SPIPORT |= (1<<CS);

}

void spi_send(uint8_t data){
	SPIC_DATA = data;	//write to data register
	//_delay_ms(5);
	while(!(SPIC_STATUS & (1<<SPI_IF_bp))){}	//wait until the flag is set making data
}
void spi_writeCommand(uint8_t command){
	SPIPORT &= ~((1<<DC)|(1<<CS));		//dc and cs both set as low to send
	//_delay_us(5);						//delay 5us
	spi_send(command);
	SPIPORT |= (1<<CS);					//cs is set high
}
void spi_writeData(uint8_t data){
	SPIPORT |= (1<<DC);					//dc set high for data
	//_delay_us(1);
	SPIPORT &= ~(1<<CS);				//CS set high for
	spi_send(data);
	//_delay_us(5);
	SPIPORT |= (1<<CS);
}
void setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
	spi_writeCommand(0x2A);
	spi_writeData(x1>>8);
	spi_writeData(x1);
	spi_writeData(x2>>8);
	spi_writeData(x2);

	spi_writeCommand(0x2B);
	spi_writeData(y1>>8);
	spi_writeData(y1);
	spi_writeData(y2>>8);
	spi_writeData(y2);

	spi_writeCommand(0x2C);
}
void reset(void){
	SPIPORT |= (1<<RESET);		//pull RESET high
	_delay_us(200);
	SPIPORT &= ~(1<<RESET);		//pull RESET low to reset
	_delay_us(200);
	SPIPORT |= (1<<RESET);		//pull RESET high
	_delay_us(200);
}
//void clear();
void begin(void){
	width = TFTHEIGHT;
	height = TFTWIDTH;
	spi_init_hardware();
	spi_init();
	reset();
	spi_writeCommand(0x01);		//software reset
	_delay_ms(1200);
	//clear(ILI9341_YELLOW);
	//power control A
	spi_writeCommand(0xCB);
	spi_writeData(0x39);
	spi_writeData(0x2C);
	spi_writeData(0x00);
	spi_writeData(0x34);
	spi_writeData(0x02);

	//power control B
	spi_writeCommand(0xCF);
	spi_writeData(0x00);
	spi_writeData(0xC1);
	spi_writeData(0x30);

	//driver timing control A
	spi_writeCommand(0xE8);
	spi_writeData(0x85);
	spi_writeData(0x00);
	spi_writeData(0x78);

	//driver timing control B
	spi_writeCommand(0xEA);
	spi_writeData(0x00);
	spi_writeData(0x00);

	//power on sequence control
	spi_writeCommand(0xED);
	spi_writeData(0x64);
	spi_writeData(0x03);
	spi_writeData(0x12);
	spi_writeData(0x81);

	//pump ratio control
	spi_writeCommand(0xF7);
	spi_writeData(0x20);

	//power control,VRH[5:0]
	spi_writeCommand(0xC0);
	spi_writeData(0x23);

	//Power control,SAP[2:0];BT[3:0]
	spi_writeCommand(0xC1);
	spi_writeData(0x10);

	//vcm control
	spi_writeCommand(0xC5);
	spi_writeData(0x3E);
	spi_writeData(0x28);

	//vcm control 2
	spi_writeCommand(0xC7);
	spi_writeData(0x86);

	//memory access control
	spi_writeCommand(0x36);
	spi_writeData(0x48);

	//pixel format
	spi_writeCommand(0x3A);
	spi_writeData(0x55);

	//frameration control,normal mode full colours
	spi_writeCommand(0xB1);
	spi_writeData(0x00);
	spi_writeData(0x18);

	//display function control
	spi_writeCommand(0xB6);
	spi_writeData(0x08);
	spi_writeData(0x82);
	spi_writeData(0x27);

	//3gamma function disable
	spi_writeCommand(0xF2);
	spi_writeData(0x00);

	//gamma curve selected
	spi_writeCommand(0x26);
	spi_writeData(0x01);

	//set positive gamma correction
	spi_writeCommand(0xE0);
	spi_writeData(0x0F);
	spi_writeData(0x31);
	spi_writeData(0x2B);
	spi_writeData(0x0C);
	spi_writeData(0x0E);
	spi_writeData(0x08);
	spi_writeData(0x4E);
	spi_writeData(0xF1);
	spi_writeData(0x37);
	spi_writeData(0x07);
	spi_writeData(0x10);
	spi_writeData(0x03);
	spi_writeData(0x0E);
	spi_writeData(0x09);
	spi_writeData(0x00);

	//set negative gamma correction
	spi_writeCommand(0xE1);
	spi_writeData(0x00);
	spi_writeData(0x0E);
	spi_writeData(0x14);
	spi_writeData(0x03);
	spi_writeData(0x11);
	spi_writeData(0x07);
	spi_writeData(0x31);
	spi_writeData(0xC1);
	spi_writeData(0x48);
	spi_writeData(0x08);
	spi_writeData(0x0F);
	spi_writeData(0x0C);
	spi_writeData(0x31);
	spi_writeData(0x36);
	spi_writeData(0x0F);

	//exit sleep
	spi_writeCommand(0x11);
	_delay_ms(120);
	//display on
	spi_writeCommand(0x29);

}

void pushColor(uint16_t color){
	spi_writeData(color>>8);			//sending first 8 msb by bitwise shifting to the right by 8
	spi_writeData(color);			//sending the 8
}
void clear(uint16_t color){
	uint16_t i,j;
	setAddress(0,0,width-1,height-1);

	for(i=0;i<width;i++)
	{
		for(j=0;j<height;j++)
		{
			pushColor(color);
		}
	}

}
void drawPixel(uint16_t x1, uint16_t y1, uint16_t color){
	if((x1 < 0) ||(x1 >=width) || (y1 < 0) || (y1 >=height)) {
		return;
	}
	setAddress(x1,y1,x1+1,y1+1);

	pushColor(color);
}

//vertical line
void drawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color){
	if((x >=width) || (y >=height)) return;
	if((y+h-1)>=height)
	h=height-y;
	setAddress(x,y,x,y+h-1);
	while(h--)
	{
		pushColor(color);
	}
}
//horizontal line
void drawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color){
	if((x >=width) || (y >=height)) {
		return;
	}
	if((x+w-1)>=width)
	w=width-x;
	setAddress(x,y,x+w-1,y);
	while(w--)
	{
		pushColor(color);
	}
}
void fillrect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	if((x >=width) || (y >=height)){
		return;
	}
	if((x+w-1)>=width)
	w=width-x;
	if((y+h-1)>=height)
	h=height-y;

	setAddress(x, y, x+w-1, y+h-1);

	for(y=h; y>0; y--)
	{
		for(x=w; x>0; x--)
		{
			pushColor(color);
		}
	}
}
void fillScreen(){
		uint16_t i,j;
		setAddress(0,0,width-1,height-1);
		
		for(i=0;i<width;i++)
		{
			for(j=0;j<height;j++)
			{
				uint16_t colorx = rand();
				pushColor(colorx);
			}
		}
}

void setRotation(uint8_t x){
	uint8_t rotation;
	spi_writeCommand(0x36);
	rotation=x;
	switch (rotation)
	{
		case 0:
		spi_writeData(0x40|0x08);
		width = 240;
		height = 320;
		break;
		case 1:
		spi_writeData(0x20|0x08);
		width  = 320;
		height = 240;
		break;
		case 2:
		spi_writeData(0x80|0x08);
		width  = 240;
		height = 320;
		break;
		case 3:
		spi_writeData(0x40|0x80|0x20|0x08);
		width  = 320;
		height = 240;
		break;
	}
}
