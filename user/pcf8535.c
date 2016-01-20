
#include <stdio.h>
#include "osapi.h"
#include "os_type.h"
#include "font_6x8.h"
#include "pcf8535.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1



void ICACHE_FLASH_ATTR pcf8535_putchar(char c)
{
unsigned    int i;

if (c=='\n')
{
	pcf8535_ycnt--;
	if (pcf8535_xcnt<133)
	{
        i2c_master_start();                  // (i2c_restart();)
        i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
	i2c_master_checkAck();
        i2c_master_writeByte(0b01000000);         //0x40 control byte (i2c_write(0b01000000);)
	i2c_master_checkAck();
    
//        pic = (const char *) qweqwe;   
        for(i = pcf8535_xcnt; i < 133; i++)
            {
		if (pcf8535_xcnt>127) // Вывод столбцов со 128 по 132
		{
	            i2c_master_stop();
		    i2c_master_start();
	            i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
		    i2c_master_checkAck();
	            i2c_master_writeByte(0b00000000);         //0x00 control byte (i2c_write(0b00000000);)
		    i2c_master_checkAck();
    		    i2c_master_writeByte(0x01);               //0x01на основн стр (i2c_write(0x01);)
		    i2c_master_checkAck();
		    i2c_master_writeByte(0x24);               //0x24 Set XM0 bit
		    i2c_master_checkAck();
	            i2c_master_writeByte(0x80|(pcf8535_xcnt-128));         //0x80 X = 0; (i2c_write(0b10000000);)
		    i2c_master_checkAck();
	            i2c_master_stop();
	            i2c_master_start();                  // (i2c_restart();)
	            i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
			i2c_master_checkAck();
	            i2c_master_writeByte(0b01000000);         //0x40 control byte (i2c_write(0b01000000);)
			i2c_master_checkAck();
		}
//		i2c_master_writeByte(*pic++);
		i2c_master_writeByte(0x00);
		i2c_master_checkAck();
            //WriteI2C(qwe);

            }
        i2c_master_stop();
	}
	pcf8535_gotoxy(0,pcf8535_ycnt);
}
else
{
        i2c_master_start();                  // (i2c_restart();)
        i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
	i2c_master_checkAck();
        i2c_master_writeByte(0b01000000);         //0x40 control byte (i2c_write(0b01000000);)
	i2c_master_checkAck();
    
//        pic = (const char *) qweqwe;   
        for(i = 0; i < 6; i++)
            {
		if (pcf8535_xcnt>127) // Вывод столбцов со 128 по 132
		{
	            i2c_master_stop();
		    i2c_master_start();
	            i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
		    i2c_master_checkAck();
	            i2c_master_writeByte(0b00000000);         //0x00 control byte (i2c_write(0b00000000);)
		    i2c_master_checkAck();
    		    i2c_master_writeByte(0x01);               //0x01на основн стр (i2c_write(0x01);)
		    i2c_master_checkAck();
		    i2c_master_writeByte(0x24);               //0x24 Set XM0 bit
		    i2c_master_checkAck();
	            i2c_master_writeByte(0x80|(pcf8535_xcnt-128));         //0x80 X = 0; (i2c_write(0b10000000);)
		    i2c_master_checkAck();
	            i2c_master_stop();
	            i2c_master_start();                  // (i2c_restart();)
	            i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
			i2c_master_checkAck();
	            i2c_master_writeByte(0b01000000);         //0x40 control byte (i2c_write(0b01000000);)
			i2c_master_checkAck();
		}
//		i2c_master_writeByte(*pic++);
		i2c_master_writeByte(font_6x8_Data[c*6+i]);
		i2c_master_checkAck();
            //WriteI2C(qwe);
		pcf8535_xcnt++;
            }
        i2c_master_stop();
}

}

