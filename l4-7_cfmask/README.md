## l4-7_cfmask

This application produces Cloud Mask products for Landsats 4, 5, and 7 based
on the CFMASK (Function of Mask Algorithm).

## Detailed Algorithm
See TODO TODO TOD.

## Release Notes
TODO TODO TODO

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
```
    git clone https://github.com/USGS-EROS/espa-cloud-masking.git
    cd espa-cloud-masking/l4-7_cfmask
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
