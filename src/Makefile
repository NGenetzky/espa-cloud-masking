# Set up compile options
CC    = gcc
RM    = rm -f
MV    = mv
EXTRA = -Wall -g

# Define the include files
INC = const.h date.h error.h mystring.h input.h myhdf.h 2d_array.h \
      cfmask.h output.h space.h myproj.h myproj_const.h
#INCDIR  = -I. -I$(JPEGINC) -I$(HDFEOS_INC) -I$(HDFEOS_GCTPINC) \
#          -I$(HDFINC)
INCDIR  = -I. -I$(JPEGINC) -I$(HDFEOS_INC) -I$(HDFEOS_GCTPINC) -I$(HDFINC)
NCFLAGS = $(EXTRA) $(INCDIR)

# Define the source code and object files
SRC = date.c error.c input.c myhdf.c mystring.c 2d_array.c cfmask.c \
      potential_cloud_shadow_snow_mask.c \
      write_envi_hdr.c split_filename.c \
      object_cloud_shadow_match.c misc.c output.c space.c
OBJ = $(SRC:.c=.o)

# Define the object libraries
LIB   = -L$(HDFLIB) -lmfhdf -ldf -L$(JPEGLIB) -ljpeg -lz -lpthread -lrt \
        -lstdc++ -lm
EOSLIB = -L$(HDFEOS_LIB) -lhdfeos -L$(HDFEOS_GCTPLIB) -lGctp

# Define the executable
EXE = cfmask

# Target for the executable
all: $(EXE)

cfmask: $(OBJ) $(INC)
#	$(CC) $(NCFLAGS) -o cfmask $(OBJ) $(CVLIB) $(EOSLIB) $(LIB)
	$(CC) $(NCFLAGS) -o cfmask $(OBJ) $(EOSLIB) $(LIB)

install:
	cp $(EXE) $(BIN)
	cp ../scripts/run_fillminima.py $(BIN)
	cp ../scripts/fillminima.py $(BIN)

clean:
	$(RM) *.o $(EXE)

$(OBJ): $(INC)

.c.o:
	$(CC) $(NCFLAGS) $(INCDIR) -c $<

