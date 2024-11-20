#ifndef WU_H
#define WU_H

#include <stdint.h>

void guess_palette_wu(uint8_t *image, int w, int h, int c, uint32_t *palette,
                      int palette_size);

#endif
