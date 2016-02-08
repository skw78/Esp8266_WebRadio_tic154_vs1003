/*
 *  Example of working sensor DHT22 (temperature and humidity) and send data on the service thingspeak.com
 *  https://thingspeak.com
 *
 *  For a single device, connect as follows:
 *  DHT22 1 (Vcc) to Vcc (3.3 Volts)
 *  DHT22 2 (DATA_OUT) to ESP Pin GPIO2
 *  DHT22 3 (NC)
 *  DHT22 4 (GND) to GND
 *
 *  Between Vcc and DATA_OUT need to connect a pull-up resistor of 10 kOh.
 *
 *  (c) 2015 by Mikhail Grigorev <sleuthhound@gmail.com>
 *
 */

#include <user_interface.h>
#include <osapi.h>
#include <c_types.h>
#include <mem.h>
#include <os_type.h>
#include "httpclient.h"
#include "driver/uart.h"
#include "user_config.h"
#include "ssd1306.h"
//#include "pcf8535.h"

#include "gpio.h"
#include "driver/gpio16.h"
#include "eagle_soc.h"
#include "wifi_api.c"
#include <time.h>
#include "u_time.h"
#include "driver/vs1003.h"

#define volstep 5
#define kb_nokey 0x00
#define kb_up 0x01
#define kb_down 0x02
#define kb_left 0x04
#define kb_right 0x08
#define kb_menu 0x10
#define kb_master 0x20
#define kb_exit 0x40


static volatile os_timer_t WiFiLinker;
static volatile os_timer_t gpio_timer;
static volatile os_timer_t mp3_timer;

static void ICACHE_FLASH_ATTR wifi_check_ip(os_event_t *events);

uint8 leftvol=50, rightvol=50;
uint8 bass=0;
int8 treable=0;
uint8 bass_lim=0;
uint8 treable_lim=0;
bool mute=false;

uint8 url_idx=0;
uint16 zerobuf_cnt=0;
uint8 menu=0;
uint8 submenu=0;


//#define fifo_size 16384
#define fifo_size 32768
uint8 fifobuf[fifo_size];

uint32 old_fifo_cnt=0;

uint16	readCount=0;
uint16	writeCount=0;
uint8 change_lcd=0xff;
//char lcdbuf[8][25];
uint8 ds=0;

static const uint16 fifo_mask = fifo_size-1;
//static void ICACHE_FLASH_ATTR loop(os_event_t *events);

//Main code function




#define user_procTaskPrio        1
#define user_procTaskQueueLen    1

os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void ICACHE_FLASH_ATTR ICACHE_FLASH_ATTR mp3_task(os_event_t *events);



LOCAL void  ICACHE_FLASH_ATTR   radio_http_callback(char * response, int http_status, char * full_response)
{
	os_printf ("\n\r--------------disconnect -----------\n\r");
	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);

}


char ICACHE_FLASH_ATTR mp3_callback(char * buf, unsigned short len) {

	uint16 i,j,k;
	uint16 cnt=0;
//	os_printf("RECV l=%d\r\n", len);
	system_update_cpu_freq(SYS_CPU_160MHZ);
//	ETS_GPIO_INTR_DISABLE();
	if (((buf[0] == 'I') && (buf[1] == 'C') && (buf[2] == 'Y'))
			|| ((buf[0] == 'H') && (buf[1] == 'T') && (buf[2] == 'T') && (buf[3] == 'P'))) {

//		os_printf("%s", buf);
		system_update_cpu_freq(SYS_CPU_80MHZ);
		return 0;
	}

	i=0;
	zerobuf_cnt=0;
	while (i<len)
	{
	  fifobuf[writeCount++ & fifo_mask]=buf[i++];
	}


        if (((writeCount-readCount)&fifo_mask)>=(fifo_size-2048))
	{
//          gpio_pin_intr_state_set(GPIO_ID_PIN(4), GPIO_PIN_INTR_HILEVEL);
//	  ETS_GPIO_INTR_ENABLE();
	    system_os_post(user_procTaskPrio, 0, 0 );
	}

	system_update_cpu_freq(SYS_CPU_80MHZ);
	return 0;
}

