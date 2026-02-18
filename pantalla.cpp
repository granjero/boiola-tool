#include <cstdint>
#include "TFT_eSPI.h"
// #include "esp32-hal.h"
// #include "XPT2046_Bitbang.h"
// #include <cstdint>
// #include "esp32-hal-gpio.h"
// #include "TFT_eSPI.h"
// #include <Arduino.h>
// #include <Chrono.h>
#include "pantalla.h"
#include "icons.h"

extern TFT_eSPI tft;

Chrono pantalla_on_off_debounce;
Chrono pantalla_off_debounce(Chrono::SECONDS);
// Chrono touch_debounce;


bool pantalla_esta_encendida = true;

void pantalla_init(TFT_eSPI &tft) {
  tft.init();
  tft.setRotation(2);
}


void pantalla_setup(TFT_eSPI &tft) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextWrap(false);
  tft.fillRect(0, 0, tft.width(), 40, TFT_BLACK);  // para la hora y los iconos
  // tft.println("SETUP");
  // delay(3000);
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
  uint16_t color;
  uint8_t x = tft.width() - ICON - BORDE;
  uint8_t y = BORDE;
  if (estado) color = TFT_GREEN;
  else color = TFT_RED;
  tft.drawBitmap(
    x,
    y,
    sd_icon,
    ICON,
    ICON,
    TFT_BLACK,
    color);
}
void pantalla_icono_gps(TFT_eSPI &tft, TinyGPSPlus &gps) {
  uint16_t color;
  uint8_t x = tft.width() - ICON * 2 - BORDE * 2;
  uint8_t y = BORDE;
  if (gps.hdop.isValid()) {
    float hdop = gps.hdop.hdop();

    if (hdop <= 1) color = TFT_GREEN;
    else if (hdop <= 2) color = TFT_YELLOW;
    else if (hdop <= 5) color = TFT_ORANGE;
    else color = TFT_RED;

    tft.drawBitmap(
      x,
      y,
      gps_icon,
      ICON,
      ICON,
      TFT_BLACK,
      color);

  } else {
    tft.drawBitmap(
      x,
      y,
      gps_invalid_icon,
      ICON,
      ICON,
      TFT_BLACK,
      TFT_RED);
  }
  tft.setCursor(tft.width() - ICON - BORDE, BORDE);
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.printf("%d", gps.satellites.value());
}

void pantalla_icono_server_wifi(TFT_eSPI &tft, bool estado) {
  uint16_t color;
  uint8_t x = tft.width() - ICON * 3 - BORDE * 3;
  uint8_t y = BORDE;
  if (estado)
    color = TFT_GREEN;

  else color = TFT_RED;

  tft.drawBitmap(
    x,
    y,
    no_server_wifi_icon,
    ICON,
    ICON,
    TFT_BLACK,
    color);
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
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(X, Y);
  tft.printf("%s %.4f    ", latDir(lat), abs(lat));
  tft.setCursor(X, Y + 20);
  tft.printf("%s %.4f   ", lonDir(lon), abs(lon));
  tft.setCursor(X, Y + 40);
  tft.printf("%.2f msnm   ", gps.altitude.meters());
  tft.setCursor(X, Y + 60);
  tft.printf("%.1f kph ", gps.speed.kmph());
  tft.setCursor(X, Y + 80);
  tft.printf("Sats: %d        ", gps.satellites.value());
  tft.setCursor(X, Y + 100);
  tft.printf("Age: %f", gps.location.age());
  tft.setCursor(X, Y + 120);
  tft.printf("RX: %d", gps.charsProcessed());
  tft.setCursor(X, Y + 140);
  tft.printf("fail: %d", gps.failedChecksum());
  tft.setCursor(X, Y + 160);
  tft.printf("pass: %d", gps.passedChecksum());
}


const char *latDir(double lat) {
  return (lat >= 0) ? "N+" : "S-";
}


const char *lonDir(double lon) {
  return (lon >= 0) ? "E+" : "O-";
}


void pantalla_xy(TFT_eSPI &tft, TouchPoint &toque) {
  if (toque.zRaw > 0) {
    tft.setTextSize(1);
    // tft.setCursor(10, 290);
    // tft.printf("xR:%d yR:%d zR:%d    ", toque.xRaw, toque.yRaw, toque.zRaw);
    tft.setCursor(10, 310);
    tft.printf("x:%d y:%d     ", toque.x, toque.y);
    tft.fillCircle(toque.x, toque.y, 4, TFT_PINK);
  }
}


void pantalla_on_off() {
  // if (pantalla_off_debounce.hasPassed(60 * 5)) {
  //   pantalla_off_debounce.restart();
  //   pantalla_esta_encendida = false;
  //   digitalWrite(TFT_BL, pantalla_esta_encendida);
  // } else if (pantalla_on_off_debounce.hasPassed(1000) && toque.zRaw >= 2000 && toque.x >= 190 && toque.y >= 270) {
  //   pantalla_on_off_debounce.restart();
  //   pantalla_off_debounce.restart();
  //   pantalla_esta_encendida = !pantalla_esta_encendida;
  //   digitalWrite(TFT_BL, pantalla_esta_encendida);
  // }
  // return pantalla_esta_encendida;
  if (pantalla_on_off_debounce.hasPassed(1000)) {
    pantalla_on_off_debounce.restart();
    pantalla_esta_encendida = !pantalla_esta_encendida;
    digitalWrite(TFT_BL, pantalla_esta_encendida);
  }
}


void pantalla_auto_off() {
  if (pantalla_off_debounce.hasPassed(60 * 3)) {
    pantalla_off_debounce.restart();
    pantalla_esta_encendida = false;
    digitalWrite(TFT_BL, pantalla_esta_encendida);
  }
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
  tft.drawWideLine(tft.width() / 2.0, 50, tft.width() / 2.0, 320 - 50, 3, TFT_PURPLE);
  tft.drawWideLine(30, tft.height() / 2.0, 240 - 30, tft.height() / 2.0, 3, TFT_PURPLE);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);

  // icono Server GPX
  tft.drawBitmap(
    40,
    180,
    gpx_download,
    64,
    64,
    TFT_BLACK,
    TFT_GREENYELLOW);
  // icono Volver
  tft.drawBitmap(
    130,
    180,
    volver,
    64,
    64,
    TFT_BLACK,
    TFT_GREENYELLOW);
  // tft.setCursor(130, 190);
  // tft.print("VOLVER");
}
