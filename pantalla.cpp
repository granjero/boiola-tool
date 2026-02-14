#include "XPT2046_Bitbang.h"
#include <cstdint>
// #include "esp32-hal-gpio.h"
#include "TFT_eSPI.h"
#include <Arduino.h>
#include <Chrono.h>
#include "pantalla.h"
#include "icons.h"

extern TFT_eSPI tft;

Chrono pantalla_on_off_debounce;
Chrono pantalla_off_debounce(Chrono::SECONDS);
Chrono touch_debounce;


bool pantalla_esta_encendida = true;

void pantalla_init(TFT_eSPI &tft) {
  tft.init();
  tft.setRotation(2);
}


void pantalla_setup(TFT_eSPI &tft, uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextWrap(false);
  tft.fillRect(0, 0, tft.width(), 40, TFT_BLACK);  // para la hora y los iconos
}


bool pantalla_encendida() {
  return pantalla_esta_encendida;
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
    tft.drawBitmap(
      tft.width() - ICON - BORDE,
      BORDE,
      sd_icon,
      ICON,
      ICON,
      TFT_BLACK,
      TFT_GREEN);
  } else {
    tft.drawBitmap(
      tft.width() - ICON - BORDE,
      BORDE,
      sd_icon,
      ICON,
      ICON,
      TFT_BLACK,
      TFT_RED);
  }
}
void pantalla_icono_gps(TFT_eSPI &tft, TinyGPSPlus &gps) {
  if (gps.hdop.isValid()) {
    float hdop = gps.hdop.hdop();
    if (hdop <= 1) {
      tft.drawBitmap(
        tft.width() - ICON * 2 - BORDE,
        BORDE,
        gps_icon,
        ICON,
        ICON,
        TFT_BLACK,
        TFT_GREEN);
    } else if (hdop <= 2) {
      tft.drawBitmap(
        tft.width() - ICON * 2 - BORDE,
        BORDE,
        gps_icon,
        ICON,
        ICON,
        TFT_BLACK,
        TFT_YELLOW);
    } else if (hdop < 5) {
      tft.drawBitmap(
        tft.width() - ICON * 2 - BORDE,
        BORDE,
        gps_icon,
        ICON,
        ICON,
        TFT_BLACK,
        TFT_ORANGE);
    } else {
      tft.drawBitmap(
        tft.width() - ICON * 2 - BORDE,
        BORDE,
        gps_icon,
        ICON,
        ICON,
        TFT_BLACK,
        TFT_RED);
    }
  } else {
    tft.drawBitmap(
      tft.width() - ICON * 2 - BORDE,
      BORDE,
      gps_invalid_icon,
      ICON,
      ICON,
      TFT_BLACK,
      TFT_RED);
  }
}

void pantalla_icono_server_wifi(TFT_eSPI &tft, bool estado) {
  if (estado) tft.drawBitmap(
    tft.width() - ICON * 3 - BORDE,
    BORDE,
    server_wifi_icon,
    ICON,
    ICON,
    TFT_BLACK,
    TFT_GREEN);
  else tft.drawBitmap(
    tft.width() - ICON * 3 - BORDE,
    BORDE,
    no_server_wifi_icon,
    ICON,
    ICON,
    TFT_BLACK,
    TFT_RED);
}


void pantalla_fecha_y_hora(TFT_eSPI &tft, TinyGPSPlus &gps) {
  // Date
  if (gps.time.isUpdated() && gps.time.isValid() && gps.date.isUpdated() && gps.date.isValid()) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(BORDE, BORDE);
    tft.printf("%02d-%02d-%04d",
               gps.date.day(),
               gps.date.month(),
               gps.date.year());
    tft.setCursor(BORDE, 20);
    tft.printf("%02d:%02d UTC",
               gps.time.hour(),
               gps.time.minute());
    tft.setCursor(0, 38);
    tft.printf("%02d:%02d(-3)  ",
               horaGMT(gps.time.hour(), -3),
               gps.time.minute());
    tft.printf("%02d:%02d(+9)",
               horaGMT(gps.time.hour(), 9),
               gps.time.minute());
  }
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

  pantalla_icono_gps(tft, gps);

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


TouchPoint pantalla_touch(XPT2046_Bitbang &touch) {
  TouchPoint toque = touch.getTouch();

  int16_t x = toque.x;
  int16_t y = toque.y;

  toque.x = y;
  toque.y = 320 - x;

  if (touch_debounce.hasPassed(150) && toque.zRaw >= 2000) {
    return toque;
  }
  toque.zRaw = 0;
  return toque;
}

void pantalla_xy(TFT_eSPI &tft, TouchPoint &toque) {
  if (toque.zRaw > 0) {
    tft.setTextSize(1);
    tft.setCursor(10, 310);
    tft.printf("xR:%d yR:%d zR:%d    ", toque.xRaw, toque.yRaw, toque.zRaw);
    tft.setCursor(10, 290);
    tft.printf("x:%d y:%d     ", toque.x, toque.y);
    tft.fillCircle(toque.x, toque.y, 4, TFT_PINK);
  }
}



bool pantalla_on_off(TouchPoint &toque) {
  if (pantalla_off_debounce.hasPassed(60 * 5)) {
    pantalla_off_debounce.restart();
    pantalla_esta_encendida = false;
    digitalWrite(TFT_BL, pantalla_esta_encendida);
  } else if (pantalla_on_off_debounce.hasPassed(1000) && toque.zRaw >= 2000 && toque.x >= 190 && toque.y >= 270) {
    pantalla_on_off_debounce.restart();
    pantalla_off_debounce.restart();
    pantalla_esta_encendida = !pantalla_esta_encendida;
    digitalWrite(TFT_BL, pantalla_esta_encendida);
  }
  return pantalla_esta_encendida;
}

void pantalla_img_jpg(TFT_eSPI &tft, TJpg_Decoder &tjpj) {

  tft.setSwapBytes(true);  // We need to swap the colour bytes (endianess)
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(pantalla_jpg_output);
  TJpgDec.drawSdJpg(0, 0, "/pics/boiola.jpg");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(15, 310);
  tft.setTextSize(1);
  tft.print("boiola-tool v0.1");
}


bool pantalla_jpg_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  if (y >= tft.height()) return false;
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

int8_t horaGMT(uint8_t hora, int8_t gmt) {
  int8_t resultado = hora + gmt;
  if (resultado < 0) return 24 + resultado;  // resultado negativo (-- = +)
  if (resultado > 23) return resultado - 24;
  return resultado;
}

void pantalla_setup_menu0(TFT_eSPI &tft) {
  tft.fillScreen(TFT_BLACK);
  tft.drawLine(tft.width() / 2, 50, tft.width() / 2, 320 - 50, TFT_PURPLE);
  tft.drawLine(30, tft.height() / 2, 240 - 30, tft.height() / 2, TFT_PURPLE);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);

  tft.drawBitmap(
    30,
    60,
    gpx_download,
    64,
    64,
    TFT_BLACK,
    TFT_GREENYELLOW);
  // tft.setCursor(10, 60);
  // tft.print("Descargar");
  // tft.setCursor(10, 80);
  // tft.print("GPX");

  tft.setCursor(150, 190);
  tft.print("VOLVER");
}
