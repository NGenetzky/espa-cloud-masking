## l4-7_cfmask Version 1.6.0

This application produces Cloud Mask products for Landsats 4, 5, and 7 based
on the CFMASK (Function of Mask Algorithm).

## Product Descriptions
See TODO TODO TODO.

## Detailed Algorithm
See TODO TODO TODO.

## Release Notes
* Added --version option to the command line
* Updated some command line options in the usage to be consistent with orders
* Fixed so that the --help option exits successfully instead of indicating a failure
* Other minor changes to comments and logging output
* TODO TODO TODO - Additional changes are in development

## Installation

### Dependencies
* ESPA raw binary libraries, tools, and it's dependencies, found here [espa-product-formatter](https://github.com/USGS-EROS/espa-product-formatter)
* Python 2.7 and Scipy

### Environment Variables
* Required for building this software
```
export XML2INC="path_to_LIBXML2_include_files"
export XML2LIB="path_to_LIBXML2_libraries"
export ESPAINC="path_to_ESPA_PRODUCT_FORMATTER_include_files"
export ESPALIB="path_to_ESPA_PRODUCT_FORMATTER_libraries"
```
* Required for execution of this application
```
export ESUN="path_to_EarthSunDistance.txt_file" # included and installed with this source
```

### Build Steps
* Clone the repository
```
git clone https://github.com/USGS-EROS/espa-cloud-masking.git
```
* Change to the cloned repository folder
```
cd espa-cloud-masking
```
* Replace the defaulted version(master) with this version of the software
```
git checkout l4-7_cfmask-version_<version>
```
* Change to the application folder
```
cd l4-7_cfmask
```
* Build and install the software
```
make
make install
```

## Usage
See `cfmask --help` for command line details.

## More Information
This project is provided by the US Geological Survey (USGS) Earth Resources
Observation and Science (EROS) Land Satellite Data Systems (LSDS) Science
Research and Development (LSRD) Project. For questions regarding products
produced by this source code, please contact the Landsat Contact Us page and
specify USGS CDR/ECV in the "Regarding" section.
https://landsat.usgs.gov/contactus.php 
