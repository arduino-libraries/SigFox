
#define UINT16_t_MAX  ((2 << 16)-1)
#define INT16_t_MAX  ((2 << 15)-1)

int16_t convertoFloadToInt16(float value, int max, int min) {
  float conversionFactor = (float) (INT16_t_MAX) / (float)(max - min);
  return (int16_t)(value * conversionFactor);
}

uint16_t convertoFloadToUInt16(float value, int max, int min = 0) {
  float conversionFactor = (float) (UINT16_t_MAX) / (float)(max - min);
  return (uint16_t)(value * conversionFactor);
}
