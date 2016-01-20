#define lcd_init ssd1306_init
#define lcd_print ssd1306_print
#define lcd_clear ssd1306_clear
#define lcd_gotoxy ssd1306_gotoxy
#define lcd_fillscreen ssd1306_fillscreen

//LOCAL uint8 pcf8535_xcnt=0, pcf8535_ycnt=7;
/*
static const char* wdays[]=
{
  "Su",
  "Mo",
  "Tu",
  "We",
  "Th",
  "Fr",
  "Sa"

};
*/
/*
static const char* wdays[]=
{
  "Âñ",
  "Ïí",
  "Âò",
  "Ñð",
  "×ò",
  "Ïò",
  "Ñá"
};
*/
void ICACHE_FLASH_ATTR ssd1306_putchar(unsigned char b);
void ICACHE_FLASH_ATTR ssd1306_print (unsigned char *characters);
void ICACHE_FLASH_ATTR ssd1306_gotoxy(unsigned char x, unsigned char y);
void ICACHE_FLASH_ATTR ssd1306_init(void);
void ICACHE_FLASH_ATTR  ssd1306_fillscreen(unsigned char fill);
void ICACHE_FLASH_ATTR ssd1306_clear(void);

