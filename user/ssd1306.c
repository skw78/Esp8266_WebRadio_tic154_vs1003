#include <stdio.h>
#include "osapi.h"
#include "os_type.h"
#include "font_6x8.h"
#include "ssd1306.h"


#define SSD1306_WRITE 0x78
#define SSD1306_READ 0x79
unsigned char SSD1306_row_cnt=0, SSD1306_col_cnt=0;


void ICACHE_FLASH_ATTR   ssd1306_command(unsigned char c){



        i2c_master_start();                  // (i2c_restart();)
        i2c_master_writeByte(SSD1306_WRITE);           // адрес (i2c_write(LCD_ADDR);)
	i2c_master_checkAck();
        i2c_master_writeByte(0x00); // set pointer to CRA
	i2c_master_checkAck();

        i2c_master_writeByte(c); // set pointer to CRA
	i2c_master_checkAck();
	i2c_master_stop();
}

void ICACHE_FLASH_ATTR   ssd1306_init(void){
        i2c_master_start();                  // (i2c_restart();)
        i2c_master_writeByte(SSD1306_WRITE);           // адрес (i2c_write(LCD_ADDR);)
	i2c_master_checkAck();
        i2c_master_writeByte(0x00);
	i2c_master_checkAck();
        i2c_master_writeByte(0x10);
	i2c_master_checkAck();
        i2c_master_writeByte(0x40);
	i2c_master_checkAck();
        i2c_master_writeByte(0x81);
	i2c_master_checkAck();
        i2c_master_writeByte(0xCF);
	i2c_master_checkAck();

        i2c_master_writeByte(0xA0);
	i2c_master_checkAck();
        i2c_master_writeByte(0xA6);
	i2c_master_checkAck();
        i2c_master_writeByte(0xA8);
	i2c_master_checkAck();
        i2c_master_writeByte(0x3F);
	i2c_master_checkAck();

        i2c_master_writeByte(0xD3);
	i2c_master_checkAck();
        i2c_master_writeByte(0x00);
	i2c_master_checkAck();
        i2c_master_writeByte(0xD5);
	i2c_master_checkAck();
        i2c_master_writeByte(0x80);
	i2c_master_checkAck();


        i2c_master_writeByte(0xD9);
	i2c_master_checkAck();
        i2c_master_writeByte(0xF1);
	i2c_master_checkAck();
        i2c_master_writeByte(0xDA);
	i2c_master_checkAck();
        i2c_master_writeByte(0x12);
	i2c_master_checkAck();

        i2c_master_writeByte(0xDB);
	i2c_master_checkAck();
        i2c_master_writeByte(0x30);
	i2c_master_checkAck();
        i2c_master_writeByte(0x8D);
	i2c_master_checkAck();
        i2c_master_writeByte(0x14);
	i2c_master_checkAck();

        i2c_master_writeByte(0x20);
	i2c_master_checkAck();
        i2c_master_writeByte(0x00);
	i2c_master_checkAck();
        i2c_master_writeByte(0xAF);
	i2c_master_checkAck();

	i2c_master_stop();
}


void ICACHE_FLASH_ATTR  ssd1306_gotoxy(unsigned char x, unsigned char y)
{
        SSD1306_row_cnt=y;
        SSD1306_col_cnt=x;

        i2c_master_start();                  // (i2c_restart();)
        i2c_master_writeByte(SSD1306_WRITE);           // адрес (i2c_write(LCD_ADDR);)
	i2c_master_checkAck();
        i2c_master_writeByte(0x00);
	i2c_master_checkAck();
        i2c_master_writeByte(0xb0+7-y);
	i2c_master_checkAck();
        i2c_master_writeByte(((x & 0xf0) >> 4) | 0x10);
	i2c_master_checkAck();
        i2c_master_writeByte((x & 0x0f) | 0x01);
	i2c_master_checkAck();
	i2c_master_stop();
}

void ICACHE_FLASH_ATTR  ssd1306_fillscreen(unsigned char fill)
{
uint8_t m,n;
	for (m = 0; m < 8; m++)
	{
		ssd1306_command(0xb0 + m);	// page0 - page1
		ssd1306_command(0x00);		// low column start address
		ssd1306_command(0x10);		// high column start address

	        i2c_master_start();                  // (i2c_restart();)
    		i2c_master_writeByte(SSD1306_WRITE);           // адрес (i2c_write(LCD_ADDR);)
		i2c_master_checkAck();
	        i2c_master_writeByte(0x40);
		i2c_master_checkAck();


		for (n = 0; n < 128; n++)
		{
	          i2c_master_writeByte(fill);
		  i2c_master_checkAck();
		}
	i2c_master_stop();
	}
}

void ICACHE_FLASH_ATTR  ssd1306_clear(void)
{
  ssd1306_fillscreen(0);
}


void ICACHE_FLASH_ATTR ssd1306_putchar(unsigned char b)
{
  unsigned int j;
//  unsigned int idx;

  if (b == '\n') 
  {
    SSD1306_row_cnt++;
    if (SSD1306_row_cnt>7) SSD1306_row_cnt=0;
    ssd1306_gotoxy(0,SSD1306_row_cnt);
    SSD1306_col_cnt=0;
  }
  else
  {
  SSD1306_col_cnt++;
  if (SSD1306_col_cnt>127)
  {
    SSD1306_row_cnt++;
    if (SSD1306_row_cnt>7) SSD1306_row_cnt=0;
    SSD1306_col_cnt=0;
  }

    i2c_master_start();                  // (i2c_restart();)
    i2c_master_writeByte(SSD1306_WRITE);           // адрес (i2c_write(LCD_ADDR);)
    i2c_master_checkAck();
    i2c_master_writeByte(0x40);
    i2c_master_checkAck();

  for (j=0;j<6;j++)
  {
      i2c_master_writeByte(font_6x8_Data[b*6+j]);
      i2c_master_checkAck();
  }
    i2c_master_stop();
  }
}



void ICACHE_FLASH_ATTR ssd1306_print (unsigned char *characters) {
unsigned    int i;

        i2c_master_start();                  // (i2c_restart();)
        i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
	i2c_master_checkAck();
        i2c_master_writeByte(0b01000000);         //0x40 control byte (i2c_write(0b01000000);)
	i2c_master_checkAck();


  while (*characters) {
  uint8 c=*characters;
//    pcf8535_putchar(*characters++);
	*characters++;
	if (c!='\n')
	{
          for(i = 0; i < 6; i++)
          {
		i2c_master_writeByte(font_6x8_Data[c*6+i]);
		i2c_master_checkAck();
		SSD1306_col_cnt++;
          }
	}
	else
	{
          SSD1306_row_cnt++;
	{
          for(i = SSD1306_col_cnt; i < 128; i++)
            {
		i2c_master_writeByte(0x00);
		i2c_master_checkAck();
            }

	  }
//	pcf8535_gotoxy(0,pcf8535_ycnt);
	}

  }
        i2c_master_stop();
}