void ICACHE_FLASH_ATTR pcf8535_print (char *characters) {
unsigned    int i;
  while (*characters) {
uint8 c=*characters;
//    pcf8535_putchar(*characters++);
	*characters++;
        i2c_master_start();                  // (i2c_restart();)
        i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
	i2c_master_checkAck();
        i2c_master_writeByte(0b01000000);         //0x40 control byte (i2c_write(0b01000000);)
	i2c_master_checkAck();


if (c=='\n')
{
	pcf8535_ycnt--;
	if (pcf8535_xcnt<133)
	{
//        pic = (const char *) qweqwe;   
        for(i = pcf8535_xcnt; i < 133; i++)
            {
		if (pcf8535_xcnt>127) // Вывод столбцов со 128 по 132
		{
	            i2c_master_stop();
		    i2c_master_start();
	            i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
		    i2c_master_checkAck();
	            i2c_master_writeByte(0b00000000);         //0x00 control byte (i2c_write(0b00000000);)
		    i2c_master_checkAck();
    		    i2c_master_writeByte(0x01);               //0x01на основн стр (i2c_write(0x01);)
		    i2c_master_checkAck();
		    i2c_master_writeByte(0x24);               //0x24 Set XM0 bit
		    i2c_master_checkAck();
	            i2c_master_writeByte(0x80|(pcf8535_xcnt-128));         //0x80 X = 0; (i2c_write(0b10000000);)
		    i2c_master_checkAck();
	            i2c_master_stop();
	            i2c_master_start();                  // (i2c_restart();)
	            i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
			i2c_master_checkAck();
	            i2c_master_writeByte(0b01000000);         //0x40 control byte (i2c_write(0b01000000);)
			i2c_master_checkAck();
		}
//		i2c_master_writeByte(*pic++);
		i2c_master_writeByte(0x00);
		i2c_master_checkAck();
            //WriteI2C(qwe);

            }

	}
	pcf8535_gotoxy(0,pcf8535_ycnt);
}
else
{
        for(i = 0; i < 6; i++)
            {
		if (pcf8535_xcnt>127) // Вывод столбцов со 128 по 132
		{
	            i2c_master_stop();
		    i2c_master_start();
	            i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
		    i2c_master_checkAck();
	            i2c_master_writeByte(0b00000000);         //0x00 control byte (i2c_write(0b00000000);)
		    i2c_master_checkAck();
    		    i2c_master_writeByte(0x01);               //0x01на основн стр (i2c_write(0x01);)
		    i2c_master_checkAck();
		    i2c_master_writeByte(0x24);               //0x24 Set XM0 bit
		    i2c_master_checkAck();
	            i2c_master_writeByte(0x80|(pcf8535_xcnt-128));         //0x80 X = 0; (i2c_write(0b10000000);)
		    i2c_master_checkAck();
	            i2c_master_stop();
	            i2c_master_start();                  // (i2c_restart();)
	            i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
			i2c_master_checkAck();
	            i2c_master_writeByte(0b01000000);         //0x40 control byte (i2c_write(0b01000000);)
			i2c_master_checkAck();
		}
//		i2c_master_writeByte(*pic++);
		i2c_master_writeByte(font_6x8_Data[c*6+i]);
		i2c_master_checkAck();
            //WriteI2C(qwe);
		pcf8535_xcnt++;
            }
//        i2c_master_stop();
}
  }
        i2c_master_stop();
}

void ICACHE_FLASH_ATTR pcf8535_gotoxy(char x,char y)
{
	pcf8535_ycnt=y;
        i2c_master_start();
        i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
	i2c_master_checkAck();
        i2c_master_writeByte(0b00000000);         //0x00 control byte (i2c_write(0b00000000);)
	i2c_master_checkAck();
        i2c_master_writeByte(0x01);               //0x01на основн стр (i2c_write(0x01);)
	i2c_master_checkAck();
        i2c_master_writeByte(0x40|(y&0x07));         //0x40 Y = 0; (i2c_write(0b01000000);)
	i2c_master_checkAck();
	if (x>127)
	{
	  i2c_master_writeByte(0x24);               //0x24 Set XM0 bit
	  i2c_master_checkAck();
          i2c_master_writeByte(0x80|(x-127));         //0x80 X = 0; (i2c_write(0b10000000);)
	  i2c_master_checkAck();
	}
	else
	{
	  i2c_master_writeByte(0x20);               //0x24 Set XM0 bit
	  i2c_master_checkAck();

          i2c_master_writeByte(0x80|x);         //0x80 X = 0; (i2c_write(0b10000000);)
	  i2c_master_checkAck();
	}

	i2c_master_stop();
	pcf8535_xcnt=x;
//	os_delay_us(100);
}



