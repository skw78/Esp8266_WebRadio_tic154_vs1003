
LOCAL uint8 pcf8535_xcnt=0, pcf8535_ycnt=7;
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
void ICACHE_FLASH_ATTR pcf8535_putchar(char c);
void ICACHE_FLASH_ATTR pcf8535_print (char *characters);
void ICACHE_FLASH_ATTR pcf8535_gotoxy(char x,char y);
void ICACHE_FLASH_ATTR pcf8535_init(void);
void ICACHE_FLASH_ATTR pcf8535_clear(void);

