#------------------------------------------------------------------------------ # Makefile
#
# For building L8 cfmask.
#------------------------------------------------------------------------------

# Set up compile options
CC    = gcc
RM    = rm -f
EXTRA = -Wall -g -O2

# Define the include files
INC = const.h date.h error.h input.h 2d_array.h cfmask.h output.h
INCDIR  = -I. -I$(XML2INC) -I$(ESPAINC)
NCFLAGS = $(EXTRA) $(INCDIR)

# Define the source code and object files
SRC = \
      misc.c                             \
      2d_array.c                         \
      date.c                             \
      split_filename.c                   \
      error.c                            \
      input.c                            \
      output.c                           \
      potential_cloud_shadow_snow_mask.c \
      object_cloud_shadow_match.c        \
      cfmask.c
OBJ = $(SRC:.c=.o)

# Define the object libraries
EXLIB = -L$(ESPALIB) -l_espa_raw_binary -l_espa_common \
        -l_espa_format_conversion -L$(XML2LIB) -lxml2 -L$(LZMALIB) \
        -lz -lpthread -lrt
MATHLIB = -lm
LOADLIB = $(EXLIB) $(MATHLIB)

# Define the executable
EXE = l8cfmask

# Target for the executable
all: $(EXE)

$(EXE): $(OBJ) $(INC)
	$(CC) $(EXTRA) -o $(EXE) $(OBJ) $(LOADLIB)

install:
	install -d $(PREFIX)/bin
	install -m 755 $(EXE) $(PREFIX)/bin

clean:
	$(RM) *.o $(EXE)

$(OBJ): $(INC)

.c.o:
	$(CC) $(NCFLAGS) -c $<

