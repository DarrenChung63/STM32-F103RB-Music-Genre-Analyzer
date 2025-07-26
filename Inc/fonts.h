#ifndef FONTS_H_
#define FONTS_H_

#include <stdint.h>

typedef struct {
    uint8_t FontWidth;    // Font width in pixels
    uint8_t FontHeight;   // Font height in pixels
    const uint8_t *data;  // Pointer to font data (vertical bytes)
} FontDef_t;

extern FontDef_t Font_7x10;
extern const uint8_t Font_7x10_data[];

#endif /* FONTS_H_ */
