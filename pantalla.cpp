#include <cstdlib>
#include "pantalla.h"
#include "icons.h"


void pantalla_init(TFT_eSPI &tft) {
  tft.init();
  tft.setRotation(2);
}


void pantalla_setup(TFT_eSPI &tft, uint16_t color) {
  tft.fillScreen(TFT_BLACK);
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
    // tft.setTextColor(TFT_GREEN, TFT_BLACK);
  } else if (hdop <= 2) {
    tft.drawBitmap(176, 0, gps_icon, 32, 32, TFT_BLACK, TFT_YELLOW);
    // tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  } else if (hdop < 5) {
    tft.drawBitmap(176, 0, gps_icon, 32, 32, TFT_BLACK, TFT_ORANGE);
    // tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  } else {
    tft.drawBitmap(176, 0, gps_icon, 32, 32, TFT_BLACK, TFT_RED);
    // tft.setTextColor(TFT_RED, TFT_BLACK);
  }

  // tft.printf("%d   ", gps.satellites.value());
  // tft.setTextColor(TFT_BLUE, TFT_BLACK);

  tft.setTextSize(4);
  // Speed
  tft.setCursor(X, Y + 60);
  tft.printf("%.1f kph   ", gps.speed.kmph());
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
  if (toque.zRaw != 0) {
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
