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
#include "pcf8535.h"

#include "gpio.h"
#include "eagle_soc.h"
#include "wifi_api.c"

#include "driver/vs1003.h"

//#define user_procTaskPrio        0
//#define user_procTaskQueueLen    1

#define volstep 5
#define kb_nokey 0x00
#define kb_up 0x01
#define kb_down 0x02
#define kb_left 0x04
#define kb_right 0x08
#define kb_menu 0x10
#define kb_master 0x20
#define kb_exit 0x40


//os_event_t    user_procTaskQueue[user_procTaskQueueLen];

//static ETSTimer WiFiLinker;
static volatile os_timer_t WiFiLinker;
static volatile os_timer_t gpio_timer;
static volatile os_timer_t mp3_timer;

static void ICACHE_FLASH_ATTR wifi_check_ip(os_event_t *events);

//unsigned char *default_certificate;
//unsigned int default_certificate_len = 0;
//unsigned char *default_private_key;
//unsigned int default_private_key_len = 0;

//char radio[]={"http://78.140.251.2:80/mdl_320"};
static const char radio[][80]={"http://myserver.mydomain.ru:8000/myradio","http://nashe1.hostingradio.ru:80/nashe-128.mp3","http://nashe1.hostingradio.ru:80/ultra-128.mp3","http://nashe1.hostingradio.ru:80/jazz-128.mp3","http://nashe1.hostingradio.ru:80/best-128.mp3"};
static const char radio_names[][80]={"Мое радио","Наше радио","ULTRA","Radio JAZZ","BEST FM"};
uint8 leftvol=50, rightvol=50;
uint8 bass=0;
int8 treable=0;
uint8 bass_lim=0;
uint8 treable_lim=0;


uint8 url_idx=0;
uint16 zerobuf_cnt=0;
uint8 menu=0;
uint8 submenu=0;


//#define fifo_size 16384
#define max_url_idx 4
#define fifo_size 32768
uint8 fifobuf[fifo_size];

uint16	readCount=0;
uint16	writeCount=0;
uint8 change_lcd=0xff;
//char lcdbuf[8][25];


static const uint16 fifo_mask = fifo_size-1;
//static void ICACHE_FLASH_ATTR loop(os_event_t *events);

//Main code function

//static void ICACHE_FLASH_ATTR ICACHE_FLASH_ATTR
//loop(os_event_t *events)
void mp3_timerfunc(void *arg)
{

    uint32 cnt=0;
    uint8 i=0;
    uint16 c=0;
    // Increment counter
//    os_printf("\r\n-=TASK=-\r\n");
    system_update_cpu_freq(SYS_CPU_160MHZ);
    if (((writeCount-readCount)&fifo_mask)!=0)
    {
	Mp3DeselectControl();
	Mp3SelectData();
//	while (!(MP3_DREQ)) ;
	while ((MP3_DREQ)&&(((writeCount-readCount)&fifo_mask)!=0))
	{
	    i=0;
	    while((((writeCount-readCount)&fifo_mask)!=0)&&(i<32))
	    {
		cnt++;
		c++;
		i++;
		SPIPutChar(fifobuf[readCount++ & fifo_mask]);
	    }
	}

    Mp3DeselectData();

//    os_printf("DREQ p=%d r=%d w=%d s=%d\r\n", cnt, readCount, writeCount, ((writeCount-readCount)&fifo_mask));

//    os_printf("DREQ %5d (%2d), %5d\r\n", (writeCount-readCount)&fifo_mask, cnt, c);

//    if (((writeCount-readCount)&fifo_mask)!=0) gpio_pin_intr_state_set(GPIO_ID_PIN(4), 5);                        // Interrupt on Hi GPIO4 level old 5

	zerobuf_cnt=0;
    }
    else
    {
	if (mp3_conn!=NULL)
	{
	if (zerobuf_cnt>=100)
	{
	    if (mp3_conn!=NULL) espconn_disconnect(mp3_conn);
	    zerobuf_cnt=0;
	}
	else
	{
	    zerobuf_cnt++;
	}
	}
    }

//    os_printf("DREQ %5d (%2d)\r\n", (writeCount-readCount)&fifo_mask, cnt);
    os_printf("\033[2J\033[0;0f");
    os_printf("Radio: %s\r\n", radio_names[url_idx]);
    cnt=((writeCount-readCount)&fifo_mask)*100/fifo_size;
    for (i=0;i<cnt/5;i++)  os_printf(">");
    os_printf("\r\n");


    os_printf("DREQ %5d (%2d), %5d, %5d\r\n", (writeCount-readCount)&fifo_mask, cnt, c, zerobuf_cnt);
    if (rightvol<100)
    {
      os_printf("VOLUME %2d\r\n", 100-rightvol);
    }
    else
    {
      os_printf("VOLUME %2d\r\n", 0);
    }
    system_update_cpu_freq(SYS_CPU_80MHZ);
//    if (((writeCount-readCount)&fifo_mask)!=0)
//    os_delay_us(100000);
//    system_os_post(user_procTaskPrio, 0, 0 );
}


