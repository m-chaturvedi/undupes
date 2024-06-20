enable_testing()
include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(file_test file_test.cpp)
add_executable(filter_test filter_test.cpp)
add_executable(bin_compare_files_test bin_compare_files_test.cpp)
add_executable(io_test io_test.cpp)
add_executable(filters_list_test filters_list_test.cpp)

target_link_libraries(file_test GTest::gtest_main file)
target_link_libraries(filter_test GTest::gtest_main filter file filters_list
                      bin_compare_files)
target_link_libraries(bin_compare_files_test GTest::gtest_main
                      bin_compare_files)
target_link_libraries(filters_list_test GTest::gtest_main filters_list file)

target_link_libraries(
  io_test
  GTest::gtest_main
  io
  file
  filter
  filters_list
  cli)

include(GoogleTest)
gtest_discover_tests(file_test)
gtest_discover_tests(filter_test)
gtest_discover_tests(bin_compare_files_test)
gtest_discover_tests(io_test)
gtest_discover_tests(filters_list_test)
file(COPY artifacts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY io DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
