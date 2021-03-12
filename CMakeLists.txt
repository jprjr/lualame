cmake_minimum_required(VERSION 3.1)
project(lualame)

option(BUILD_SHARED_LIBS "Build modules as shared libraries" ON)

find_package(PkgConfig)
include(FindPackageHandleStandardArgs)

if(LUA_VERSION)
  find_package(Lua ${LUA_VERSION} EXACT REQUIRED)
else()
  find_package(Lua REQUIRED)
  set(LUA_VERSION "${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}" CACHE STRING "Lua version to use")
endif()

pkg_check_modules(PC_LAME QUIET lame)
find_path(LAME_INCLUDE_DIR lame/lame.h
  HINTS ${PC_LAME_INCLUDEDIR} ${PC_LAME_INCLUDE_DIRS})
find_library(LAME_LIBRARY NAMES mp3lame libmp3lame
  HINTS ${PC_LAME_LIBDIR} ${PC_LAME_LIBRARY_DIRS})
find_package_handle_standard_args(LibLame DEFAULT_MSG LAME_LIBRARY LAME_INCLUDE_DIR)
set(LAME_LIBRARIES ${LAME_LIBRARY})
set(LAME_INCLUDE_DIRS ${LAME_INCLUDE_DIR})

set(CMODULE_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/lua/${LUA_VERSION}")
set(LUAMODULE_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/share/lua/${LUA_VERSION}")

configure_file(
  "src/lualame/version.lua"
  "${CMAKE_BINARY_DIR}/lualame/version.lua"
  COPYONLY
)

install(FILES "src/lualame/version.lua"
  DESTINATION "${LUAMODULE_INSTALL_LIB_DIR}/lualame/"
)

set(lualame_sources)
list(APPEND lualame_sources "csrc/lualame.c")
list(APPEND lualame_sources "csrc/lualame_internal.c")
list(APPEND lualame_sources "csrc/lualame_defines.c")

add_library(lualame ${lualame_sources})

target_link_libraries(lualame PRIVATE ${LAME_LIBRARIES})
target_link_directories(lualame PRIVATE ${LAME_LIBRARY_DIRS})
if(WIN32)
    target_link_libraries(lualame PRIVATE ${LUA_LIBRARIES})
endif()
target_include_directories(lualame PRIVATE ${LAME_INCLUDEDIR})
target_include_directories(lualame PRIVATE ${LUA_INCLUDE_DIR})

if(APPLE)
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -undefined dynamic_lookup")
    if(BUILD_SHARED_LIBS)
        set_target_properties(lualame PROPERTIES SUFFIX ".so")
    endif()
endif()

set_target_properties(lualame PROPERTIES PREFIX "")
set_target_properties(lualame PROPERTIES OUTPUT_NAME "lualame")
set_target_properties(lualame PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(lualame PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(lualame PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set_target_properties(lualame PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
  set_target_properties(lualame PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
  set_target_properties(lualame PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
endforeach()

install(TARGETS lualame
  LIBRARY DESTINATION "${CMODULE_INSTALL_LIB_DIR}"
  RUNTIME DESTINATION "${CMODULE_INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${CMODULE_INSTALL_LIB_DIR}"
)
