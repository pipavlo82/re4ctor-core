cmake_minimum_required(VERSION 3.16)
project(re4ctor_core C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
enable_testing()

# ---- Статична бібліотека з публічними інтерфейсами ----
add_library(re4ctor_core STATIC
  src/core/re4ctor_core.c
)

# Публічні інклуди: у build — з каталогу проекту, після інсталу — з ${CMAKE_INSTALL_INCLUDEDIR}
target_include_directories(re4ctor_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ---- Утиліта re4_dump ----
add_executable(re4_dump
  src/core/re4_dump.c
  src/drbg/drbg.c
  src/entropy/entropy.c
  src/health/health.c
  src/sp800_90b/estimator.c
)
target_link_libraries(re4_dump PRIVATE re4ctor_core m)

# Увімкнути RDRAND/RDSEED лише для файлу entropy.c (на інших платформах просто проігнорується)
set_source_files_properties(src/entropy/entropy.c
  PROPERTIES COMPILE_FLAGS "-mrdrnd -mrdseed"
)

# ---- Тести (мінімальний smoke) ----
add_executable(re4_tests
  tests/basic.c
  src/drbg/drbg.c
  src/entropy/entropy.c
  src/health/health.c
  src/sp800_90b/estimator.c
)
target_link_libraries(re4_tests PRIVATE re4ctor_core m)
add_test(NAME basic COMMAND re4_tests)

# ---- Інсталяція ----
# 1) Встановити бібліотеку та експортувати таргети у re4ctor_coreTargets
install(TARGETS re4ctor_core
  EXPORT re4ctor_coreTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 2) Встановити утиліту
install(TARGETS re4_dump
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 3) Встановити заголовки
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 4) Встановити експортований набір таргетів (щоб find_package() міг підхопити)
install(EXPORT re4ctor_coreTargets
  NAMESPACE re4::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/re4ctor_core
)

<<<<<<< Updated upstream
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
=======
# (Опційно можна додати Config/Version-файли через CMakePackageConfigHelpers,
# але для усунення поточної помилки достатньо експорту вище)
message(STATUS "Build type    : ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
>>>>>>> Stashed changes
