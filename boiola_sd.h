#include <cstddef>
#ifndef BOIOLA_SD_H
#define BOIOLA_SD_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <Chrono.h>

#define SD_CS 5


// --- Init ---
bool sd_estado();
bool sd_init();
bool sd_file_boot_record();

// --- File layer ---
bool sd_file_gpx_header();
bool sd_file_gpx_footer();

// --- Track layer ---
bool sd_file_track_open();
bool sd_file_track_close();

// --- Segment layer ---
bool sd_file_segment_open();
bool sd_file_segment_close();

// --- Data writers ---
// bool sd_file_trackpoint(TinyGPSPlus &gps, const char *name, const char *desc);

// --- Orchestration ---
bool sd_set_filename(TinyGPSPlus &gps);
bool sd_guarda_dato_gpx(TinyGPSPlus &gps);

// --- State setters (called from CYD UI) ---
void sd_set_track_name(const char *name);

// --- Buffer ---
void sd_buffer_trkpt(TinyGPSPlus &gps, const char *name, const char *desc);
bool sd_buffer_flush_to_sd();


#endif
