#ifndef BOIOLA_WEB_H
#define BOIOLA_WEB_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <SD.h>

#define SD_CS 5

// WiFi AP Configuration
#define AP_SSID "BOIOLA-TOOL"
#define AP_PASS "BOIOLA-TOOL"  // Change this!
#define AP_CHANNEL 1
#define AP_HIDDEN false
#define AP_MAX_CONNECTIONS 4

// DNS Configuration
#define DNS_PORT 53

// Function declarations
bool web_init();
bool web_start();
void web_stop();
bool web_is_running();
void web_set_filename(const char* filename);
void web_setup_routes();
String web_generate_file_list();
void web_stop();

#endif