//static void  wifi_check_ip(void *arg)
static void ICACHE_FLASH_ATTR wifi_check_ip(os_event_t *events)
{
	static struct ip_info ipConfig;
	os_timer_disarm(&WiFiLinker);

	system_update_cpu_freq(SYS_CPU_160MHZ);
	switch(wifi_station_get_connect_status())
	{
		case STATION_GOT_IP:
			wifi_get_ip_info(STATION_IF, &ipConfig);
						if(ipConfig.ip.addr != 0) {
							    http_get(radio[url_idx],"", radio_http_callback);
							    os_printf("WiFi connected, ip.addr is NOT null\r\n");
							return;
						} else {
							os_printf("WiFi connected, ip.addr is null\r\n");
						}
			break;
		case STATION_WRONG_PASSWORD:

			os_printf("WiFi connecting error, wrong password\r\n");
			break;
		case STATION_NO_AP_FOUND:

			os_printf("WiFi connecting error, ap not found\r\n");
			break;
		case STATION_CONNECT_FAIL:

			os_printf("WiFi connecting fail\r\n");
			break;
		default:

			os_printf("WiFi connecting...\r\n");
	}
	system_update_cpu_freq(SYS_CPU_80MHZ);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);
}


void  ICACHE_FLASH_ATTR  user_rf_pre_init(void)
{
}


