#ifndef CFMASK_H
#define CFMASK_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "error.h"
#include "espa_metadata.h"
#include "parse_metadata.h"
#include "write_metadata.h"
#include "envi_header.h"
#include "espa_geoloc.h"
#include "raw_binary_io.h"

#define CFMASK_VERSION "1.2.1"

typedef signed short int16;

/* There are currently a maximum of 6 reflective bands in the output surface
   reflectance product */
#define NBAND_REFL_MAX 6

#define MAX_STR_LEN (510)

#endif
