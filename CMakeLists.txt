cmake_minimum_required(VERSION 3.16)
project(re4ctor_core C)
set(CMAKE_C_STANDARD 99)

# --- core static lib (публічні хедери в include/)
add_library(re4ctor_core STATIC src/core/re4ctor_core.c)
set_target_properties(re4ctor_core PROPERTIES OUTPUT_NAME "re4ctor_core")
target_include_directories(re4ctor_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# --- executable (все інше тулимо сюди)
add_executable(re4_dump
  src/core/re4_dump.c
  src/drbg/drbg.c
  src/entropy/entropy.c
  src/health/health.c
  src/sp800_90b/estimator.c
)

target_include_directories(re4_dump PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(re4_dump PRIVATE re4ctor_core)
target_include_directories(re4_dump PRIVATE ${CMAKE_SOURCE_DIR}/src)

# math lib не потрібен на Windows
if (WIN32)
  target_link_libraries(re4_dump PRIVATE bcrypt)
else()
  target_link_libraries(re4_dump PRIVATE m)
endif()

# Увімкнути RDRAND/RDSEED ТІЛЬКИ на GCC/Clang та x86_64 і лише для entropy.c
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|x86-64")
    set_source_files_properties(src/entropy/entropy.c
      PROPERTIES COMPILE_OPTIONS "-mrdrnd;-mrdseed")
  endif()
endif()

# MSVC на x64 має інтрінсики без додаткових флагів, нічого робити не треба
include(GNUInstallDirs)

install(TARGETS re4ctor_core
        EXPORT re4ctorCoreTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT re4ctorCoreTargets
        NAMESPACE re4::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/re4ctor_core)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/re4ctor_coreConfigVersion.cmake"
  VERSION 0.1.0
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/re4ctor_coreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/re4ctor_coreConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/re4ctor_core)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/re4ctor_coreConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/re4ctor_coreConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/re4ctor_core)
