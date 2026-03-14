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
// #define PANTALLA_PW_PIN 21

#define GPS_RX_PIN 27
#define GPS_TX_PIN 22

#include "pantalla.h"
#include "boiola_sd.h"
#include "boiola_web.h"
#include "boiola_trip.h"
#include "touch.h"
#include "icons.h"

enum Estado_BOIOLA {
  ERROR,
  IDLE,
  MENU0,
  GPS_DATA,
  GPX_SERVER,
  TRIP_MENU,
  TRIP_ACTIVE,
};

Estado_BOIOLA estado_actual_app = IDLE;

void set_estado_actual_app(int estado) {
  estado_actual_app = (Estado_BOIOLA)estado;
}

XPT2046_Bitbang touch(TOUCH_MOSI, TOUCH_MISO, TOUCH_CLK, TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);
TouchPoint toque;

PropiedadesBoton botonOnOffPantalla = { 200, 290, 40, 30, nullptr, 0 };
PropiedadesBoton botonMenu0 = { 70, 100, 100, 50, set_estado_actual_app, MENU0 };

PropiedadesBoton botonMenu1 = { 40, 60, 60, 80, set_estado_actual_app, TRIP_MENU };

PropiedadesBoton botonTripBici   = { 40,  60, 60, 80, nullptr, 0 };
PropiedadesBoton botonTripTren   = { 140, 60, 60, 80, nullptr, 0 };
PropiedadesBoton botonTripViaje  = { 40, 180, 60, 80, nullptr, 0 };
PropiedadesBoton botonTripVolver = { 140, 180, 60, 80, set_estado_actual_app, MENU0 };
PropiedadesBoton botonMenu2 = { 140, 60, 60, 80, set_estado_actual_app, GPS_DATA };
PropiedadesBoton botonMenu3 = { 40, 180, 60, 80, set_estado_actual_app, GPX_SERVER };
PropiedadesBoton botonMenu4 = { 140, 180, 60, 80, set_estado_actual_app, IDLE };

Chrono web_on_off_debounce;


bool estado_sd = false;

// File archivo
// char filename[32] = "/boiola.boot";

void setup() {
  pantalla_init(tft);                                         // inicia la pantalla
  touch.begin();                                              // inicia el touch
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);  // inicia HardwareSerial GPS
  estado_sd = sd_init();                                      // intenta iniciar la sd

  pantalla_setup(tft);
  if (estado_sd) {
    sd_file_boot_record();
    pantalla_img_jpg(tft, TJpgDec);  // imagen
  } else {
    estado_actual_app = ERROR;
  }

  pantalla_icono_server_wifi(tft, web_is_running());
  pantalla_icono_gps(tft, gps);
  pantalla_icono_sd(tft, sd_estado());
}

