# Esp8266_WebRadio_tic154_vs1003
ESP8266 + VS1003 MP3 Decoder + TIC154 LCD + ADC resistive keyboard

Functions:
- multiple WebRadio stations (see user_main.c)
- 32768 bytes FiFo buffer
- Volume, Treable and Bass control by keyboard
- LCD display I2C TIC154 (SCL - GPIO2, SDA - GPIO0)

VS1003:
MISO - GPIO12
MOSI - GPIO13
SCK - GPIO14
DREQ - GPIO4
XRST - +3.3V
XCS - GPIO5
XDCS - GPIO15