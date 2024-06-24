# Ubuntu:20.04 supports 3.13.4
cmake_minimum_required(VERSION 3.16.3)
include(FetchContent)

project(undupes LANGUAGES CXX)

# set(CMAKE_CXX_COMPILER "/usr/bin/clang++-18") set(CMAKE_C_COMPILER
# "/usr/bin/clang-18") set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra
# -stdlib=libc++") set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}
# -stdlib=libc++ -lc++abi")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_CXX_STANDARD_REQUIRED ON)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.14.1)

# FetchContent_Declare(ftxui GIT_REPOSITORY
# https://github.com/ArthurSonzogni/ftxui GIT_TAG v5.0.0 )

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1)

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)

FetchContent_Declare(
  xxhash
  GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
  GIT_TAG v0.8.1)

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG v3.2.1)

FetchContent_Declare(
  icecream-cpp
  GIT_REPOSITORY https://github.com/renatoGarcia/icecream-cpp.git
  GIT_TAG 31ca6d826e924208853c09ff244d7073d45835b8)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(xxhash)

# FetchContent_GetProperties(ftxui) if(NOT ftxui_POPULATED)
# FetchContent_Populate(ftxui) add_subdirectory(${ftxui_SOURCE_DIR}
# ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL) endif()

# Get gflags
set(CMAKE_CXX_FLAGS_OLD "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wno-cast-function-type -Wno-restrict -Wno-implicit-fallthrough -Wno-unknown-warning-option"
)

FetchContent_MakeAvailable(cxxopts)
FetchContent_MakeAvailable(googletest)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_OLD}")

include_directories(${xxhash_SOURCE_DIR} ${cxxopts_SOURCE_DIR}/include
                    ${spdlog_SOURCE_DIR}/include ${icecream-cpp_SOURCE_DIR})
FetchContent_MakeAvailable(icecream-cpp)

include_directories(
  ${icecream-cpp_SOURCE_DIR} ${xxhash_SOURCE_DIR} ${json_SOURCE_DIR}/include
  ${gflags_BINARY_DIR}/include ${fmt_SOURCE_DIR}/include)
# defines targets and sources
add_subdirectory(src)
add_subdirectory(tests)