void ICACHE_FLASH_ATTR pcf8535_init(void)
{
	i2c_master_start();
        i2c_master_writeByte(0x78);        // адрес
	i2c_master_checkAck();
        i2c_master_writeByte(0b00000000);      // control byte
	i2c_master_checkAck();
        i2c_master_writeByte(0x01);            // на основн стр
	i2c_master_checkAck();
        i2c_master_writeByte(0b00001110);      // на стр 110
	i2c_master_checkAck();
        i2c_master_writeByte(0b00010010);      // BIAS = 1/9
	i2c_master_checkAck();
        i2c_master_writeByte(0b10000100);      // MUX = 1/65
	i2c_master_checkAck();
        i2c_master_writeByte(0b00000100);      // D = 0, E = 0;
	i2c_master_checkAck();
        i2c_master_writeByte(0b00100100);      // IB = 1
	i2c_master_checkAck();
//      WriteI2C(0b00100000);      // IB = 0
        i2c_master_writeByte(0b00001100);      // MX = 0, MY = 1;
	i2c_master_checkAck();
//      WriteI2C(0b00001100);      // MX = 1, MY = 0;

        i2c_master_writeByte(0x01);            // на основн стр
	i2c_master_checkAck();
        i2c_master_writeByte(0b00010000);      // PD = 0, V = 0; горизонтальная адресация
	i2c_master_checkAck();
//      WriteI2C(0b00010010);      // PD = 0, V = 0; вертикальная

        i2c_master_writeByte(0b00001011);      // на стр 011
	i2c_master_checkAck();
        i2c_master_writeByte(0b01011000);      // TRS = 1, BRS = 1;
	i2c_master_checkAck();
        i2c_master_writeByte(0b00000101);      // DM = 1
	i2c_master_checkAck();

//        delay_ms(1);
//	os_delay_us(100);

        i2c_master_writeByte(0x01);            // на основн стр
	i2c_master_checkAck();
        i2c_master_writeByte(0b00001101);      // на стр 101
	i2c_master_checkAck();

        i2c_master_writeByte(0b00001000);      // S[1;0] = 00, mul factor = 2
	i2c_master_checkAck();
//      WriteI2C(0b00000110);      // PRS = 1, Vlcd programing range high
        i2c_master_writeByte(0b00000100);      // PRS = 0, Vlcd programing range low
	i2c_master_checkAck();

//        i2c_master_writeByte(61 | 0x80);       // установка напряжения
        i2c_master_writeByte(51 | 0x80);       // установка напряжения
	i2c_master_checkAck();
        i2c_master_writeByte(0b00000101);      // включение генератора
	i2c_master_checkAck();

//        delay_ms(1);                // should be более 20 мкс
	os_delay_us(100);
        i2c_master_writeByte(0b00001001);      // S[1;0] = 01, mul factor = 3
	i2c_master_checkAck();
//        delay_ms(1);
//	os_delay_us(100);
        i2c_master_writeByte(0b00001010);      // S[1;0] = 10, mul factor = 4
	i2c_master_checkAck();
//        delay_ms(1);
//	os_delay_us(100);
        i2c_master_writeByte(0b00001011);      // S[1;0] = 11, mul factor = 5
	i2c_master_checkAck();

        i2c_master_writeByte(0x01);            // на основн стр
	i2c_master_checkAck();
        i2c_master_writeByte(0b00001011);      // на стр 011
	i2c_master_checkAck();

        i2c_master_writeByte(0b00000100);      // DM = 0
	i2c_master_checkAck();

        i2c_master_writeByte(0x01);            // на основн стр
	i2c_master_checkAck();
        i2c_master_writeByte(0b00001110);      // на стр 110
	i2c_master_checkAck();
        i2c_master_writeByte(0b00000110);      // D = 1, E = 0; normal
	i2c_master_checkAck();

        i2c_master_stop();
//	os_delay_us(100);
}

void ICACHE_FLASH_ATTR  pcf8535_fillscreen(unsigned char fill)
{
unsigned    int i;
/*  -------------------------------------------------------------------------
 *  очистка ОЗУ драйвера
 */
        i2c_master_start();
    
        i2c_master_writeByte(0x78);           // адрес (i2c_write(LCD_ADDR);)
	i2c_master_checkAck();
        i2c_master_writeByte(0b00000000);         //0x00 control byte (i2c_write(0b00000000);)
	i2c_master_checkAck();
        i2c_master_writeByte(0x01);               //0x01на основн стр (i2c_write(0x01);)
	i2c_master_checkAck();
        i2c_master_writeByte(0b01000000);         //0x40 Y = 0; (i2c_write(0b01000000);)
	i2c_master_checkAck();
        i2c_master_writeByte(0b10000000);         //0x80 X = 0; (i2c_write(0b10000000);)
	i2c_master_checkAck();
	i2c_master_stop();



        i2c_master_start();

        i2c_master_writeByte(0x78);        // адрес
	i2c_master_checkAck();
        i2c_master_writeByte(0b01000000);      // control byte
	i2c_master_checkAck();
        for(i = 0; i < 1064; i++)
	{
            i2c_master_writeByte(fill);
	    i2c_master_checkAck();
	}
        i2c_master_stop();
//	os_delay_us(100);
}

void ICACHE_FLASH_ATTR  pcf8535_clear(void)
{
  pcf8535_fillscreen(0);
}
