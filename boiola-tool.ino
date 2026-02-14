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
TouchPoint toque;

Chrono web_on_off_debounce;

enum Estado_BOIOLA {
  ERROR,
  IDLE,
  MENU0,
};

Estado_BOIOLA estado_actual_app = IDLE;

// bool estado_pantalla = true;
bool estado_sd = false;

// File archivo;

char filename[32] = "/boiola.boot";

void setup() {
  pantalla_init(tft);  // inicia la pantalla
  touch.begin();       // inicia el touch
  // touch.setCalibration(250, 3800, 340, 3800);
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

  // debug
  // tft.setTextSize(2);
  // tft.setCursor(0, 80);
  // tft.println(gps.satellites.value());
  // tft.println(gps.hdop.hdop());
  // tft.println(gps.hdop.value());

  // todo el tiempo
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  sd_guarda_dato_gpx(gps, filename, sizeof(filename));

  toque = pantalla_touch(touch);
  pantalla_xy(tft, toque);
  pantalla_on_off(toque);

  // segun Estado_BOIOLA
  switch (estado_actual_app) {
    case ERROR:
      tft.fillScreen(TFT_RED);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.setCursor(0, 40);
      tft.print("estado SD: ");
      tft.println(sd_estado());
      while (true) {}
      break;

    case IDLE:
      pantalla_fecha_y_hora(tft, gps);
      pantalla_icono_server_wifi(tft, web_is_running());
      pantalla_icono_gps(tft, gps);
      pantalla_icono_sd(tft, sd_estado());
      if (pantalla_encendida() && toque.x > 60 && toque.x < 160 && toque.y > 110 && toque.y < 180) {
        estado_actual_app = MENU0;
        pantalla_setup_menu0(tft);
      }
      break;

    case MENU0:
      pantalla_icono_server_wifi(tft, web_is_running());
      pantalla_icono_gps(tft, gps);
      pantalla_icono_sd(tft, sd_estado());
      if (toque.x > 130 && toque.y > 190) {
        estado_actual_app = IDLE;
        pantalla_setup(tft, TFT_BLACK);
        pantalla_img_jpg(tft, TJpgDec);  // imagen
      }
      break;
  }




  // if (web_on_off_debounce.hasPassed(1000)
  //     && toque.zRaw >= 1000 && toque.x <= 50 && toque.y >= 250) {
  //   web_on_off_debounce.restart();
  //
  //   if (!web_is_running()) {
  //     pantalla_icono_server_wifi(tft, web_start());
  //   } else {
  //     web_stop();
  //     pantalla_icono_server_wifi(tft, web_is_running());
  //   }
  // }
}
