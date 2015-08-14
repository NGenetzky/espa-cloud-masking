
cmake_minimum_required ( VERSION 2.8.12 )

#-----------------------------------------------------------------------------
# To install where you want do the following
# cmake CMakeLists.txt -DCMAKE_INSTALL_PREFIX=$PREFIX

########################### Un-Installing software ###########################
# For a complex uninstall do something like the distclean target
# Do this for a simple uninstall everything
add_custom_target ( uninstall COMMAND xargs rm < install_manifest.txt )
set ( CLOUD_MASKING_UNINSTALL_TARGET 1 )

# Add a distclean target to the Makefile
add_custom_target ( distclean COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/distclean.cmake )
set ( CLOUD_MASKING_DISTCLEAN_TARGET 1 )

############################## Build Directories #############################
add_subdirectory ( l4-7_cfmask )
add_subdirectory ( not-validated-prototype-l8_cfmask )

