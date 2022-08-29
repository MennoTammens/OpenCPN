cmake_minimum_required(VERSION 3.14)


project(opencpn_tests)
set(CMAKE_CXX_STANDARD 14)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing ()

set(PROJ_SRC ${PROJECT_SOURCE_DIR}/../src)

add_executable(tests
  tests.cpp
  ${MODEL_SRC}
  ${CMAKE_SOURCE_DIR}/src/api_shim.cpp
  ${CMAKE_SOURCE_DIR}/src/BasePlatform.cpp
)

target_include_directories(
  tests
  PRIVATE
  ${PROJECT_SOURCE_DIR}/../include
  ${CMAKE_BINARY_DIR}/include
  ${PROJECT_SOURCE_DIR}/../libs/sound/include
  ${PROJECT_SOURCE_DIR}/../libs/sound/include
  ${PROJECT_SOURCE_DIR}/../buildandroid/libcurl/include
)

target_link_libraries(tests PRIVATE ${wxWidgets_LIBRARIES})
if (DEFINED LIBELF_LIBRARY)
  target_link_libraries(tests PRIVATE "${LIBELF_LIBRARY}")
endif ()
target_link_libraries(tests PRIVATE ocpn::wxjson)
target_link_libraries(tests PRIVATE ocpn::libarchive)
target_link_libraries(tests PRIVATE ocpn::gdal)
target_link_libraries(tests PRIVATE ocpn::nmea0183)
target_link_libraries(tests PRIVATE ocpn::N2KParser)
target_link_libraries(tests PRIVATE ocpn::iso8211)

if (DEFINED CURL_LIBRARIES)
  target_link_libraries(tests PRIVATE ${CURL_LIBRARIES})
endif ()
if (DEFINED WXCURL_LIBRARIES)
  target_link_libraries(tests PRIVATE ${WXCURL_LIBRARIES})
elseif (DEFINED WXSYS_CURL_LIBRARIES)
  target_link_libraries(tests PRIVATE ${SYS_WXCURL_LIBRARIES})
elseif (TARGET ocpn::wxcurl)
  target_link_libraries(tests PRIVATE ocpn::wxcurl)
endif ()

if (DEFINED LIBELF_LIBRARY)
  target_link_libraries(tests PRIVATE ${LIBELF_LIBRARY})
endif ()

target_link_libraries(tests PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(tests)


add_test(NAME tests COMMAND tests)