LOCAL void  radio_http_callback(char * response, int http_status, char * full_response)
{
	os_printf ("\n\r--------------disconnect -----------\n\r");
	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 10, 0);

}


char mp3_callback(char * buf, unsigned short len) {

	uint16 i,j,k;
	uint16 cnt=0;
//	os_printf("RECV l=%d\r\n", len);
	system_update_cpu_freq(SYS_CPU_160MHZ);

	if (((buf[0] == 'I') && (buf[1] == 'C') && (buf[2] == 'Y'))
			|| ((buf[0] == 'H') && (buf[1] == 'T') && (buf[2] == 'T') && (buf[3] == 'P'))) {

		os_printf("%s", buf);
		system_update_cpu_freq(SYS_CPU_80MHZ);
		return 0;
	}


/*

	if (((writeCount-readCount)&fifo_mask)!=0)
	{
	    // Запись в VS из FiFo
	    Mp3DeselectControl();
	    Mp3SelectData();
	    while ((MP3_DREQ)&&(((writeCount-readCount)&fifo_mask)!=0))
	    {
		j=0;
		while((((writeCount-readCount)&fifo_mask)!=0)&&(j<32))
		{
		    j++;
		    SPIPutChar(fifobuf[readCount++ & fifo_mask]);
		}
	    }
	    Mp3DeselectData();
	}
*/	
	i=0;

	if ((MP3_DREQ)&&(((writeCount-readCount)&fifo_mask)==0))
	{
	    // Запись в VS из Recv Buf
	    Mp3DeselectControl();
	    Mp3SelectData();
	    while((i<len)&&MP3_DREQ)
	    {
		j=0;
		while ((i<len)&&(j<32))
		{
		    j++;
	    	    SPIPutChar(buf[i++]);
		    cnt++;
		}
	    }
	    Mp3DeselectData();
	}


	while (i<len)
	{
	    if (((writeCount-readCount)&fifo_mask)>=(fifo_size-1))
	    {

		k=0;
		while ((fifo_size-((writeCount-readCount)&fifo_mask))<=(len-i))
		{
		// Запись в VS из FiFo
		    Mp3DeselectControl();
		    Mp3SelectData();
		    while ((MP3_DREQ)&&(((writeCount-readCount)&fifo_mask)!=0))
		    {
		    j=0;
		    while((((writeCount-readCount)&fifo_mask)!=0)&&(j<32))
		    {
			j++;
			k++;
			SPIPutChar(fifobuf[readCount++ & fifo_mask]);
		    }
		    }
		    Mp3DeselectData();

		}

//		    os_printf("<OVF> w=%5d i=%5d\r\n",k, i);
//	        system_os_post(user_procTaskPrio, 0, 0 );

	    }
	    else
	    {
		fifobuf[writeCount++ & fifo_mask]=buf[i++];
	    }
	}

//        os_printf("RECV c=%5d l=%5d r=%5d w=%5d s=%5d\r\n", cnt, len, readCount, writeCount, ((writeCount-readCount)&fifo_mask));
//	system_os_post(user_procTaskPrio, 0, 0 );
//	if (((writeCount-readCount)&fifo_mask)>(fifo_size-2048)) os_delay_us(10000);
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


void user_rf_pre_init(void)
{

}


void gpio_timerfunc(void *arg)
{
char i, b;
char strkey[4];
char key=0;
int adc=0;
char outstr[40];
int8  whole;
int8  decimal;



	system_update_cpu_freq(SYS_CPU_160MHZ);
    adc = system_adc_read(); // Р§С‚РµРЅРёРµ РђР¦Рџ
    if (adc<150)
    {
	key=kb_up;

    }
    else if (adc<300)
    {
	key=kb_down;
    }
    else if (adc<450)
    {
	key=kb_left;
    }
    else if (adc<600)
    {
	key=kb_right;
    }
    else if (adc<750)
    {
	key=kb_master;
    }
    else if (adc<900)
    {
	key=0x40;
    }
    else if (adc<1000)
    {
	key=kb_menu;
    }
    else
    {
	key=kb_nokey;
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
        	    if (mp3_conn!=NULL) espconn_disconnect(mp3_conn);
		  }
		  change_lcd=0x01;
		  break;
		}
		case kb_right:
		{
		  if (url_idx<max_url_idx)
		  {
		    url_idx++;
                    if (mp3_conn!=NULL) espconn_disconnect(mp3_conn);
		  }
		  change_lcd=0x01;
		  break;
		}
	      }
	      break;
	    }
	    case 1: // Volume
	    {
	      switch (key)
	      {
		case kb_left:
		{
		  if (rightvol+volstep<100)
		  {
		    rightvol+=volstep;
		  }
		  else
		  {
		    rightvol=255;
		  }
		  leftvol=rightvol;
		  Mp3SelectControl();
		  Mp3SetVolume(leftvol,rightvol);
		  Mp3DeselectControl();
		  change_lcd=0x02;
		  break;
		}
		case kb_right:
		{
		  if (rightvol-volstep>0)
		  {
		    if (rightvol!=255)
		    {
		      rightvol-=volstep;
		    }
		    else
		    {
		      rightvol=100;
		    }
		  }
		  else
		  {
		    rightvol=0;
		  }
		  leftvol=rightvol;
		  Mp3SelectControl();
		  Mp3SetVolume(leftvol,rightvol);
		  Mp3DeselectControl();
		  change_lcd=0x02;
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
		    change_lcd=0x04;
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
		    change_lcd=0x04;
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
		    change_lcd=0x08;
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
		    change_lcd=0x08;
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
		    change_lcd=0x10;
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
		    change_lcd=0x10;
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
		    change_lcd=0x20;
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
		    change_lcd=0x20;
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

	    change_lcd=0xff;
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
	    change_lcd=0xff;
	    break;
	  }

	}

    break;
    }
}


