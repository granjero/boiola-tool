#include "boiola_sd.h"

bool sd_init() {
  return SD.begin(SD_CS);
}
