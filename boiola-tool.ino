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
#include "boiola_web.h"


XPT2046_Bitbang touch(TOUCH_MOSI, TOUCH_MISO, TOUCH_CLK, TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

// Chrono sd_write_interval(Chrono::SECONDS);
Chrono web_on_off_debounce;

enum Estado_BOIOLA {
  ERROR,
  IDLE,
};

bool estado_pantalla = true;
bool estado_sd = false;

Estado_BOIOLA estado_actual_app = IDLE;

// File archivo;

char filename[32] = "/boiola.boot";

void setup() {
  pantalla_init(tft);                                         // inicia la pantalla
  touch.begin();                                              // inicia el touch
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);  // inicia HardwareSerial GPS
  estado_sd = sd_init();                                      // intenta iniciar la sd

  pantalla_setup(tft, TFT_PURPLE);  // setup borde
  tft.println(estado_sd);
  if (estado_sd) {
    sd_boot_record(filename);
    pantalla_img_jpg(tft, TJpgDec);  // imagen
    // delay(10000);
  } else {
    estado_actual_app = ERROR;
  }

  pantalla_icono_server_wifi(tft, web_is_running());
  pantalla_icono_gps(tft, gps);
  pantalla_icono_sd(tft, sd_estado());
}

void loop() {
  // todo el tiempo
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  pantalla_touch(tft, touch);
  pantalla_on_off(tft, touch);

  // segun Estado_BOIOLA
  switch (estado_actual_app) {
    case ERROR:
      tft.fillScreen(TFT_RED);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.setCursor(0, 40);
      tft.print("estado SD: ");
      tft.println(sd_estado());

    case IDLE:
      pantalla_fecha_y_hora(tft, gps);
      pantalla_icono_server_wifi(tft, web_is_running());
      pantalla_icono_gps(tft, gps);
      pantalla_icono_sd(tft, sd_estado());

      sd_guarda_dato_gpx(gps, filename, sizeof(filename));
  }

  tft.setCursor(0, 100);
  tft.print(filename);


  if (web_on_off_debounce.hasPassed(1000)) {
    web_on_off_debounce.restart();

    TouchPoint toque = touch.getTouch();
    if (toque.zRaw >= 1500 && toque.x <= 70 && toque.y <= 70) {
      if (!web_is_running()) {
        pantalla_icono_server_wifi(tft, web_start());
      } else {
        web_stop();
        pantalla_icono_server_wifi(tft, web_is_running());
      }
    }
  }
}


// viejo loop
// void loop() {
//   // data GPS
//   while (gpsSerial.available()) {
//     gps.encode(gpsSerial.read());
//   }
//
//   if (gps.location.isUpdated() && gps.location.isValid()) {
//     pantalla_fecha_y_hora(tft, gps);
//     pantalla_gps(tft, gps, 60);
//   }
//
//   if (estado_sd && sd_write_interval.hasPassed(10)) {
//     sd_write_interval.restart();
//     sd_set_filename(gps, filename, sizeof(filename));
//     if (!SD.exists(filename)) {
//       sd_file_encabezado(filename);
//     } else {
//       sd_file_append_gps_point(filename, gps);
//     }
//   }
//
//   pantalla_touch(tft, touch);
//   if(pantalla_on_off_debounce.hasPassed(500)) {
//     pantalla_on_off_debounce.restart();
//     digitalWrite(PANTALLA_PW_PIN, pantalla_on_off(tft, touch));
//   }
//
//
//   if (web_on_off_debounce.hasPassed(1000)) {
//     web_on_off_debounce.restart();
//
//     TouchPoint toque = touch.getTouch();
//     if (toque.zRaw >= 1500 && toque.x <= 70 && toque.y <= 70) {
//       if(!web_is_running()){
//         pantalla_icono_server_wifi(tft, web_start());
//       } else {
//         web_stop();
//
//     }
//   }
// }
