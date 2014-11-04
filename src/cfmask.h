#ifndef CFMASK_H
#define CFMASK_H

#define CFMASK_APP_NAME "cfmask"
#define CFMASK_VERSION "1.4.2"

typedef signed short int16;

typedef enum {
    BI_BLUE   = 0,
    BI_GREEN  = 1,
    BI_RED    = 2,
    BI_NIR    = 3,
    BI_SWIR_1 = 4,
    BI_SWIR_2 = 5,
    BI_REFL_BAND_COUNT,
    BI_TIR    = 6,
    BI_BAND_COUNT
} BAND_INDEX;

typedef enum
{
    WATER_BIT = 0,
    SHADOW_BIT,
    SNOW_BIT,
    CLOUD_BIT,
    FILL_BIT
} Bits_t;

typedef enum
{
    CLEAR_BIT = 0,
    CLEAR_WATER_BIT,
    CLEAR_LAND_BIT
} Clear_Bits_t;

void usage ();

#endif
