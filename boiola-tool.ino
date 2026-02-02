#include <HardwareSerial.h>
#include <SD.h>

#define TOUCH_CLK 25
#define TOUCH_MOSI 32
#define TOUCH_CS 33
#define TOUCH_IRQ 36
#define TOUCH_MISO 39
#define PANTALLA_PW_PIN 21

#define GPS_RX_PIN 27
#define GPS_TX_PIN 22

#include "pantalla.h"
#include "icons.h"

XPT2046_Bitbang touch(TOUCH_MOSI, TOUCH_MISO, TOUCH_CLK, TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);


bool estado_pantalla = true;
bool estado_sd = false;

String dataBuffer;
File myFile;
const char filename[] = "/foo.txt";

void setup() {
  // SPIClass spi = SPIClass(VSPI);
  pantalla_init(tft);  // inicia la pantalla

  pantalla_bandera(tft, 30, 30, 5);
  pantalla_bandera(tft, 50, 50, 5);
  pantalla_bandera(tft, 70, 70, 5);
  pantalla_bandera(tft, 0, 0, 5);

  delay(1000);
  pantalla_setup(tft, TFT_PURPLE);

  pantalla_bandera(tft, 0, 0, 5);

  touch.begin();  // inicia el touch

  if (SD.begin(SS)) {
    tft.drawBitmap(208, 0, sd_icon, 32, 32, TFT_BLACK, TFT_GREEN);
  } else {
    tft.drawBitmap(208, 0, sd_icon, 32, 32, TFT_BLACK, TFT_RED);
  }
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);


  dataBuffer.reserve(1024);
  myFile = SD.open(filename, FILE_WRITE);
  if (!myFile) {
    tft.setCursor(10, 100);
    tft.println("error opening: ");
    tft.println(filename);
    while (true);
  }
  myFile.println();
  myFile.println("Hello World!");
  myFile.close();
}


void loop() {

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated()) {
    pantalla_fecha(tft, gps);
    pantalla_gps(tft, gps, 60);

    myFile = SD.open(filename, FILE_APPEND);
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

  pantalla_touch(tft, touch);
}



// void writeFile(fs::FS &fs, const char *path, const char *message) {
//   Serial.printf("Writing file: %s\n", path);
//
//   File file = fs.open(path, FILE_WRITE);
//   if (!file) {
//     Serial.println("Failed to open file for writing");
//     return;
//   }
//   if (file.print(message)) {
//     Serial.println("File written");
//   } else {
//     Serial.println("Write failed");
//   }
//   file.close();
// }
//
// void appendFile(fs::FS &fs, const char *path, const char *message) {
//   // Serial.printf("Appending to file: %s\n", path);
//
//   File file = fs.open(path, FILE_APPEND);
//   if (!file) {
//     // Serial.println("Failed to open file for appending");
//     return;
//   }
//   if (file.print(message)) {
//     // Serial.println("Message appended");
//   } else {
//     // Serial.println("Append failed");
//   }
//   file.close();
// }
