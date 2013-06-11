# Set the HDF4 environment variables
HDFINC=/home/gschmidt/hdf-4.2.5/include
HDFLIB=/home/gschmidt/hdf-4.2.5/lib

# Set the openCV environment variables
OPENCVLIB=/opt/cots/opencv-2.4.3/lib
OPENCVINC=/opt/cots/opencv-2.4.3/include/opencv

# Set up compile options
CC	= gcc
RM	= rm -f
MV	= mv
EXTRA = -g -Wall 

# Define the include files
INC = const.h date.h error.h mystring.h input.h myhdf.h ias_misc_2d_array.h ias_const.h ias_logging.h ias_types.h cfmask.h output.h
INCDIR  = -I. -I$(HDFINC) -I$(OPENCVINC)  
NCFLAGS = $(EXTRA) $(INCDIR)

# Define the source code and object files
SRC = date.c error.c input.c myhdf.c mystring.c ias_misc_2d_array.c cfmask.c ias_logging.c potential_cloud_shadow_snow_mask.c ias_misc_write_envi_header.c ias_misc_split_filename.c object_cloud_shadow_match.c ias_misc.c output.c
OBJ = $(SRC:.c=.o)

# Define the object libraries
LIB   = -L$(HDFLIB) -lmfhdf -ldf \
	-lxdr -lm -lstdc++
EOSLIB = -L$(HDFEOS_LIB) -lhdfeos 

CVLIB = -L$(OPENCVLIB)  -lopencv_core -lopencv_imgproc \
	-lopencv_calib3d -lopencv_contrib -lopencv_features2d -lopencv_flann \
	-lopencv_gpu -lopencv_highgui -lopencv_legacy -lopencv_ml \
	-lopencv_nonfree -lopencv_objdetect -lopencv_photo -lopencv_ts 

# Define the executable
EXE = cfmask

# Target for the executable
all: $(EXE)

cfmask: $(OBJ) $(INC)
	$(CC) -o cfmask $(OBJ) $(CVLIB) $(LIB) $(EOSLIB) 

install:
#	$(MV) $(EXE) $(BINDIR)/

clean:
	$(RM) *.o

$(OBJ): $(INC)

.c.o:
	$(CC) $(NCFLAGS) $(INCDIR) -c $<