void  ICACHE_FLASH_ATTR  gpio_timerfunc(void *arg)
{
char b;
char strkey[4];
char key=0;
int adc=0;
char outstr[40];
int8  whole;
int8  decimal;
uint8 i;
struct tmElements ts;
time_t cs=0;
    uint32 fifo_cnt=0;

    system_update_cpu_freq(SYS_CPU_160MHZ);
//    if (((writeCount-readCount)&fifo_mask)==0)
//    {
	if (mp3_conn!=NULL)
	{
	  if (zerobuf_cnt>=20)
	  {
            if (mp3_conn!=NULL) 
	    {
		espconn_disconnect(mp3_conn);
		writeCount=0; readCount=0;
	    }
	    zerobuf_cnt=0;
	  }
	  else
	  {
	    zerobuf_cnt++;
	  }
	}
//    }
//    else
//    {
//	zerobuf_cnt=0;
//    }


    // Структура с информацией о полученном, ip адресе клиента STA, маске подсети, шлюзе.
    struct ip_info ipConfig;
    // Получаем данные о сетевых настройках
    wifi_get_ip_info(STATION_IF, &ipConfig);
    // Проверяем статус wi-fi соединения и факт получения ip адреса
    if ((wifi_station_get_connect_status() == STATION_GOT_IP && ipConfig.ip.addr != 0) || cs != 0)
    {
	cs = sntp_get_current_timestamp();
	timet_to_tm(cs, &ts);

//	os_sprintf(outstr,"%02u:%02u:%02u %s %02u.%02u.%4u\n", ts.Hour, ts.Minute, ts.Second, wdays[ts.Wday-1], ts.Day, ts.Month, 1970+ts.Year);
	os_sprintf(outstr,"%02u:%02u:%02u %s %02u.%02u.%2u\n", ts.Hour, ts.Minute, ts.Second, wdays[ts.Wday-1], ts.Day, ts.Month, (1970+ts.Year)%100);
//	os_sprintf(outstr,"%02u:%02u:%02u %02u.%02u.%02u", ts.Hour, ts.Minute, ts.Second, ts.Day, ts.Month, (70+ts.Year)%100);
        lcd_gotoxy(0,7);
        lcd_print(outstr);

//	os_sprintf(outstr,"%02u.%02u.%02u", ts.Day, ts.Month, (70+ts.Year)%100);
    }

    os_sprintf(outstr,"");
    fifo_cnt=((writeCount-readCount)&fifo_mask)*100/fifo_size;
    if (fifo_cnt!=old_fifo_cnt)
    {
        for (i=0;i<fifo_cnt/5;i++)  os_sprintf(outstr,"%s>",outstr);
        os_sprintf(outstr,"%s\n",outstr);
        lcd_gotoxy(0,5);
        lcd_print(outstr);
	fifo_cnt=old_fifo_cnt;
    }

    adc = system_adc_read(); // Чтение АЦП
    os_printf("ADC: %d\n",adc);
    if (adc<10)
    {
	key=kb_up;
    }
    else if ((adc<210)&&(adc>190))
    {
	key=kb_down;
    }
    else if ((adc<380)&&(adc>360))
    {
	key=kb_left;
    }
    else if ((adc<550)&&(adc>530))
    {
	key=kb_right;
    }
    else if ((adc<690)&&(adc>670))
    {
	key=kb_master;
    }
    else if ((adc<820)&&(adc>800))
    {
	key=kb_exit;
    }
    else if ((adc<940)&&(adc>920))
    {
	key=kb_menu;
    }
    else
    {
	key=kb_nokey;
    }


// Обработка нажатия клавиш
// ETS_GPIO_INTR_DISABLE();

if (key==kb_master)
{
  mute=~mute;
  if (mute)
  {
    Mp3SelectControl();
    Mp3SetVolume(254,254);
    Mp3DeselectControl();
    change_lcd|=0x02;
  }
  else
  {
    leftvol=rightvol;
    Mp3SelectControl();
    Mp3SetVolume(100-leftvol,100-rightvol);
    Mp3DeselectControl();
    change_lcd|=0x02;
  }
}


switch (menu)
{
case 0:
    {
        switch (submenu)
	{
	    case 0:  // Change station
	    {
	      switch (key)
	      {
		case kb_left:
		{
		  if (url_idx>0)
		  {
		    url_idx--;
        	    if (mp3_conn!=NULL) 
		    {
			espconn_disconnect(mp3_conn);
			writeCount=0; readCount=0;
		    }
		  }
		  change_lcd|=0x01;
		  break;
		}
		case kb_right:
		{
		  if (url_idx<max_url_idx)
		  {
		    url_idx++;
        	    if (mp3_conn!=NULL) 
		    {
			espconn_disconnect(mp3_conn);
			writeCount=0; readCount=0;
		    }

		  }
		  change_lcd|=0x01;
		  break;
		}
	      }
	      break;
	    }
	    case 1: // Volume
	    {
	      switch (key)
	      {
		case kb_right:
		{
		  mute=false;
		  if (rightvol+volstep<100)
		  {
		    rightvol+=volstep;
		  }
		  else
		  {
		    rightvol=100;
		  }

		  leftvol=rightvol;
		  Mp3SelectControl();
		  Mp3SetVolume(100-leftvol,100-rightvol);
		  Mp3DeselectControl();
		  change_lcd|=0x02;
		  break;
		}
		case kb_left:
		{
		  mute=false;
		  if (rightvol>volstep)
		  {
		      rightvol-=volstep;
		  }
		  else
		  {
		    rightvol=0;
		  }

		  leftvol=rightvol;
		  Mp3SelectControl();
		  Mp3SetVolume(100-leftvol,100-rightvol);
		  Mp3DeselectControl();
		  change_lcd|=0x02;
		  break;

		}
	      }
	      break;
	    }

	    case 2: // Treable
	    {
	      switch (key)
	      {
		case kb_left:
		{
		  if (treable>-7)
		  {
		    treable--;
		    Mp3SelectControl();
		    Mp3WriteRegister(SPI_BASS, (treable<<4)|treable_lim, (bass<<4)|bass_lim);
		    Mp3DeselectControl();
		    change_lcd|=0x04;
		  }
		  break;
		}

		case kb_right:
		{
		  if (treable<7)
		  {
		    treable++;
		    Mp3SelectControl();
		    Mp3WriteRegister(SPI_BASS, (treable<<4)|treable_lim, (bass<<4)|bass_lim);
		    Mp3DeselectControl();
		    change_lcd|=0x04;
		  }
		  break;
		}
	      }
	      break;
	    }

	    case 3: // Treable Freq Limit
	    {
	      switch (key)
	      {
		case kb_left:
		{
		  if (treable_lim>0)
		  {
		    treable_lim--;
		    Mp3SelectControl();
		    Mp3WriteRegister(SPI_BASS, (treable<<4)|treable_lim, (bass<<4)|bass_lim);
		    Mp3DeselectControl();
		    change_lcd|=0x08;
		  }
		  break;
		}

		case kb_right:
		{
		  if (treable_lim<15)
		  {
		    treable_lim++;
		    Mp3SelectControl();
		    Mp3WriteRegister(SPI_BASS, (treable<<4)|treable_lim, (bass<<4)|bass_lim);
		    Mp3DeselectControl();
		    change_lcd|=0x08;
		  }
		  break;
		}
	      }
	      break;
	    }


	    case 4: // Bass
	    {
	      switch (key)
	      {
		case kb_left:
		{
		  if (bass>0)
		  {
		    bass--;
		    Mp3SelectControl();
		    Mp3WriteRegister(SPI_BASS, (treable<<4)|treable_lim, (bass<<4)|bass_lim);
		    Mp3DeselectControl();
		    change_lcd|=0x10;
		  }
		  break;
		}

		case kb_right:
		{
		  if (bass<15)
		  {
		    bass++;
		    Mp3SelectControl();
		    Mp3WriteRegister(SPI_BASS, (treable<<4)|treable_lim, (bass<<4)|bass_lim);
		    Mp3DeselectControl();
		    change_lcd|=0x10;
		  }
		  break;
		}
	      }
	      break;
	    }

	    case 5: // Bass Freq Limit
	    {
	      switch (key)
	      {
		case kb_left:
		{
		  if (bass_lim>0)
		  {
		    bass_lim--;
		    Mp3SelectControl();
		    Mp3WriteRegister(SPI_BASS, (treable<<4)|treable_lim, (bass<<4)|bass_lim);
		    Mp3DeselectControl();
		    change_lcd|=0x20;
		  }
		  break;
		}

		case kb_right:
		{
		  if (bass_lim<15)
		  {
		    bass_lim++;
		    Mp3SelectControl();
		    Mp3WriteRegister(SPI_BASS, (treable<<4)|treable_lim, (bass<<4)|bass_lim);
		    Mp3DeselectControl();
		    change_lcd|=0x20;
		  }
		  break;
		}
	      }
	      break;
	    }



	    default:
	    {
	      submenu=0;
	    }
	}

	switch (key)
	{
	  case kb_up:
	  {
	    if (submenu>0)
	    {
	      submenu--;
	    }
	    else
	    {
	      submenu=0x40;
	    }

	    change_lcd|=0xff;
	    break;
	  }
	  case kb_down:
	  {
	    if (submenu<5)
	    {
	      submenu++;
	    }
	    else
	    {
	      submenu=0;
	    }
	    change_lcd|=0xff;
	    break;
	  }

	}

    break;
    }
}

//    ETS_GPIO_INTR_ENABLE();

if (change_lcd)
{
//    lcd_clear();
    if (change_lcd&0x01)
    {

	lcd_gotoxy(0,6);
	os_sprintf(outstr,"STATION: ");
	lcd_print(outstr);

	if (submenu==0)
	{
	  os_sprintf(outstr,"<%10s>", radio_names[url_idx]);
	}
	else
	{
	  os_sprintf(outstr,"%12s", radio_names[url_idx]);
	}
	lcd_print(outstr);
    }
/*
    cnt=((writeCount-readCount)&fifo_mask)*100/fifo_size;
    for (i=0;i<cnt/5;i++)  os_printf(">");
    os_printf("\r\n");


    os_printf("DREQ %5d (%2d), %5d, %5d\r\n", (writeCount-readCount)&fifo_mask, cnt, c, zerobuf_cnt);
*/
    if (change_lcd&0x02)
    {
        lcd_gotoxy(0,4);
        os_sprintf(outstr,"VOLUME: ");
        lcd_print(outstr);
	if (mute)
	{
    	    if (submenu==1)
	    {
             os_sprintf(outstr,"<Mute>\n");
	    }
	    else
	    {
    	     os_sprintf(outstr,"Mute\n");
	    }
	}
	else
	{
    	    if (submenu==1)
	    {
             os_sprintf(outstr,"<%3d%%>\n", rightvol);
	    }
	    else
	    {
    	     os_sprintf(outstr,"%3d%%\n", rightvol);
	    }
	}
	lcd_print(outstr);
    }

    if (change_lcd&0x04)
    {
        lcd_gotoxy(0,3);
        os_sprintf(outstr,"TREABLE: ");
        lcd_print(outstr);

        whole    = (float)treable*1.5;
        decimal = (((float)treable*1.5) - whole ) * 10; // only deci ..


        if (submenu==2)
        {
          os_sprintf(outstr,"<%+02d.%01idB>\n", whole, abs(decimal));
        }
        else
        {
          os_sprintf(outstr,"%+02d.%01idB\n", whole, abs(decimal));
        }
        lcd_print(outstr);
    }

    if (change_lcd&0x08)
    {
        lcd_gotoxy(0,2);
        os_sprintf(outstr,"TREABLE_LIM: ");
        lcd_print(outstr);
        if (submenu==3)
        {
          os_sprintf(outstr,"<%2dkHz>\n", treable_lim);
        }
        else
        {
          os_sprintf(outstr,"%2dkHz\n", treable_lim);
        }
        lcd_print(outstr);
    }

    if (change_lcd&0x10)
    {
        lcd_gotoxy(0,1);
        os_sprintf(outstr,"BASS: ");
        lcd_print(outstr);

        if (submenu==4)
        {
          os_sprintf(outstr,"<%+2ddB>\n", bass);
        }
        else
        {
          os_sprintf(outstr,"%+2ddB\n", bass);
        }
        lcd_print(outstr);
    }

    if (change_lcd&0x20)
    {
        lcd_gotoxy(0,0);
        os_sprintf(outstr,"BASS_LIM: ");
        lcd_print(outstr);
        if (submenu==5)
        {
          os_sprintf(outstr,"<%2dHz>\n", bass_lim*10);
        }
        else
        {
          os_sprintf(outstr,"%2dHz\n", bass_lim*10);
        }
	lcd_print(outstr);
    }

/*
    if (change_lcd&0x40)
    {
        os_sprintf(outstr,"MODE: %2d %2d\n", menu, submenu);
        lcd_gotoxy(0,1);
        lcd_print(outstr);
    }
*/
/*
    if (change_lcd&0x80)
    {
        lcd_gotoxy(0,0);
        os_sprintf(outstr,"Skw WiFi Radio v.0.0.1");
        lcd_print(outstr);
    }
*/
    change_lcd=0;
}

    system_update_cpu_freq(SYS_CPU_80MHZ);
}


