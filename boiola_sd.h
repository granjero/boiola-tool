#include <cstddef>
#ifndef BOIOLA_SD_H
#define BOIOLA_SD_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <Chrono.h>

#define SD_CS 5


bool sd_estado();
bool sd_init();
bool sd_boot_record(char *filename);
File sd_open_file(char *filename);
// setea filename a los valores de la fecha
bool sd_set_filename(TinyGPSPlus &gps, char *filename, size_t maxLen);
// escribe el dato gpx
bool sd_guarda_dato_gpx(TinyGPSPlus &gpx, char *filename, size_t buffersize);
//escribe el encabbezado del archivo gpx
bool sd_file_encabezado(char *filename);
// cierra el archivo gpx
bool sd_file_footer(char *filename);
// escribe un dato en el archivo
bool sd_file_append_gps_point(TinyGPSPlus &gps, char *filename);

#endif
