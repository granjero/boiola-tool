#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include <XPT2046_Bitbang.h>
#include <Chrono.h>
// #include <TFT_eSPI.h>

// Screen dimensions for CYD (320x240)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Touch region structure
struct PropiedadesBoton {
  int x;
  int y;
  int width;
  int height;
  void (*action)(int);
  int estado_app;
};

TouchPoint touch_pantalla(XPT2046_Bitbang &touch);

bool touch_OK(int touchX, int touchY, const PropiedadesBoton &region);
// };

#endif
