
cmake_minimum_required ( VERSION 2.8.12 )

find_package ( PythonInterp 2.7.0 REQUIRED )
find_package ( PythonLibs 2.7.0 REQUIRED )

install ( PROGRAMS run_fillminima.py
          DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

install ( FILES fillminima.py
          DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

