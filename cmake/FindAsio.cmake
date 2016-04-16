# The following variables will be defined:
#
#  ASIO_ROOT
#  ASIO_INCLUDE_DIR
#  ASIO_DEFINES
#
#  No ASIO_LIBRARIES because it's header only library.
#


# ============================================================================
# Find ASIO header directory
# ============================================================================

set (ASIO_ROOT ${PROJECT_SOURCE_DIR}/src/libs/asio)
set (ASIO_INCLUDE_DIR ${ASIO_ROOT}/include)

# ============================================================================
# Add definitions
# ============================================================================