static void ICACHE_FLASH_ATTR ICACHE_FLASH_ATTR
mp3_task(os_event_t *events)
{
//    uint32 fifo_cnt=0;
    //Set GPIO to HIGH
    gpio_pin_intr_state_set(GPIO_ID_PIN(4), GPIO_PIN_INTR_DISABLE);
    ETS_GPIO_INTR_DISABLE();
    gpio16_output_set(1);
    uint8 i=0;
    uint16 c=0;
    system_update_cpu_freq(SYS_CPU_160MHZ);
    if (((writeCount-readCount)&fifo_mask)!=0)
    {
	Mp3DeselectControl();
	Mp3SelectData();

	while ((MP3_DREQ)&&(((writeCount-readCount)&fifo_mask)!=0))
	{
	    i=0;
	    while((((writeCount-readCount)&fifo_mask)!=0)&&(i<32))
	    {
		c++;
		i++;
		SPIPutChar(fifobuf[readCount++ & fifo_mask]);
	    }
	}
	Mp3DeselectData();
    }
//    fifo_cnt=((writeCount-readCount)&fifo_mask)*100/fifo_size;

    if ((!MP3_DREQ)&&(((writeCount-readCount)&fifo_mask)>0))
    {
	gpio_pin_intr_state_set(GPIO_ID_PIN(4), GPIO_PIN_INTR_POSEDGE);
	ETS_GPIO_INTR_ENABLE();
    }
    gpio16_output_set(0);
    system_update_cpu_freq(SYS_CPU_80MHZ);
}


