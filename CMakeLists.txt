cmake_minimum_required(VERSION 3.0...3.28)

project(
  RDPBlocker
  VERSION 1.2.6.5
  DESCRIPTION "Block unauthorized users login in via RDP service."
  LANGUAGES CXX)

# Set project variables
set(CMAKE_CXX_STANDARD
    20
    CACHE STRING "Cpp standard 20")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

aux_source_directory(./source SOURCE_FILES)
file(GLOB RC_FILES "./source/*.rc")

add_executable(RDPBlocker ${SOURCE_FILES} ${RC_FILES})

if(MSVC)
  # Default source code encoding utf-8
  add_compile_options(/utf-8)
  # Enable CFG
  if (MSVC AND (MSVC_VERSION GREATER 1900))
        message("Enable CONTROL FLOW GUARD") 
        add_compile_options(/guard:cf)
  endif()
  # Enable ASLR
  add_link_options(/DYNAMICBASE /HIGHENTROPYVA /LARGEADDRESSAWARE)
  
  # Embeds UAC information
  set_target_properties(RDPBlocker PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
  target_link_options(RDPBlocker PRIVATE "/RELEASE")
  add_compile_definitions(_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
endif()

if(DEFINED ENV{BOOST_ROOT})
  set(BOOST_ROOT $ENV{BOOST_ROOT})
endif()
set(Boost_NO_WARN_NEW_VERSIONS 1)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost 1.84.0 REQUIRED COMPONENTS nowide filesystem locale regex
                                              program_options thread)
if(Boost_FOUND)
  message(STATUS "boost version : ${Boost_VERSION_STRING}")
  message(STATUS "boost include path is : ${Boost_INCLUDE_DIRS}")
  message(STATUS "boost library path is : ${Boost_LIBRARY_DIRS}")
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(RDPBlocker ${Boost_LIBRARIES})
else()
  message(FATAL_ERROR "boost library not found.")
endif()

if(DEFINED ENV{SPDLOG_INCLUDE})
  message(STATUS "ENV SPDLOG_INCLUDE : $ENV{SPDLOG_INCLUDE}")
  set(SPDLOG_INCLUDE $ENV{SPDLOG_INCLUDE})
endif()
if(DEFINED SPDLOG_INCLUDE)
  message(STATUS "SPDLOG_INCLUDE : ${SPDLOG_INCLUDE}")
  include_directories(${SPDLOG_INCLUDE})
  target_include_directories(RDPBlocker PUBLIC ${SPDLOG_INCLUDE})
else()
  message(FATAL_ERROR "SPDLOG_INCLUDE not defined.")
endif()

if(DEFINED ENV{SPDLOG_LIB})
  message(STATUS "ENV SPDLOG_LIB : $ENV{SPDLOG_LIB}")
  set(SPDLOG_LIB $ENV{SPDLOG_LIB})
endif()
if(DEFINED SPDLOG_LIB)
  target_link_directories(RDPBlocker PRIVATE ${SPDLOG_LIB})
  target_link_libraries(RDPBlocker spdlog)
endif()

if(DEFINED ENV{YAML_CPP_INCLUDE})
  message(STATUS "ENV YAML_CPP_INCLUDE : $ENV{YAML_CPP_INCLUDE}")
  set(YAML_CPP_INCLUDE $ENV{YAML_CPP_INCLUDE})
endif()
if(DEFINED YAML_CPP_INCLUDE)
  include_directories(${YAML_CPP_INCLUDE})
  target_include_directories(RDPBlocker PUBLIC ${YAML_CPP_INCLUDE})
endif()

if(DEFINED ENV{YAML_CPP_LIB})
  message(STATUS "ENV YAML_CPP_LIB : $ENV{YAML_CPP_LIB}")
  set(YAML_CPP_LIB $ENV{YAML_CPP_LIB})
endif()
if(DEFINED YAML_CPP_LIB)
    set(YAML_CPP_LIB_FILENAME
        "$<$<CONFIG:Debug>:"
            "yaml-cppd.lib"
        ">"
        "$<$<NOT:$<CONFIG:Debug>>:"
            "yaml-cpp.lib"
        ">"
    )
  target_link_directories(RDPBlocker PRIVATE ${YAML_CPP_LIB})
  target_link_libraries(RDPBlocker ${YAML_CPP_LIB_NAME})
endif()
