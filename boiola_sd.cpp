#include "boiola_sd.h"

struct TrkptBuffer {
  double lat;
  double lon;
  float ele;
  float speed;
  float course;
  float hdop;
  uint8_t sat;
  uint16_t year;
  uint8_t month, day, hour, minute, second;
  bool valido;
  bool tiene_extensiones;
  bool ele_valido;
  bool speed_valido;
  bool course_valido;
  bool hdop_valido;
  bool sat_valido;
  char name[32];
  char desc[32];
};
#define BUFFER_SIZE 64

struct EstadoGPX {
  bool archivo_abierto;
  bool track_abierto;
  bool segmento_abierto;
  char filename[32];
  char track_name[32];
  TrkptBuffer buffer[BUFFER_SIZE];
  uint8_t buffer_count;
};

static EstadoGPX estado_gpx = { false, false, false, "/boiola.boot", "BOIOLA-TOOL", {}, 0 };
static Chrono sd_gpx_interval(Chrono::SECONDS);
static Chrono sd_flush_interval(Chrono::SECONDS);
static bool estadoSD = false;


bool sd_estado() {
  return estadoSD;
}

bool sd_init() {
  estadoSD = SD.begin(SD_CS);
  return estadoSD;
}

bool sd_file_boot_record() {
  File archivo = SD.open("/boiola.boot", FILE_APPEND, true);
  if (archivo) {
    archivo.print("boot;");
    archivo.close();
    return true;
  }
  return false;
}

