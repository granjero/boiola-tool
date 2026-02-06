#include <Arduino.h>
#include <HardwareSerial.h>
#include <Chrono.h>
// #include <TJpg_Decoder.h>

#define TOUCH_CLK 25
#define TOUCH_MOSI 32
#define TOUCH_CS 33
#define TOUCH_IRQ 36
#define TOUCH_MISO 39
#define SD_CS 5
#define PANTALLA_PW_PIN 21

#define GPS_RX_PIN 27
#define GPS_TX_PIN 22

#include "pantalla.h"
#include "boiola_sd.h"
// #include "icons.h"

XPT2046_Bitbang touch(TOUCH_MOSI, TOUCH_MISO, TOUCH_CLK, TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);
Chrono sd_write_interval(Chrono::SECONDS);

bool estado_pantalla = true;
bool estado_sd = false;

File archivo;

char filename[32] = "/boiola.boot";

void setup() {
  pantalla_init(tft);                                         // inicia la pantalla
  touch.begin();                                              // inicia el touch
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);  // inicia HardwareSerial GPS
  estado_sd = sd_init();                                      // intenta iniciar la sd

  if (estado_sd) {
    File archivo = SD.open(filename, FILE_APPEND, true);
    if (archivo) {
      archivo.print("boot;");
      archivo.close();
    }
    pantalla_img_jpg(tft, TJpgDec);  // imagen
    delay(10000);
  }

  pantalla_setup(tft, TFT_PURPLE);  // setup borde
  pantalla_bandera(tft, 0, 0, 5);   // bandera
  pantalla_icono_sd(tft, estado_sd);
}


void loop() {
  // data GPS
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated() && gps.location.isValid()) {
    pantalla_fecha(tft, gps);
    pantalla_gps(tft, gps, 60);
  }

  if (estado_sd && sd_write_interval.hasPassed(10)) {
    sd_write_interval.restart();
    sd_set_filename(gps, filename, sizeof(filename));
    if (!SD.exists(filename)) {
      sd_file_encabezado(filename);
    } else {
      sd_file_append_gps_point(filename, gps);
    }
  }

  pantalla_touch(tft, touch);
}
