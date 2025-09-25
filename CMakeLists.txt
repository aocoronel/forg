cmake_minimum_required(VERSION 3.16)

project (forg
  VERSION 0.1.0
  DESCRIPTION "forg | A simple file organizer"
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CheckCCompilerFlag)
function(enable_strict_warnings target)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE /W4 /WX)
    endif()
endfunction()

add_library(vendor STATIC)
add_library(core STATIC)

target_sources(vendor PRIVATE
    vendor/bashgen.c
    vendor/zshgen.c
    vendor/printfc.c
    vendor/printh.c
)
target_sources(core PRIVATE
    src/validate.c
)

set(CMAKE_C_FLAGS_RELEASE "-O2")

target_include_directories(vendor PUBLIC vendor)
target_compile_features(vendor PUBLIC c_std_99)

target_include_directories(core PUBLIC include)
target_compile_features(core PUBLIC c_std_99)

add_executable(forg forg.c)

target_include_directories(forg PRIVATE include)
target_link_libraries(forg PRIVATE core vendor)

enable_strict_warnings(forg)
enable_strict_warnings(core)
enable_strict_warnings(vendor)

install(TARGETS forg RUNTIME DESTINATION bin)
# install(DIRECTORY include/ DESTINATION include)
# install(FILES docs/forg.1 DESTINATION share/man/man1)
# install(FILES docs/forg.5 DESTINATION share/man/man5)
