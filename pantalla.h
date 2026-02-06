#ifndef PANTALLA_H
#define PANTALLA_H

#include <SPI.h>
#include <TFT_eSPI.h>
#include <TinyGPSPlus.h>
#include <XPT2046_Bitbang.h>
#include <TJpg_Decoder.h>

// -----------------
//PANTALLA
// inicia la pantalla
void pantalla_init(TFT_eSPI &tft);
// dibuja el setup
void pantalla_setup(TFT_eSPI &tft, uint16_t color);
// dibuja una bandera
void pantalla_bandera(TFT_eSPI &tft, int x, int y, int tamanio);
// icono sd
void pantalla_icono_sd(TFT_eSPI &tft, bool estado);
// imagen jpg
void pantalla_img_jpg(TFT_eSPI &tft, TJpg_Decoder &tjpj);
bool pantalla_jpg_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);

// -----------------
// TOUCH
// imprime los valores xyz del touch
void pantalla_touch(TFT_eSPI &tft, XPT2046_Bitbang &touch);

// -----------------
//GPS
void pantalla_fecha(TFT_eSPI &tft, TinyGPSPlus &gps);
void pantalla_gps(TFT_eSPI &tft, TinyGPSPlus &gps, int y);
const char *latDir(double lat);
const char *lonDir(double lon);

#endif
