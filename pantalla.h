#ifndef PANTALLA_H
#define PANTALLA_H

#define TOUCH_MIN_X 303
#define TOUCH_MAX_X 3824
#define TOUCH_MIN_Y 120
#define TOUCH_MAX_Y 3933


#include <SPI.h>
// #include <XPT2046_Touchscreen.h>
#include <TFT_eSPI.h>
#include <TinyGPSPlus.h>
#include <XPT2046_Bitbang.h>

// -----------------
//PANTALLA
// inicia la pantalla
void pantalla_init(TFT_eSPI &tft);
// dibuja el setup
void pantalla_setup(TFT_eSPI &tft, uint16_t color);
// dibuja una bandera
void pantalla_bandera(TFT_eSPI &tft, int x, int y, int tamanio);


// -----------------
// TOUCH
// imprime los valores xyz del touch
// void pantalla_valores_touch(TFT_eSPI &tft);
void pantalla_touch(TFT_eSPI &tft, XPT2046_Bitbang &touch);

// -----------------
//GPS
void pantalla_fecha(TFT_eSPI &tft, TinyGPSPlus &gps);
void pantalla_gps(TFT_eSPI &tft, TinyGPSPlus &gps, int y);
const char *latDir(double lat);
const char *lonDir(double lon);

#endif
