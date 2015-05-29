
cmake_minimum_required ( VERSION 2.8.12 )

include ( FindESPALibCommon.cmake )

find_package ( LibXml2 2.9.1 REQUIRED )
find_package ( ZLIB 1.2.8 REQUIRED )
find_package ( LibLZMA 5.1.2 REQUIRED )

find_library ( Math_Library m ) # We need the standard math library

include_directories ( ${LibESPA_INCLUDES} ${LIBXML2_INCLUDE_DIR} )

add_executable ( cfmask cfmask.c
                        input.c
                        output.c
                        error.c
                        2d_array.c
                        date.c
                        misc.c
                        split_filename.c
                        potential_cloud_shadow_snow_mask.c
                        object_cloud_shadow_match.c )

target_link_libraries ( cfmask ${LibESPA_LIBRARIES}
                               ${LIBXML2_LIBRARIES}
                               ${ZLIB_LIBRARIES}
                               ${LIBLZMA_LIBRARIES}
                               ${Math_Library} )

install ( TARGETS cfmask
          DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

