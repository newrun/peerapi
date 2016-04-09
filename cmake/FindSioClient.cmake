# The following variables will be defined:
#
#  SIOCLIENT_ROOT_DIR
#  SIOCLIENT_INCLUDE_DIR
#  SIOCLIENT_LIBRARIES
#

# ============================================================================
#  Find Boost library. Socket.io cpp client needs boost library.
# ============================================================================

message("Check Boost library for Socket.Io-client-cpp")

if (DEFINED ENV{BOOST_ROOT})
  set(BOOST_ROOT $ENV{BOOST_ROOT})
  message("BOOST_ROOT = '${BOOST_ROOT}' from environment variable")
else() 
  set(BOOST_ROOT
    "boost"
    CACHE PATH
    "Boost root directory."
  )
endif()

if (DEFINED ENV{BOOST_LIBRARYDIR})
  set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})
  message("BOOST_LIBRARYDIR = '${BOOST_LIBRARYDIR}' from environment variable")
else() 
  set(BOOST_LIBRARYDIR
    "${BOOST_ROOT}/out/lib"
    CACHE PATH
    "Boost library directory."
  )
endif()

if (DEFINED ENV{BOOST_VER})
  set(BOOST_VER $ENV{BOOST_VER})
  message("BOOST_VER = '${BOOST_VER}' from environment variable")
else() 
  set(BOOST_VER "1.60.0" CACHE STRING "boost version" )
endif()

set(Boost_USE_STATIC_LIBS ON CACHE INTERNAL "")
set(Boost_USE_MULTITHREADED ON CACHE INTERNAL "")
set(Boost_USE_STATIC_RUNTIME ON CACHE INTERNAL "")  

find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS system date_time random) 


# ============================================================================
# Find Socket.Io client cpp
# ============================================================================

set(SIOCLIENT_ROOT_DIR ${PROJECT_SOURCE_DIR}/src/libs/socket.io-client-cpp)

aux_source_directory(${SIOCLIENT_ROOT_DIR}/src ALL_SIOCLIENT_SRC)
aux_source_directory(${SIOCLIENT_ROOT_DIR}/src/internal ALL_SIOCLIENT_SRC)
file(GLOB ALL_SIOCLIENT_HEADERS ${SIOCLIENT_ROOT_DIR}/src/*.h )
set(SIO_INCLUDEDIR ${SIOCLIENT_ROOT_DIR})


# ============================================================================
# Add static library that support TLS with BoringSSL
# ============================================================================

add_library(sioclient_tls STATIC ${ALL_SIOCLIENT_SRC})

# ============================================================================
# Add include directory
# ============================================================================

target_include_directories(sioclient_tls PRIVATE
    ${Boost_INCLUDE_DIRS} 
    ${SIOCLIENT_ROOT_DIR}/src 
    ${WEBSOCKETPP_ROOT}
    ${RAPIDJSON_INCLUDE_DIR}
    ${WEBRTC_BORING_SSL_INCLUDE}
)

set(SIOCLIENT_INCLUDE_DIR ${SIOCLIENT_ROOT_DIR}/src)

# ============================================================================
# Add definitions
# ============================================================================

if (MSVC)
  target_compile_definitions(sioclient_tls PRIVATE
    -D_WEBSOCKETPP_CPP11_FUNCTIONAL_
    -D_WEBSOCKETPP_CPP11_SYSTEM_ERROR_
    -D_WEBSOCKETPP_CPP11_RANDOM_DEVICE_
    -D_WEBSOCKETPP_CPP11_MEMORY_
    -D_WEBSOCKETPP_CPP11_TYPE_TRAITS_
    )
else()
  target_compile_definitions(sioclient_tls PRIVATE
    -D_WEBSOCKETPP_CPP11_STL_
    )
endif()

# Enable TLS
target_compile_definitions(sioclient_tls PRIVATE -DSIO_TLS)


# ============================================================================
# Add additional propery
# ============================================================================

set_property(TARGET sioclient_tls PROPERTY CXX_STANDARD 11)
set_property(TARGET sioclient_tls PROPERTY CXX_STANDARD_REQUIRED ON)
