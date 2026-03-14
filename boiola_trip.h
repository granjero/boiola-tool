#ifndef BOIOLA_TRIP_H
#define BOIOLA_TRIP_H

#include <Arduino.h>
#include <TinyGPSPlus.h>

enum TipoViaje {
  VIAJE_NINGUNO = 0,
  VIAJE_BICICLETA,
  VIAJE_TREN,
  VIAJE_GENERICO
};

struct KalmanPos {
  double x;  // estimated value
  double p;  // error covariance
  double q;  // process noise
  double r;  // measurement noise
};

struct EstadoTrip {
  bool activo;
  TipoViaje tipo;
  unsigned long inicio_ms;
  double distancia_km;
  double ultimo_lat;
  double ultimo_lon;
  bool tiene_ultima_pos;
  KalmanPos kf_lat;
  KalmanPos kf_lon;
  char track_name[32];
};

void   trip_iniciar(TipoViaje tipo, TinyGPSPlus &gps);
void   trip_terminar();
bool   trip_is_active();
TipoViaje trip_get_tipo();
void   trip_update(TinyGPSPlus &gps);
void   trip_get_cronometro(uint8_t &hh, uint8_t &mm, uint8_t &ss);
float  trip_get_velocidad_kph(TinyGPSPlus &gps);
double trip_get_distancia_km();
void   trip_get_track_name(char *buf, size_t len);
const char *trip_get_type_string();

double kalman_update(KalmanPos &kf, double medicion);
const char *trip_get_type_string_es();

#endif