bool sd_file_gpx_header() {
  if (estado_gpx.archivo_abierto) return false;

  File archivo = SD.open(estado_gpx.filename, FILE_APPEND, true);
  if (!archivo) return false;

  archivo.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  archivo.println("<gpx version=\"1.1\" creator=\"BOIOLA-TOOL-Logger\"");
  archivo.println("  xmlns=\"http://www.topografix.com/GPX/1/1\"");
  archivo.println("  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"");
  archivo.println("  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1");
  archivo.println("  http://www.topografix.com/GPX/1/1/gpx.xsd\"");
  archivo.println("  xmlns:boiola=\"http://boiola.local/gpx\">");
  archivo.close();

  estado_gpx.archivo_abierto = true;
  return true;
}

bool sd_file_gpx_footer() {
  if (!estado_gpx.archivo_abierto) return false;

  File archivo = SD.open(estado_gpx.filename, FILE_APPEND, true);
  if (!archivo) return false;

  archivo.println("</gpx>");
  archivo.close();

  estado_gpx.archivo_abierto = false;
  return true;
}

bool sd_file_track_open() {
  if (!estado_gpx.archivo_abierto) return false;
  if (estado_gpx.track_abierto) return false;

  File archivo = SD.open(estado_gpx.filename, FILE_APPEND, true);
  if (!archivo) return false;

  archivo.println("  <trk>");
  archivo.print("    <name>");
  archivo.print(estado_gpx.track_name);
  archivo.println("</name>");
  archivo.close();

  estado_gpx.track_abierto = true;
  return true;
}

bool sd_file_track_close() {
  if (!estado_gpx.track_abierto) return false;

  File archivo = SD.open(estado_gpx.filename, FILE_APPEND, true);
  if (!archivo) return false;

  archivo.println("  </trk>");
  archivo.close();

  estado_gpx.track_abierto = false;
  return true;
}

bool sd_file_segment_open() {
  if (!estado_gpx.track_abierto) return false;
  if (estado_gpx.segmento_abierto) return false;

  File archivo = SD.open(estado_gpx.filename, FILE_APPEND, true);
  if (!archivo) return false;

  archivo.println("    <trkseg>");
  archivo.close();

  estado_gpx.segmento_abierto = true;
  return true;
}

bool sd_file_segment_close() {
  if (!estado_gpx.segmento_abierto) return false;

  File archivo = SD.open(estado_gpx.filename, FILE_APPEND, true);
  if (!archivo) return false;

  archivo.println("    </trkseg>");
  archivo.close();

  estado_gpx.segmento_abierto = false;
  return true;
}

// bool sd_file_trackpoint(TinyGPSPlus &gps, const char *name, const char *desc) {
//   if (!estado_gpx.segmento_abierto) return false;
//
//   File archivo = SD.open(estado_gpx.filename, FILE_APPEND, true);
//   if (!archivo) return false;
//
//   if (!gps.location.isValid() || !gps.date.isValid() || !gps.time.isValid()) {
//     archivo.println("    </trkseg>");
//     archivo.print("    <!-- GPS signal lost");
//     if (name && strlen(name) > 0) {
//       archivo.print(" - name: ");
//       archivo.print(name);
//     }
//     if (desc && strlen(desc) > 0) {
//       archivo.print(" - desc: ");
//       archivo.print(desc);
//     }
//     archivo.println(" -->");
//     archivo.println("    <trkseg>");
//     archivo.close();
//     estado_gpx.segmento_abierto = true;
//     return false;
//   }
//
//   archivo.print("      <trkpt lat=\"");
//   archivo.print(gps.location.lat(), 6);
//   archivo.print("\" lon=\"");
//   archivo.print(gps.location.lng(), 6);
//   archivo.println("\">");
//
//   archivo.print("        <time>");
//   archivo.printf("%04d-%02d-%02dT%02d:%02d:%02dZ",
//                  gps.date.year(),
//                  gps.date.month(),
//                  gps.date.day(),
//                  gps.time.hour(),
//                  gps.time.minute(),
//                  gps.time.second());
//   archivo.println("</time>");
//
//   if (gps.altitude.isValid()) {
//     archivo.print("        <ele>");
//     archivo.print(gps.altitude.meters(), 1);
//     archivo.println("</ele>");
//   }
//
//   if (gps.satellites.isValid()) {
//     archivo.print("        <sat>");
//     archivo.print(gps.satellites.value());
//     archivo.println("</sat>");
//   }
//
//   if (gps.hdop.isValid()) {
//     archivo.print("        <hdop>");
//     archivo.print(gps.hdop.hdop(), 1);
//     archivo.println("</hdop>");
//   }
//
//   if (gps.speed.isValid()) {
//     archivo.print("        <speed>");
//     archivo.print(gps.speed.mps(), 2);
//     archivo.println("</speed>");
//   }
//
//   if (gps.course.isValid()) {
//     archivo.print("        <course>");
//     archivo.print(gps.course.deg(), 1);
//     archivo.println("</course>");
//   }
//
//   if ((name && strlen(name) > 0) || (desc && strlen(desc) > 0)) {
//     archivo.println("        <extensions>");
//     if (name && strlen(name) > 0) {
//       archivo.print("          <boiola:name>");
//       archivo.print(name);
//       archivo.println("</boiola:name>");
//     }
//     if (desc && strlen(desc) > 0) {
//       archivo.print("          <boiola:desc>");
//       archivo.print(desc);
//       archivo.println("</boiola:desc>");
//     }
//     archivo.println("        </extensions>");
//   }
//
//   archivo.println("      </trkpt>");
//   archivo.close();
//   return true;
// }

bool sd_set_filename(TinyGPSPlus &gps) {
  if (!gps.date.isValid()) return false;

  char nombreGPS[32];
  snprintf(nombreGPS, 32, "/gpx/%04d-%02d-%02d.gpx",
           gps.date.year(),
           gps.date.month(),
           gps.date.day());

  // primer loop
  if (strcmp(estado_gpx.filename, "/boiola.boot") == 0) {
    snprintf(estado_gpx.filename, sizeof(estado_gpx.filename), "%s", nombreGPS);
    if (!SD.exists(estado_gpx.filename)) {
      // archivo nuevo
      sd_file_gpx_header();
      sd_file_track_open();
      sd_file_segment_open();
    } else {
      // archivo existe — reinicio en el mismo dia
      estado_gpx.archivo_abierto = true;
      estado_gpx.track_abierto = true;
      estado_gpx.segmento_abierto = true;
      File archivo = SD.open(estado_gpx.filename, FILE_APPEND, true);
      if (archivo) {
        archivo.println("    </trkseg>");
        archivo.println("    <!-- device rebooted -->");
        archivo.println("    <trkseg>");
        archivo.close();
      }
    }
    return true;
  }

  // filename ya coincide con la fecha, nada que hacer
  if (strcmp(estado_gpx.filename, nombreGPS) == 0) return false;

  // cambio de dia
  sd_buffer_flush_to_sd();
  sd_file_segment_close();
  sd_file_track_close();
  sd_file_gpx_footer();
  snprintf(estado_gpx.filename, sizeof(estado_gpx.filename), "%s", nombreGPS);
  sd_file_gpx_header();
  sd_file_track_open();
  sd_file_segment_open();
  return true;
}
// bool sd_guarda_dato_gpx(TinyGPSPlus &gps) {
//   if (!sd_gpx_interval.hasPassed(10)) return false;
//   sd_gpx_interval.restart();
//   sd_set_filename(gps);
//   sd_file_trackpoint(gps, nullptr, nullptr);
//   return true;
// }

void sd_set_track_name(const char *name) {
  snprintf(estado_gpx.track_name, sizeof(estado_gpx.track_name), "%s", name);
}

void sd_buffer_trkpt(TinyGPSPlus &gps, const char *name, const char *desc) {
  if (estado_gpx.buffer_count >= BUFFER_SIZE) {
    sd_buffer_flush_to_sd();
    if (estado_gpx.buffer_count >= BUFFER_SIZE) return;  // flush failed, drop point
  }

  TrkptBuffer &punto = estado_gpx.buffer[estado_gpx.buffer_count];

  punto.valido = gps.location.isValid() && gps.date.isValid() && gps.time.isValid();

  if (punto.valido) {
    punto.lat = gps.location.lat();
    punto.lon = gps.location.lng();
    punto.year = gps.date.year();
    punto.month = gps.date.month();
    punto.day = gps.date.day();
    punto.hour = gps.time.hour();
    punto.minute = gps.time.minute();
    punto.second = gps.time.second();

    punto.ele_valido = gps.altitude.isValid();
    punto.ele = gps.altitude.meters();

    punto.speed_valido = gps.speed.isValid();
    punto.speed = gps.speed.mps();

    punto.course_valido = gps.course.isValid();
    punto.course = gps.course.deg();

    punto.hdop_valido = gps.hdop.isValid();
    punto.hdop = gps.hdop.hdop();

    punto.sat_valido = gps.satellites.isValid();
    punto.sat = gps.satellites.value();
  }

  punto.tiene_extensiones = (name && strlen(name) > 0) || (desc && strlen(desc) > 0);

  if (name && strlen(name) > 0)
    snprintf(punto.name, sizeof(punto.name), "%s", name);
  else
    punto.name[0] = '\0';

  if (desc && strlen(desc) > 0)
    snprintf(punto.desc, sizeof(punto.desc), "%s", desc);
  else
    punto.desc[0] = '\0';

  estado_gpx.buffer_count++;
}
bool sd_buffer_flush_to_sd() {
  if (estado_gpx.buffer_count == 0) return false;
  if (!estado_gpx.segmento_abierto) return false;

  File archivo = SD.open(estado_gpx.filename, FILE_APPEND, true);
  if (!archivo) return false;

  for (uint8_t i = 0; i < estado_gpx.buffer_count; i++) {
    TrkptBuffer &punto = estado_gpx.buffer[i];

    if (!punto.valido) {
      archivo.println("    </trkseg>");
      archivo.print("    <!-- GPS signal lost");
      if (strlen(punto.name) > 0) {
        archivo.print(" - name: ");
        archivo.print(punto.name);
      }
      if (strlen(punto.desc) > 0) {
        archivo.print(" - desc: ");
        archivo.print(punto.desc);
      }
      archivo.println(" -->");
      archivo.println("    <trkseg>");
      continue;
    }

    archivo.print("      <trkpt lat=\"");
    archivo.print(punto.lat, 6);
    archivo.print("\" lon=\"");
    archivo.print(punto.lon, 6);
    archivo.println("\">");

    archivo.printf("        <time>%04d-%02d-%02dT%02d:%02d:%02dZ</time>\n",
                   punto.year, punto.month, punto.day,
                   punto.hour, punto.minute, punto.second);

    if (punto.ele_valido) {
      archivo.print("        <ele>");
      archivo.print(punto.ele, 1);
      archivo.println("</ele>");
    }

    if (punto.sat_valido) {
      archivo.print("        <sat>");
      archivo.print(punto.sat);
      archivo.println("</sat>");
    }

    if (punto.hdop_valido) {
      archivo.print("        <hdop>");
      archivo.print(punto.hdop, 1);
      archivo.println("</hdop>");
    }

    if (punto.speed_valido) {
      archivo.print("        <speed>");
      archivo.print(punto.speed, 2);
      archivo.println("</speed>");
    }

    if (punto.course_valido) {
      archivo.print("        <course>");
      archivo.print(punto.course, 1);
      archivo.println("</course>");
    }

    if (punto.tiene_extensiones) {
      archivo.println("        <extensions>");
      if (strlen(punto.name) > 0) {
        archivo.print("          <boiola:name>");
        archivo.print(punto.name);
        archivo.println("</boiola:name>");
      }
      if (strlen(punto.desc) > 0) {
        archivo.print("          <boiola:desc>");
        archivo.print(punto.desc);
        archivo.println("</boiola:desc>");
      }
      archivo.println("        </extensions>");
    }

    archivo.println("      </trkpt>");
  }

  archivo.close();
  estado_gpx.buffer_count = 0;
  return true;
}


bool sd_guarda_dato_gpx(TinyGPSPlus &gps) {
  if (sd_gpx_interval.hasPassed(10)) {
    sd_gpx_interval.restart();
    sd_set_filename(gps);
    sd_buffer_trkpt(gps, nullptr, nullptr);
  }

  if (sd_flush_interval.hasPassed(60)) {
    sd_flush_interval.restart();
    sd_buffer_flush_to_sd();
  }

  return true;
}
