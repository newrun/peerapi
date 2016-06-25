# The following variables will be defined:
#
#  WEBSOCKETPP_ROOT
#  WEBSOCKETPP_INCLUDE
#  WEBSOCKETPP_DEFINES
#
#  No WEBSOCKETPP_LIBRARIES because it's header only library.
#


# ============================================================================
# Find WEBSOCKETPP header directory
# ============================================================================

set (WEBSOCKETPP_ROOT ${PROJECT_SOURCE_DIR}/src/libs/websocketpp)
set (WEBSOCKETPP_INCLUDE_DIIR ${WEBSOCKETPP_ROOT})

# ============================================================================
# Add definitions
# ============================================================================

if (MSVC)
  set(WEBSOCKETPP_DEFINES
    -D_WEBSOCKETPP_CPP11_FUNCTIONAL_
    -D_WEBSOCKETPP_CPP11_SYSTEM_ERROR_
    -D_WEBSOCKETPP_CPP11_RANDOM_DEVICE_
    -D_WEBSOCKETPP_CPP11_MEMORY_
    -D_WEBSOCKETPP_CPP11_TYPE_TRAITS_
    -D_WEBSOCKETPP_CPP11_THREAD_
    -DASIO_STANDALONE
    -DASIO_HAS_STD_CHRONO
    )
else()
  set(WEBSOCKETPP_DEFINES
    -D_WEBSOCKETPP_CPP11_STL_
    -DASIO_STANDALONE
    )
endif()

