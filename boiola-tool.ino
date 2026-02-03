#include <Arduino.h>
#include <HardwareSerial.h>

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

bool estado_pantalla = true;
bool estado_sd = false;

String dataBuffer;
File myFile;
const char filename[] = "/boiolaTool.boot";

void setup() {
  pantalla_init(tft);                                         // inicia la pantalla
  pantalla_setup(tft, TFT_PURPLE);                            // setup borde
  pantalla_bandera(tft, 0, 0, 5);                             // bandera
  touch.begin();                                              // inicia el touch
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);  // HardwareSerial GPS

  estado_sd = sd_init();  // intenta iniciar la sd
  pantalla_icono_sd(tft, estado_sd);

  if (estado_sd) {
    myFile = SD.open(filename, FILE_APPEND, true);
    if (!myFile) {
      tft.setCursor(10, 100);
      tft.println("error opening: ");
      tft.println(filename);
      // while (true);
    }
    myFile.print("boot;");
    myFile.close();
  }
}


void loop() {

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated()) {
    pantalla_fecha(tft, gps);
    pantalla_gps(tft, gps, 60);

    if (estado_sd) {
      myFile = SD.open(filename, FILE_APPEND, true);
      char buf[64];
      snprintf(buf, sizeof(buf),
               "%02d:%02d:%02d,%.6f,%.6f\n",
               gps.time.hour(),
               gps.time.minute(),
               gps.time.second(),
               gps.location.lat(),
               gps.location.lng());
      myFile.print(buf);
      myFile.close();
    }
  }

  pantalla_touch(tft, touch);
}