// GPIO interrupt handler:store local data and start task
static void ICACHE_FLASH_ATTR GPIO4_handler(int8_t key){
    uint32 gpio_status;

    gpio_pin_intr_state_set(GPIO_ID_PIN(4), GPIO_PIN_INTR_DISABLE);
    gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    //clear interrupt status
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    system_os_post(user_procTaskPrio, 0, 0 );

}

void  ICACHE_FLASH_ATTR  user_init(void)
{

char outstr[40];
	// Configure the UART
//	uart_init(BIT_RATE_115200,0);
        uart_div_modify(0, UART_CLK_FREQ / 115200);
	// Enable system messages
	system_set_os_print(1);

extern SpiFlashChip * flashchip;

if(flashchip != NULL) os_printf("FlashID: 0x%08x\nChip size: %d\nBlock size: %d\nSector size: %d\nPage size: %d\nStatus mask: 0x%08x\n",
flashchip->deviceId, flashchip->chip_size, flashchip->block_size, flashchip->sector_size, flashchip->page_size, flashchip->status_mask );
else os_printf("Unknown Flash type!\n");

	if(wifi_get_opmode() != STATION_MODE)
	{
		setup_wifi_st_mode();
	}

	os_printf("Wifi setup ok\n\r");


    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);


        os_printf("I2C init...\n");
	i2c_master_gpio_init();
        i2c_master_init();

	os_printf("lcd init...\n");
	lcd_init();
	lcd_clear();
	lcd_fillscreen(0xAA);
	lcd_fillscreen(0x55);
	lcd_fillscreen(0xFF);
	lcd_fillscreen(0x00);
	lcd_init();
	lcd_clear();

	VS1003_Config();
	Mp3Reset();
	os_delay_us(100);
	Mp3SetVolume(leftvol,rightvol);
	Mp3WriteRegister(SPI_BASS, 0x00, 0x00);