if (change_lcd)
{
//    pcf8535_clear();
    if (change_lcd&0x01)
    {

	pcf8535_gotoxy(0,7);
	os_sprintf(outstr,"STATION: ");
	pcf8535_print(outstr);

	if (submenu==0)
	{
	  os_sprintf(outstr,"<%11s>", radio_names[url_idx]);
	}
	else
	{
	  os_sprintf(outstr,"%13s", radio_names[url_idx]);
	}
	pcf8535_print(outstr);
    }
/*
    cnt=((writeCount-readCount)&fifo_mask)*100/fifo_size;
    for (i=0;i<cnt/5;i++)  os_printf(">");
    os_printf("\r\n");


    os_printf("DREQ %5d (%2d), %5d, %5d\r\n", (writeCount-readCount)&fifo_mask, cnt, c, zerobuf_cnt);
*/
    if (change_lcd&0x02)
    {
        pcf8535_gotoxy(0,6);
        os_sprintf(outstr,"VOLUME: ");
        pcf8535_print(outstr);
    

        if (rightvol<100)
        {
    	    if (submenu==1)
	    {
             os_sprintf(outstr,"<%3d%%>\n", 100-rightvol);
	    }
	    else
	    {
    	     os_sprintf(outstr,"%3d%%\n", 100-rightvol);
	    }
	}
	else
	{
	    if (submenu==1)
	    {
             os_sprintf(outstr,"<%3d%%>\n", 0);
	    }
	    else
	    {
             os_sprintf(outstr,"%3d%%\n", 0);
	    }
	}
	pcf8535_print(outstr);
    }

    if (change_lcd&0x04)
    {
        pcf8535_gotoxy(0,5);
        os_sprintf(outstr,"TREABLE: ");
        pcf8535_print(outstr);

        whole    = (float)treable*1.5;
        decimal = (((float)treable*1.5) - whole ) * 10; // only deci ..


        if (submenu==2)
        {
          os_sprintf(outstr,"<%+02d.%01i>\n", whole, abs(decimal));
        }
        else
        {
          os_sprintf(outstr,"%+02d.%01i\n", whole, abs(decimal));
        }
        pcf8535_print(outstr);
    }

    if (change_lcd&0x08)
    {
        pcf8535_gotoxy(0,4);
        os_sprintf(outstr,"TREABLE_LIM: ");
        pcf8535_print(outstr);
        if (submenu==3)
        {
          os_sprintf(outstr,"<%2dkHz>\n", treable_lim);
        }
        else
        {
          os_sprintf(outstr,"%2dkHz\n", treable_lim);
        }
        pcf8535_print(outstr);
    }

    if (change_lcd&0x10)
    {
        pcf8535_gotoxy(0,3);
        os_sprintf(outstr,"BASS: ");
        pcf8535_print(outstr);

        if (submenu==4)
        {
          os_sprintf(outstr,"<%2ddB>\n", bass);
        }
        else
        {
          os_sprintf(outstr,"%2ddB\n", bass);
        }
        pcf8535_print(outstr);
    }

    if (change_lcd&0x20)
    {
        pcf8535_gotoxy(0,2);
        os_sprintf(outstr,"BASS_LIM: ");
        pcf8535_print(outstr);
        if (submenu==5)
        {
          os_sprintf(outstr,"<%2dHz>\n", bass_lim*10);
        }
        else
        {
          os_sprintf(outstr,"%2dHz\n", bass_lim*10);
        }
	pcf8535_print(outstr);
    }


    if (change_lcd&0x40)
    {
        os_sprintf(outstr,"MODE: %2d %2d\n", menu, submenu);
        pcf8535_gotoxy(0,1);
        pcf8535_print(outstr);
        change_lcd=false;
    }

    if (change_lcd&0x80)
    {
        pcf8535_gotoxy(0,0);
        os_sprintf(outstr,"Skw WiFi Radio v.0.0.1");
        pcf8535_print(outstr);
    }
    change_lcd=0;
}
    system_update_cpu_freq(SYS_CPU_80MHZ);
}


