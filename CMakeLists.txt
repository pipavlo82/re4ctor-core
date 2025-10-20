cmake_minimum_required(VERSION 3.16)
project(re4ctor_core C)
set(CMAKE_C_STANDARD 99)

add_library(re4ctor_core STATIC src/core/re4ctor_core.c)
set_target_properties(re4ctor_core PROPERTIES OUTPUT_NAME "re4ctor_core")
target_include_directories(re4ctor_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(re4_dump
  src/core/re4_dump.c
  src/drbg/drbg.c
  src/entropy/entropy.c
  src/health/health.c
  src/sp800_90b/estimator.c
)
 target_link_libraries(re4_dump PRIVATE re4ctor_core)
target_link_libraries(re4_dump PRIVATE m)
# Увімкнути RDRAND/RDSEED лише для реалізації ентропії (x86)
set_source_files_properties(src/entropy/entropy.c
    PROPERTIES COMPILE_FLAGS "-mrdrnd -mrdseed")

