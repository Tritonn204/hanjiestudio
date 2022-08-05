#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "KiWi" for configuration ""
set_property(TARGET KiWi APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(KiWi PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LINK_INTERFACE_LIBRARIES_NOCONFIG "mingw32;C:/mingw64/lib/libSDL2.dll.a;C:/mingw64/lib/libSDL2_ttf.dll.a;C:/mingw64/lib/libSDL2_image.dll.a"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libKiWi.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS KiWi )
list(APPEND _IMPORT_CHECK_FILES_FOR_KiWi "${_IMPORT_PREFIX}/lib/libKiWi.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