//	Mp3WriteRegister(SPI_BASS, 0x7A, 0xF5);
	Mp3DeselectControl();


	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);


	os_timer_disarm(&gpio_timer);
	os_timer_setfn(&gpio_timer, (os_timer_func_t *)gpio_timerfunc, NULL);
	os_timer_arm(&gpio_timer, 500, 1);
/*
	os_timer_disarm(&mp3_timer);
	os_timer_setfn(&mp3_timer, (os_timer_func_t *)mp3_timerfunc, NULL);
	os_timer_arm(&mp3_timer, 100, 1);
*/

        os_printf("Starting SNTP client...");
//    ipaddr_aton("192.168.0.1", addr);
//    sntp_setserver(0,addr);
        sntp_setservername(0, sntp_server);
        if( true == sntp_set_timezone(+3) ) {
          sntp_init();
        }



    gpio16_output_conf();
   // set interrupt routine
   // start first run with timer(otherwise doesn't work?)
    ETS_GPIO_INTR_DISABLE();
   // GPIO12 interrupt handler
    ETS_GPIO_INTR_ATTACH(GPIO4_handler, 4);
    gpio_pin_intr_state_set(GPIO_ID_PIN(4), GPIO_PIN_INTR_DISABLE);

    //Start os task
    system_os_task(mp3_task, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
//    system_os_post(user_procTaskPrio, 0, 0 );



	// reset interrupt status
//    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(4));
//    ETS_GPIO_INTR_ENABLE();                                             // Enable gpio interrupts
//    ETS_INTR_UNLOCK();

//	system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
//	system_os_post(user_procTaskPrio, 0, 0 );

}