void user_init(void)
{

char outstr[40];
	// Configure the UART
//	uart_init(BIT_RATE_115200,0);
        uart_div_modify(0, UART_CLK_FREQ / 115200);
	// Enable system messages
	system_set_os_print(1);

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

	os_printf("pcf8535 init...\n");
	pcf8535_init();
	pcf8535_clear();


	VS1003_Config();
	Mp3Reset();
	os_delay_us(100);
	Mp3SetVolume(leftvol,rightvol);
	Mp3WriteRegister(SPI_BASS, 0x00, 0x00);
//	Mp3WriteRegister(SPI_BASS, 0x7A, 0xF5);
	Mp3DeselectControl();

//	ETS_GPIO_INTR_ENABLE();                                             // Enable gpio interrupts

	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);


	os_timer_disarm(&gpio_timer);
	os_timer_setfn(&gpio_timer, (os_timer_func_t *)gpio_timerfunc, NULL);
	os_timer_arm(&gpio_timer, 500, 1);

	os_timer_disarm(&mp3_timer);
	os_timer_setfn(&mp3_timer, (os_timer_func_t *)mp3_timerfunc, NULL);
	os_timer_arm(&mp3_timer, 100, 1);




//	system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
//	system_os_post(user_procTaskPrio, 0, 0 );

}
// r b w o y r