void loop() {
  // TIENE QUE CORRER TODO EL TIEMPO
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
  sd_guarda_dato_gpx(gps);

  // TOQUES
  toque = touch_pantalla(touch);
  pantalla_xy(tft, toque);
  if (touch_OK(toque.x, toque.y, botonOnOffPantalla)) pantalla_on_off();
  pantalla_auto_off();

  dibujaBoton(botonOnOffPantalla, TFT_GREENYELLOW);

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
      if (pantalla_encendida()) {
        pantalla_fecha_y_hora(tft, gps);
        pantalla_icono_server_wifi(tft, web_is_running());
        pantalla_icono_gps(tft, gps);
        pantalla_icono_sd(tft, sd_estado());

        // dibujaBoton(botonMenu0, TFT_CYAN);
        if (touch_OK(toque.x, toque.y, botonMenu0)) {
          botonMenu0.action(botonMenu0.estado_app);
          pantalla_setup_menu0(tft, trip_is_active());
        }
      }
      break;

    case MENU0:
      pantalla_icono_server_wifi(tft, web_is_running());
      pantalla_icono_gps(tft, gps);
      pantalla_icono_sd(tft, sd_estado());

      // TRIP / FIN TRIP
      if (touch_OK(toque.x, toque.y, botonMenu1)) {
        if (trip_is_active()) {
          trip_terminar();
          sd_close_track();
          set_estado_actual_app(IDLE);
          pantalla_setup(tft);
          pantalla_img_jpg(tft, TJpgDec);
        } else {
          set_estado_actual_app(TRIP_MENU);
          pantalla_setup_trip_menu(tft);
        }
      }

      // GPS DATA
      if (touch_OK(toque.x, toque.y, botonMenu2)) {
        botonMenu2.action(botonMenu2.estado_app);
        tft.fillScreen(TFT_BLACK);
      }

      // GPXSERVER
      if (touch_OK(toque.x, toque.y, botonMenu3)) {
        botonMenu3.action(botonMenu3.estado_app);
        tft.fillScreen(TFT_BLACK);
      }

      // VOLVER
      if (touch_OK(toque.x, toque.y, botonMenu4)) {
        botonMenu4.action(botonMenu4.estado_app);
        pantalla_setup(tft);
        pantalla_img_jpg(tft, TJpgDec);  // imagen
      }
      break;

    case GPS_DATA:
      pantalla_gps(tft, gps, 10);
      // VOLVER
      if (touch_OK(toque.x, toque.y, botonMenu4)) {
        botonMenu4.action(botonMenu4.estado_app);
        pantalla_setup(tft);
        pantalla_img_jpg(tft, TJpgDec);  // imagen
      }
      break;

    case GPX_SERVER:
      if (!web_is_running()) {
        pantalla_icono_server_wifi(tft, web_start());
        tft.setTextSize(3);
        tft.setCursor(10, 50);
        tft.print("Conectar");
        tft.setCursor(10, 75);
        tft.print("SSID:");
        tft.setCursor(10, 100);
        tft.print("BOIOLA-TOOL");
        tft.drawBitmap(
          140,
          180,
          volver,
          64,
          64,
          TFT_BLACK,
          TFT_GREENYELLOW);
      }
      pantalla_fecha_y_hora(tft, gps);
      pantalla_icono_server_wifi(tft, web_is_running());
      pantalla_icono_gps(tft, gps);
      pantalla_icono_sd(tft, sd_estado());


      if (touch_OK(toque.x, toque.y, botonMenu4)) {
        botonMenu4.action(botonMenu4.estado_app);

        web_stop();
        pantalla_setup(tft);
        pantalla_img_jpg(tft, TJpgDec);  // imagen
      }

      break;


    case TRIP_MENU:
      pantalla_icono_server_wifi(tft, web_is_running());
      pantalla_icono_gps(tft, gps);
      pantalla_icono_sd(tft, sd_estado());

      if (touch_OK(toque.x, toque.y, botonTripBici)) {
        trip_iniciar(VIAJE_BICICLETA, gps);
        set_estado_actual_app(TRIP_ACTIVE);
        pantalla_setup(tft);
      }
      if (touch_OK(toque.x, toque.y, botonTripTren)) {
        trip_iniciar(VIAJE_TREN, gps);
        set_estado_actual_app(TRIP_ACTIVE);
        pantalla_setup(tft);
      }
      if (touch_OK(toque.x, toque.y, botonTripViaje)) {
        trip_iniciar(VIAJE_GENERICO, gps);
        set_estado_actual_app(TRIP_ACTIVE);
        pantalla_setup(tft);
      }
      if (touch_OK(toque.x, toque.y, botonTripVolver)) {
        set_estado_actual_app(MENU0);
        pantalla_setup_menu0(tft, false);
      }
      break;

    case TRIP_ACTIVE:
      if (gps.location.isUpdated()) {
        trip_update(gps);
      }
      if (pantalla_encendida()) {
        uint8_t hh, mm, ss;
        trip_get_cronometro(hh, mm, ss);
        char nombre_viaje[32];
        trip_get_track_name(nombre_viaje, sizeof(nombre_viaje));

        pantalla_fecha_y_hora(tft, gps);
        pantalla_icono_server_wifi(tft, web_is_running());
        pantalla_icono_gps(tft, gps);
        pantalla_icono_sd(tft, sd_estado());
        pantalla_trip_activo(tft, hh, mm, ss,
                             trip_get_velocidad_kph(gps),
                             trip_get_distancia_km(),
                             nombre_viaje);

        if (touch_OK(toque.x, toque.y, botonMenu0)) {
          botonMenu0.action(botonMenu0.estado_app);
          pantalla_setup_menu0(tft, true);
        }
      }
      break;
  }
}

void dibujaBoton(PropiedadesBoton boton, uint16_t color) {
  tft.fillRect(boton.x, boton.y, boton.width, boton.height, color);
}




// dibujaBoton(botonMenu1, TFT_CYAN);
// dibujaBoton(botonMenu2, TFT_CYAN);
// dibujaBoton(botonMenu3, TFT_CYAN);
// dibujaBoton(botonMenu4, TFT_CYAN);
//
// DEBUG
// tft.setTextSize(2);
// tft.setCursor(0, 80);
// tft.println(estado_actual_app);
// tft.println(gps.hdop.hdop());
// tft.println(gps.hdop.value());

// pantalla_gps(tft, gps, 120);
