#ifndef CFMASK_H
#define CFMASK_H

#define CFMASK_APP_NAME "l8cfmask"
#define CFMASK_VERSION "1.0.0"

typedef signed short int16;

typedef enum {
    BI_BLUE   = 0,
    BI_GREEN  = 1,
    BI_RED    = 2,
    BI_NIR    = 3,
    BI_SWIR_1 = 4,
    BI_SWIR_2 = 5,
    BI_CIRRUS = 6,
    BI_REFL_BAND_COUNT,
    BI_TIR    = 7,
    BI_BAND_COUNT
} BAND_INDEX;

void usage ();

#endif
