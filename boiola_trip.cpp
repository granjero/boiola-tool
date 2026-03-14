#include "boiola_trip.h"
#include "boiola_sd.h"
#include <math.h>

static EstadoTrip estado_trip = {
  false, VIAJE_NINGUNO, 0, 0.0, 0.0, 0.0, false,
  { 0.0, 1.0, 0.0001, 0.0005 },
  { 0.0, 1.0, 0.0001, 0.0005 },
  ""
};

// ---------- Kalman ----------

double kalman_update(KalmanPos &kf, double medicion) {
  kf.p = kf.p + kf.q;
  double k = kf.p / (kf.p + kf.r);
  kf.x = kf.x + k * (medicion - kf.x);
  kf.p = (1.0 - k) * kf.p;
  return kf.x;
}

// ---------- Haversine ----------

static double haversine_km(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371.0;
  double dlat = (lat2 - lat1) * M_PI / 180.0;
  double dlon = (lon2 - lon1) * M_PI / 180.0;
  double a = sin(dlat / 2.0) * sin(dlat / 2.0)
           + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0)
           * sin(dlon / 2.0) * sin(dlon / 2.0);
  double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
  return R * c;
}

// ---------- Lifecycle ----------

void trip_iniciar(TipoViaje tipo, TinyGPSPlus &gps) {
  estado_trip.activo = true;
  estado_trip.tipo = tipo;
  estado_trip.inicio_ms = millis();
  estado_trip.distancia_km = 0.0;
  estado_trip.tiene_ultima_pos = false;

  // init Kalman filters
  estado_trip.kf_lat = { 0.0, 1.0, 0.0001, 0.0005 };
  estado_trip.kf_lon = { 0.0, 1.0, 0.0001, 0.0005 };

  // seed Kalman from current GPS if available
  if (gps.location.isValid()) {
    estado_trip.kf_lat.x = gps.location.lat();
    estado_trip.kf_lon.x = gps.location.lng();
  }

  // build track name: "BICICLETA 2026-03-14"
  if (gps.date.isValid()) {
    snprintf(estado_trip.track_name, sizeof(estado_trip.track_name),
             "%s %04d-%02d-%02d",
             trip_get_type_string_es(),
             gps.date.year(), gps.date.month(), gps.date.day());
  } else {
    snprintf(estado_trip.track_name, sizeof(estado_trip.track_name),
             "%s", trip_get_type_string_es());
  }

  // close current default track and reopen with trip name
  // (guard functions return false gracefully if file not open yet)
  sd_buffer_flush_to_sd();
  sd_file_segment_close();
  sd_file_track_close();
  sd_set_track_name(estado_trip.track_name);
  sd_set_track_extension(trip_get_type_string());
  sd_file_track_open();
  sd_file_segment_open();
}

void trip_terminar() {
  estado_trip.activo = false;
  estado_trip.tipo = VIAJE_NINGUNO;
  estado_trip.distancia_km = 0.0;
  estado_trip.tiene_ultima_pos = false;
  estado_trip.track_name[0] = '\0';
}

bool trip_is_active() {
  return estado_trip.activo;
}

TipoViaje trip_get_tipo() {
  return estado_trip.tipo;
}

// ---------- Per-loop update ----------

void trip_update(TinyGPSPlus &gps) {
  if (!estado_trip.activo) return;
  if (!gps.location.isValid()) return;

  double lat_f = kalman_update(estado_trip.kf_lat, gps.location.lat());
  double lon_f = kalman_update(estado_trip.kf_lon, gps.location.lng());

  if (estado_trip.tiene_ultima_pos) {
    double d = haversine_km(estado_trip.ultimo_lat, estado_trip.ultimo_lon, lat_f, lon_f);
    estado_trip.distancia_km += d;
  }

  estado_trip.ultimo_lat = lat_f;
  estado_trip.ultimo_lon = lon_f;
  estado_trip.tiene_ultima_pos = true;
}

// ---------- Accessors ----------

void trip_get_cronometro(uint8_t &hh, uint8_t &mm, uint8_t &ss) {
  unsigned long elapsed = millis() - estado_trip.inicio_ms;
  ss = (elapsed / 1000UL) % 60;
  mm = (elapsed / 60000UL) % 60;
  hh = (elapsed / 3600000UL);
}

float trip_get_velocidad_kph(TinyGPSPlus &gps) {
  if (gps.speed.isValid()) return gps.speed.kmph();
  return 0.0f;
}

double trip_get_distancia_km() {
  return estado_trip.distancia_km;
}

void trip_get_track_name(char *buf, size_t len) {
  snprintf(buf, len, "%s", estado_trip.track_name);
}

// "bicycle" / "train" / "trip"
const char *trip_get_type_string() {
  switch (estado_trip.tipo) {
    case VIAJE_BICICLETA: return "bicycle";
    case VIAJE_TREN:      return "train";
    case VIAJE_GENERICO:  return "trip";
    default:              return "";
  }
}

// Spanish display names for track name and screen labels
const char *trip_get_type_string_es() {
  switch (estado_trip.tipo) {
    case VIAJE_BICICLETA: return "BICICLETA";
    case VIAJE_TREN:      return "TREN";
    case VIAJE_GENERICO:  return "VIAJE";
    default:              return "";
  }
}