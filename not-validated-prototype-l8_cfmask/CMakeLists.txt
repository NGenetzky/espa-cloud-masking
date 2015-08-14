
cmake_minimum_required ( VERSION 2.8.12 )

#-----------------------------------------------------------------------------
# To install where you want, do the following
# cmake CMakeLists.txt -DCMAKE_INSTALL_PREFIX=$PREFIX

set ( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/l8cfmask" )

########################### Un-Installing software ###########################
# For a complex uninstall do something like the distclean target
# Do this for a simple uninstall everything
if (NOT CLOUD_MASKING_UNINSTALL_TARGET)
    add_custom_target ( uninstall COMMAND xargs rm < install_manifest.txt )
endif ()

# Add a distclean target to the Makefile
if (NOT CLOUD_MASKING_DISTCLEAN_TARGET)
    add_custom_target ( distclean COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/distclean.cmake )
endif ()

############################### Compiler Flags ###############################
set ( ESPA_COMPILE_FLAGS "-Wall -m64 -O2 -g" )
set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${ESPA_COMPILE_FLAGS}" )

################################ Linker Flags ################################
if (BUILD_STATIC)
    set ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static" )
else (BUILD_STATIC)
    set ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" )
endif (BUILD_STATIC)

############################## Build Directories #############################
add_subdirectory ( scripts )
add_subdirectory ( src )
add_subdirectory ( static_data )

