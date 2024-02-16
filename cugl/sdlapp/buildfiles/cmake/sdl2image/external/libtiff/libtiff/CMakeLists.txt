# CMake build for libtiff
#
# Copyright © 2015 Open Microscopy Environment / University of Dundee
# Written by Roger Leigh <rleigh@codelibre.net>
#
# Permission to use, copy, modify, distribute, and sell this software and
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Sam Leffler and Silicon Graphics.
#
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.

# Generate headers
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tif_config.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/tif_config.h
               @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tiffconf.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/tiffconf.h
               @ONLY)

extra_dist(
  ${TIFF_DIR}/SConstruct
  ${TIFF_DIR}/tif_config.h-vms
  ${TIFF_DIR}/tif_config.vc.h
  ${TIFF_DIR}/tif_config.wince.h
  ${TIFF_DIR}/tiffconf.vc.h
  ${TIFF_DIR}/tiffconf.wince.h
  ${TIFF_DIR}/libtiff.def
  ${TIFF_DIR}/libtiff.map
  ${TIFF_DIR}/libtiffxx.map)

set(tiff_HEADERS
  ${TIFF_DIR}/libtiff/tiff.h
  ${TIFF_DIR}/libtiff/tiffio.h
  ${TIFF_DIR}/libtiff/tiffvers.h)

set(tiff_noinst_HEADERS
  ${TIFF_DIR}/libtiff/t4.h
  ${TIFF_DIR}/libtiff/tif_dir.h
  ${TIFF_DIR}/libtiff/tif_predict.h
  ${TIFF_DIR}/libtiff/tiffiop.h
  ${TIFF_DIR}/libtiff/uvcode.h)

set(nodist_tiff_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/tiffconf.h)

set(tiff_SOURCES
  ${TIFF_DIR}/libtiff/tif_aux.c
  ${TIFF_DIR}/libtiff/tif_close.c
  ${TIFF_DIR}/libtiff/tif_codec.c
  ${TIFF_DIR}/libtiff/tif_color.c
  ${TIFF_DIR}/libtiff/tif_compress.c
  ${TIFF_DIR}/libtiff/tif_dir.c
  ${TIFF_DIR}/libtiff/tif_dirinfo.c
  ${TIFF_DIR}/libtiff/tif_dirread.c
  ${TIFF_DIR}/libtiff/tif_dirwrite.c
  ${TIFF_DIR}/libtiff/tif_dumpmode.c
  ${TIFF_DIR}/libtiff/tif_error.c
  ${TIFF_DIR}/libtiff/tif_extension.c
  ${TIFF_DIR}/libtiff/tif_fax3.c
  ${TIFF_DIR}/libtiff/tif_fax3sm.c
  ${TIFF_DIR}/libtiff/tif_flush.c
  ${TIFF_DIR}/libtiff/tif_getimage.c
  ${TIFF_DIR}/libtiff/tif_jbig.c
  ${TIFF_DIR}/libtiff/tif_jpeg.c
  ${TIFF_DIR}/libtiff/tif_jpeg_12.c
  ${TIFF_DIR}/libtiff/tif_luv.c
  ${TIFF_DIR}/libtiff/tif_lzma.c
  ${TIFF_DIR}/libtiff/tif_lzw.c
  ${TIFF_DIR}/libtiff/tif_next.c
  ${TIFF_DIR}/libtiff/tif_ojpeg.c
  ${TIFF_DIR}/libtiff/tif_open.c
  ${TIFF_DIR}/libtiff/tif_packbits.c
  ${TIFF_DIR}/libtiff/tif_pixarlog.c
  ${TIFF_DIR}/libtiff/tif_predict.c
  ${TIFF_DIR}/libtiff/tif_print.c
  ${TIFF_DIR}/libtiff/tif_read.c
  ${TIFF_DIR}/libtiff/tif_strip.c
  ${TIFF_DIR}/libtiff/tif_swab.c
  ${TIFF_DIR}/libtiff/tif_thunder.c
  ${TIFF_DIR}/libtiff/tif_tile.c
  ${TIFF_DIR}/libtiff/tif_version.c
  ${TIFF_DIR}/libtiff/tif_warning.c
  ${TIFF_DIR}/libtiff/tif_webp.c
  ${TIFF_DIR}/libtiff/tif_write.c
  ${TIFF_DIR}/libtiff/tif_zip.c
  ${TIFF_DIR}/libtiff/tif_zstd.c)

set(tiffxx_HEADERS
  ${TIFF_DIR}/libtiff/tiffio.hxx)

set(tiffxx_SOURCES
  ${TIFF_DIR}/libtiff/tif_stream.cxx)

if(USE_WIN32_FILEIO)
  extra_dist(${TIFF_DIR}/libtiff/tif_unix.c)
  list(APPEND tiff_SOURCES ${TIFF_DIR}/libtiff/tif_win32.c)
else()
  extra_dist(tif_win32.c)
  list(APPEND tiff_SOURCES ${TIFF_DIR}/libtiff/tif_unix.c)
endif()

add_library(tiff ${tiff_SOURCES} ${tiff_HEADERS} ${nodist_tiff_HEADERS}
            ${tiff_port_SOURCES} ${TIFF_DIR}/libtiff/libtiff.def)
target_include_directories(tiff
    PUBLIC
        "$<BUILD_INTERFACE:${TIFF_DIR}/libtiff>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${TIFF_INCLUDES}>"
)
target_link_libraries(tiff ${TIFF_LIBRARY_DEPS})
set_target_properties(tiff PROPERTIES SOVERSION ${SO_COMPATVERSION})
if(NOT CYGWIN)
    # This property causes shared libraries on Linux to have the full version
    # encoded into their final filename.  We disable this on Cygwin because
    # it causes cygz-${TIFF_FULL_VERSION}.dll to be created when cygz.dll
    # seems to be the default.
    set_target_properties(tiff PROPERTIES VERSION ${SO_VERSION})
endif()
if(WIN32)
# Windows links with libtiff.def, nothing else needed.
elseif(APPLE)
  set_target_properties(tiff PROPERTIES LINK_FLAGS
                        "-Wl,-exported_symbols_list,${TIFF_DIR}/libtiff/libtiff.exp")
elseif(HAVE_LD_VERSION_SCRIPT)
  set_target_properties(tiff PROPERTIES LINK_FLAGS
                        "-Wl,--version-script=${TIFF_DIR}/libtiff/libtiff.map")
endif()

install(TARGETS tiff
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

install(FILES ${tiff_HEADERS} ${nodist_tiff_HEADERS}
        DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}")

if(CXX_SUPPORT)
  add_library(tiffxx ${tiffxx_SOURCES} ${tiffxx_HEADERS})
  target_link_libraries(tiffxx tiff)
  set_target_properties(tiffxx PROPERTIES SOVERSION ${SO_COMPATVERSION})
  if(NOT CYGWIN)
    # This property causes shared libraries on Linux to have the full version
    # encoded into their final filename.  We disable this on Cygwin because
    # it causes cygz-${TIFF_FULL_VERSION}.dll to be created when cygz.dll
    # seems to be the default.
    set_target_properties(tiffxx PROPERTIES VERSION ${SO_VERSION})
  endif()
  if(HAVE_LD_VERSION_SCRIPT)
    set_target_properties(tiffxx PROPERTIES LINK_FLAGS
                          "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libtiffxx.map")
  endif()

  install(TARGETS tiffxx
          RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

  install(FILES ${tiffxx_HEADERS}
          DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}")

endif()
