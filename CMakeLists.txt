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

target_link_libraries(re4_dump PRIVATE re4ctor_core)

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
