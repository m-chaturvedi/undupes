add_executable(undupe main.cpp file.h)

add_library(debug debug.h debug.cpp)
add_library(filter filter.h filter.cpp)
add_library(file file.h file.cpp debug.h)
add_library(filters_list filters_list.h filters_list.cpp)
add_library(io io.h io.cpp)
add_library(cli cli.h cli.cpp)
add_library(bin_compare_files bin_compare_files.h bin_compare_files.cpp)

target_link_libraries(
  undupe
  filters_list
  filter
  file
  io
  cli
  bin_compare_files
  nlohmann_json::nlohmann_json
  cxxopts)

target_link_directories(undupe PRIVATE ${gflags_BINARY_DIR}
                        ${spdlog_BINARY_DIR})
