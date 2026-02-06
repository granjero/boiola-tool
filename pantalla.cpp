#include <Arduino.h>
#include "pantalla.h"
#include "icons.h"

extern TFT_eSPI tft;

void pantalla_init(TFT_eSPI &tft) {
  tft.init();
  tft.setRotation(2);
}


void pantalla_setup(TFT_eSPI &tft, uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextWrap(false);
  tft.setTextSize(2);
  int16_t w = tft.width();
  int16_t h = tft.height();
  tft.fillRect(0, 0, w, 2, color);
  tft.fillRect(0, h - 2, w, 2, color);
  tft.fillRect(0, 0, 2, h, color);
  tft.fillRect(w - 2, 0, 2, h, color);
}


void pantalla_bandera(TFT_eSPI &tft, int x, int y, int tamanio) {
  // proporcion 8:5
  int w = tamanio * 8;
  int h = tamanio * 5;

  int banda = h / 3;

  tft.fillRect(x, y, w, banda, TFT_CYAN);
  tft.fillRect(x, y + banda, w, banda, TFT_WHITE);
  tft.fillRect(x, y + 2 * banda, w, banda, TFT_CYAN);
}

void pantalla_icono_sd(TFT_eSPI &tft, bool estado) {
  if (estado) {
    tft.drawBitmap(208, 0, sd_icon, 32, 32, TFT_BLACK, TFT_GREEN);
  } else {
    tft.drawBitmap(208, 0, sd_icon, 32, 32, TFT_BLACK, TFT_RED);
  }
}


void pantalla_fecha(TFT_eSPI &tft, TinyGPSPlus &gps) {
  // Date
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.setCursor(55, 0);
  tft.printf("%02d-%02d-%04d",
             gps.date.day(),
             gps.date.month(),
             gps.date.year());

  // Time
  tft.setCursor(55, 17);
  tft.printf("%02d:%02d UTC",
             gps.time.hour(),
             gps.time.minute());
}


void pantalla_gps(TFT_eSPI &tft, TinyGPSPlus &gps, int y) {

  float hdop = gps.hdop.hdop();

  double lat = gps.location.lat();
  double lon = gps.location.lng();
  int Y = y;
  int X = 10;

  tft.setTextSize(2);
  // Lat
  tft.setCursor(X, Y);
  tft.printf("%s %.4f    ", latDir(lat), abs(lat));
  // Lon
  tft.setCursor(X, Y + 20);
  tft.printf("%s %.4f   ", lonDir(lon), abs(lon));
  // Alt
  tft.setCursor(X, Y + 40);
  tft.printf("%.2f msnm   ", gps.altitude.meters());

  // Satellites (color by HDOP)

  if (hdop <= 1) {
    tft.drawBitmap(176, 0, gps_icon, 32, 32, TFT_BLACK, TFT_GREEN);
  } else if (hdop <= 2) {
    tft.drawBitmap(176, 0, gps_icon, 32, 32, TFT_BLACK, TFT_YELLOW);
  } else if (hdop < 5) {
    tft.drawBitmap(176, 0, gps_icon, 32, 32, TFT_BLACK, TFT_ORANGE);
  } else {
    tft.drawBitmap(176, 0, gps_icon, 32, 32, TFT_BLACK, TFT_RED);
  }
  tft.setTextSize(4);
  // Speed
  tft.setCursor(X, Y + 60);
  tft.printf("%.1f kph ", gps.speed.kmph());
}


const char *latDir(double lat) {
  return (lat >= 0) ? "N+" : "S-";
}


const char *lonDir(double lon) {
  return (lon >= 0) ? "E+" : "O-";
}


void pantalla_touch(TFT_eSPI &tft, XPT2046_Bitbang &touch) {

  TouchPoint toque = touch.getTouch();

  // Display touches that have a pressure value (Z)
  if (toque.zRaw >= 1000) {
    tft.setTextSize(1);
    tft.setCursor(10, 310);
    tft.print("x: ");
    tft.print(toque.x);
    tft.print(" y: ");
    tft.print(toque.y);
    tft.print(" z: ");
    tft.print(toque.zRaw);
    tft.print("     ");
  }
}



void pantalla_img_jpg(TFT_eSPI &tft, TJpg_Decoder &tjpj) {

  tft.setSwapBytes(true);  // We need to swap the colour bytes (endianess)
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(pantalla_jpg_output);
  TJpgDec.drawSdJpg(0, 0, "/pics/boiola.jpg");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(15, 310);
  tft.print("boiola-tool v0.1");
}

bool pantalla_jpg_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  if (y >= tft.height()) return false;
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}
