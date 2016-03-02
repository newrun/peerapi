# The following variables will be defined:
#
#  ZLIB_LIBRARIES
#  ZLIB_INCLUDE_DIR
#


# ============================================================================
# Add subdirectory
# ============================================================================

set(SKIP_INSTALL_ALL ON CACHE INTERNAL "")
add_subdirectory(${PROJECT_SOURCE_DIR}/src/libs/zlib)

# ============================================================================
# Find ZLIB header directory
# ============================================================================

set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/libs/zlib)

# ============================================================================
# Find ZLIB libraries
# ============================================================================

if (MSVC)
list(APPEND
  ZLIB_LIBRARIES
  optimized ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlibstatic.lib
  debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlibstaticd.lib
  )
else()
list(APPEND
  ZLIB_LIBRARIES
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlib.a
  )
endif()