# Set up compile options
CC	= gcc
RM	= rm -f
MV	= mv
EXTRA = -g -Wall 

# Define the include files
INC = const.h date.h error.h mystring.h input.h myhdf.h 2d_array.h \
      cfmask.h output.h space.h myproj.h myproj_const.h 
INCDIR  = -I. -I$(JPEGINC) -I$(HDFINC) -I$(HDFEOS_INC) -I$(HDFEOS_GCTPINC) -I$(OPENCVINC)  
NCFLAGS = $(EXTRA) $(INCDIR)

# Define the source code and object files
SRC = date.c error.c input.c myhdf.c mystring.c 2d_array.c cfmask.c \
      potential_cloud_shadow_snow_mask.c \
      write_envi_hdr.c split_filename.c \
      object_cloud_shadow_match.c misc.c output.c space.c 
OBJ = $(SRC:.c=.o)

# Define the object libraries
LIB   = -L$(HDFLIB) -lmfhdf -ldf -L$(JPEGLIB) -ljpeg -lz \
	-lxdr -lm -lstdc++
EOSLIB = -L$(HDFEOS_LIB) -lhdfeos -L$(HDFEOS_GCTPLIB) -lGctp

CVLIB = -L$(OPENCVLIB)  -lopencv_core -lopencv_imgproc \
	-lopencv_calib3d -lopencv_contrib -lopencv_features2d -lopencv_flann \
	-lopencv_gpu -lopencv_highgui -lopencv_legacy -lopencv_ml \
	-lopencv_nonfree -lopencv_objdetect -lopencv_photo -lopencv_ts 

# Define the executable
EXE = cfmask

# Target for the executable
all: $(EXE)

cfmask: $(OBJ) $(INC)
	$(CC) -o cfmask $(OBJ) $(EOSLIB) $(CVLIB) $(LIB) 

install:
	cp $(EXE) $(BIN)
	cp ../scripts/run_fillminima.py $(BIN)
	cp ../scripts/fillminima.py $(BIN)

clean:
	$(RM) *.o $(EXE)

$(OBJ): $(INC)

.c.o:
	$(CC) $(NCFLAGS) $(INCDIR) -c $<

