## l8_cfmask Version 0.3.0 Release Notes

See git tag [l8_cfmask-version_0.3.0]

This application produces Cloud Mask products for Landsat 8 based on the
CFMASK (Function of Mask Algorithm).

## Product Descriptions
See TODO TODO TODO.

## Detailed Algorithm
See TODO TODO TODO.

## Release Notes
* Added --version option to the command line
* Updated some command line options in the usage to be consistent with orders
* Fixed so that the --help option exits successfully instead of indicating a failure
* Other minor changes to comments and logging output
* Changes to the location and installation of the EarthSunDistance.txt file
* TODO TODO TODO - Additional changes are in development

## Installation

### Dependencies
* ESPA raw binary libraries, tools, and it's dependencies, found here [espa-product-formatter](https://github.com/USGS-EROS/espa-product-formatter)
* Python 2.7 and Scipy

### Environment Variables
* Required for building this software
```
export PREFIX="path_to_Installation_Directory"
export XML2INC="path_to_LIBXML2_include_files"
export XML2LIB="path_to_LIBXML2_libraries"
export ESPAINC="path_to_ESPA_PRODUCT_FORMATTER_include_files"
export ESPALIB="path_to_ESPA_PRODUCT_FORMATTER_libraries"
```

### Build Steps
* Clone the repository and replace the defaulted version(master) with this
  version of the software
```
git clone https://github.com/USGS-EROS/espa-cloud-masking.git
cd espa-cloud-masking
git checkout l8_cfmask-version_<version>
```
* Build and install the software from the application specific folder
```
cd not-validated-prototype-l8_cfmask
make
make install
```

## Usage
See `l8cfmask --help` for command line details.

### Environment Variables
* ESUN - Points to the EarthSunDistance.txt file which is included with the source and installed into $PREFIX/static_data
```
export ESUN="path_to_EarthSunDistance.txt_file"
```

### Data Processing Requirements
This version of the CFMASK application requires the input products to be in the ESPA internal file format.

The following input data are required to generate the cloud masking products:
* Top of Atmosphere Reflectance
* Brightness Temperature

These products can be generated using the [L8_SR](https://github.com/USGS-EROS/espa-surface-reflectance) software found in our [espa-surface-reflectance](https://github.com/USGS-EROS/espa-surface-reflectance) project.  Or through our ondemand processing system [ESPA](https://espa.cr.usgs.gov), be sure to select the ENVI output format.

This cloud masking product is currently available in the [ESPA](https://espa.cr.usgs.gov) processing system as part of the Surface Reflectance product.

## More Information
This project is provided by the US Geological Survey (USGS) Earth Resources
Observation and Science (EROS) Land Satellite Data Systems (LSDS) Science
Research and Development (LSRD) Project. For questions regarding products
produced by this source code, please contact the Landsat Contact Us page and
specify USGS CDR/ECV in the "Regarding" section.
https://landsat.usgs.gov/contactus.php
