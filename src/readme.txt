Installation Guide 

1. Install dependent libraries - HDF4 and openCV 

2. Set uo the environment variables:
HDFINC="path_to_HDF4_include_files"
HDFLIB="path_to_HDF4_libraries"
OPENCVLIB="path_to_openCV_include_files"
OPENCVINC="path_to_openCV_libraries"

3. Check out the source code and compile the file:
svn co svn://l8srlscp01.cr.usgs.gov/espa/fmask/trunk
make

4. Run Fmask:
./cfmask <directory holding LEDAPS outputs>/<ledaps output metadata file> 

Note:
1. Right now, the Fmask code uses LEDAPS output files (metadata file and lndcal
and lndth files), it uses LEDAPS output metadata file as command line input
argument.
2. It's better to use the newly updated ESUN, K1/K2 and earth_sun_distance in
the digital number (DN) to TOA reflectance and brightness temperature (BT)
conversion in the LEDAPS lndcal code. But if old constant values are used, the
result differences are minor because Fmask code has taken care of the saturated
pixel conversion in its processing. 
3. If new constant values are needed in the DN to TOA reflectance and BT 
conversion, then the updated new lndcal code can be checked out from:
svn co svn://l8srlscp01.cr.usgs.gov/espa/fmask/lndcal2  
4. If the new lndcal needs to be run, just run the original LEDAPS lndpm first,
and then run lndcal the second, all the needed input files to Fmask will be
generated:
a. cd to the directory where original landsat data and *MTL* data are available.
b. $BIN/lndpm <Landsat_meta_file>
c. $BIN/lndcal <lndcal_input_text>
5. Now in the standard LEDAPS code, the saturated pixel values are set to be 
20000 for all bands. 20000 is saved for all satu_value_ref values for each band.
Now in the Fmask, the satu_value_max is calculated based on the same DN to TOA
reflectance and DN to BT conversions when DN is 255. In Fmask, when any pixel 
value is 20000, it is replaced with the satu_value_max of the band calculated 
as above. Therefore, the actual satu_value_max values are used for each 
saturated pixle instead of 20000 in Fmask. The lndcal2 code above actually set
saturated pixel values as satu_value_max values calculated above. Then the 
saturated pixel value replacement may not be needed in Fmask code, but it 
doesn't hurt to keep the replacement code in Fmask as it will simply do nothing
if the equivalent saturated TOA reflectance and BT values are saved in LEDAPS
output files (therefore the input files for Fmask) instead of 20000 is saved. 
Our long-term goal is simply use LEDAPS output files as inputs to the Fmask 
after all constant updates are taken in place. 

