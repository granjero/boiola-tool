#include "boiola_sd.h"

bool sd_init() {
  return SD.begin(SD_CS);
}


bool sd_set_filename(TinyGPSPlus &gps, char *filename, size_t maxLen) {
  if (!gps.date.isValid()) return false;

  char nombreGPS[32];
  snprintf(nombreGPS, 32, "/gpx/%04d-%02d-%02d.gpx",
    gps.date.year(), // tengo el nombre del gps
    gps.date.month(), // ahora lo tengo que buscar en el sd
    gps.date.day()); //  si no existe  tengo que cerrar el anterior
      		// que es filename hasta el proximo snprintf
  	
  // if (filename == "/boiola.boot") {
  if (strcmp(filename, "/boiola.boot") == 0) {
    snprintf(filename, maxLen, "%s", nombreGPS);
    return true;
  }

  if (strcmp(filename, nombreGPS) == 0) return false; // ya está seteado el nombre de archivo correcto

  sd_file_footer(filename);
  snprintf(filename, maxLen, "%s", nombreGPS);
  return true;
  
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
  File archivo = SD.open(filename, FILE_APPEND, true);
  if (!archivo) return false;
  archivo.println("    </trkseg>");
  archivo.println("  </trk>");
  archivo.println("</gpx>");
  archivo.close();
  return true;
}



bool sd_file_append_gps_point(char *filename, TinyGPSPlus &gps) {
  if (!gps.location.isValid() || !gps.date.isValid() || !gps.time.isValid()) {
    File archivo = SD.open(filename, FILE_APPEND, true);
    archivo.println("    </trkseg>");
    archivo.println("    <!-- GPS signal lost -->");
    archivo.println("    <trkseg>");
    archivo.close();
    return false;
  }

  File archivo = SD.open(filename, FILE_APPEND, true);
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
