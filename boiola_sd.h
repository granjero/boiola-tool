#ifndef BOIOLA_SD_H
#define BOIOLA_SD_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <TinyGPSPlus.h>

#define SD_CS 5

bool sd_estado();
bool sd_init();
// setea filename a los valores de la fecha
bool sd_set_filename(TinyGPSPlus &gps, char *filename, size_t maxLen);
// chequea si el archivo existe
// bool sd_filename_existe(File &archivo, char *filename);

//escribe el encabbezado del archivo gpx
bool sd_file_encabezado(char *filename);
// cierra el archivo gpx
bool sd_file_footer(char *filename);
// escribe un dato en el archivo
bool sd_file_append_gps_point(char *filename, TinyGPSPlus &gps);

#endif
