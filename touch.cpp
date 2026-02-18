#include "touch.h"

Chrono touch_debounce;

TouchPoint touch_pantalla(XPT2046_Bitbang &touch) {
  TouchPoint toque = touch.getTouch();

  int16_t x = toque.x;
  int16_t y = toque.y;

  toque.x = y;
  toque.y = 320 - x;

  if (touch_debounce.hasPassed(500) && toque.zRaw >= 2000) {
    touch_debounce.restart();
    return toque;
  }
  toque.zRaw = 0;
  return toque;
}

bool touch_OK(int touchX, int touchY, const PropiedadesBoton &region) {
  return (touchX >= region.x && touchX < (region.x + region.width) && touchY >= region.y && touchY < (region.y + region.height));
}
