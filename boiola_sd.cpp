#include <cstddef>
#include "boiola_sd.h"

Chrono sd_gpx_interval(Chrono::SECONDS);

static bool estadoSD = false;

bool sd_estado() {
  return estadoSD;
}


bool sd_init() {
  estadoSD = SD.begin(SD_CS);
  return estadoSD;
}

bool sd_boot_record(char *filename) {
  File archivo = sd_open_file(filename);
  if (archivo) {
    archivo.print("boot;");
    archivo.close();
    return true;
  }
  return false;
}

File sd_open_file(char *filename) {
  return SD.open(filename, FILE_APPEND, true);
}

bool sd_set_filename(TinyGPSPlus &gps, char *filename, size_t maxLen) {
  if (!gps.date.isValid()) return false;

  char nombreGPS[32];
  snprintf(nombreGPS, 32, "/gpx/%04d-%02d-%02d.gpx",
           gps.date.year(),
           gps.date.month(),
           gps.date.day());

  // si filename == /boiola.boot es el primer loop y seteo el nombre con la fecha
  if (strcmp(filename, "/boiola.boot") == 0) {
    snprintf(filename, maxLen, "%s", nombreGPS);
    // TO DO aca creo que hay que chequear si hay archivo con nombre gps y si no encabezarlo
    return true;
  }
  // si filename es igual a la fecha ya esta seteado y no hago nada
  else if (strcmp(filename, nombreGPS) == 0)
    return false;
  // si filename es distinto a boiola.boot y a la fecha hay que actualizar el nombre
  else {
    sd_file_footer(filename);
    snprintf(filename, maxLen, "%s", nombreGPS);
    sd_file_encabezado(filename);
    return true;
  }
}


bool sd_guarda_dato_gpx(TinyGPSPlus &gps, char *filename, size_t buffersize) {
  if (sd_gpx_interval.hasPassed(10)) {
    sd_gpx_interval.restart();
    sd_set_filename(gps, filename, buffersize);
    // if (!SD.exists(filename)) {
    //   sd_file_encabezado(filename);
    // } else {
    sd_file_append_gps_point(gps, filename);
    // }
    return true;
  } else return false;
}

bool sd_file_encabezado(char *filename) {
  File archivo = SD.open(filename, FILE_APPEND, true);
  if (!archivo) return false;
  archivo.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  archivo.println("<gpx version=\"1.1\" creator=\"BOIOLA-TOOL-Logger\"");
  archivo.println("  xmlns=\"http://www.topografix.com/GPX/1/1\"");
  archivo.println("  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"");
  archivo.println("  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1");
  archivo.println("  http://www.topografix.com/GPX/1/1/gpx.xsd\">");
  archivo.println("  <trk>");
  archivo.println("    <name>BOIOLA-TOOL</name>");
  archivo.println("    <trkseg>");
  archivo.close();
  return true;
}


bool sd_file_footer(char *filename) {
  // File archivo = SD.open(filename, FILE_APPEND, true);
  File archivo = sd_open_file(filename);
  if (!archivo) return false;
  archivo.println("    </trkseg>");
  archivo.println("  </trk>");
  archivo.println("</gpx>");
  archivo.close();
  return true;
}



bool sd_file_append_gps_point(TinyGPSPlus &gps, char *filename) {
  // File archivo = SD.open(filename, FILE_APPEND, true);
  File archivo = sd_open_file(filename);
  if (!gps.location.isValid() || !gps.date.isValid() || !gps.time.isValid()) {
    archivo.println("    </trkseg>");
    archivo.println("    <!-- GPS signal lost -->");
    archivo.println("    <trkseg>");
    archivo.close();
    return false;
  } else {
    // File archivo = SD.open(filename, FILE_APPEND, true);
    // Start track point with lat/lon
    archivo.print("      <trkpt lat=\"");
    archivo.print(gps.location.lat(), 6);  // 6 decimal places
    archivo.print("\" lon=\"");
    archivo.print(gps.location.lng(), 6);
    archivo.println("\">");

    // Timestamp (ISO 8601 format)
    archivo.print("        <time>");
    archivo.printf("%04d-%02d-%02dT%02d:%02d:%02dZ",
                   gps.date.year(),
                   gps.date.month(),
                   gps.date.day(),
                   gps.time.hour(),
                   gps.time.minute(),
                   gps.time.second());
    archivo.println("</time>");

    // Elevation (if valid)
    if (gps.altitude.isValid()) {
      archivo.print("        <ele>");
      archivo.print(gps.altitude.meters(), 1);
      archivo.println("</ele>");
    }


    // Optional: Number of satellites
    if (gps.satellites.isValid()) {
      archivo.print("        <sat>");
      archivo.print(gps.satellites.value());
      archivo.println("</sat>");
    }

    // Optional: HDOP (accuracy)
    if (gps.hdop.isValid()) {
      archivo.print("        <hdop>");
      archivo.print(gps.hdop.hdop(), 1);
      archivo.println("</hdop>");
    }

    // Optional: Speed
    if (gps.speed.isValid()) {
      archivo.print("        <speed>");
      archivo.print(gps.speed.mps(), 2);  // meters per second
      archivo.println("</speed>");
    }

    // Optional: Course/heading
    if (gps.course.isValid()) {
      archivo.print("        <course>");
      archivo.print(gps.course.deg(), 1);
      archivo.println("</course>");
    }

    // Close track point
    archivo.println("      </trkpt>");
    archivo.close();

    return true;
  }
}
