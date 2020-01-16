enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(Catch2)

add_executable(tokenizer_tests catch2_main.cpp tokenizer_tests.cpp)
target_link_libraries(tokenizer_tests PRIVATE Catch2 ls_cparser)

enable_language(C)
set(CMAKE_C_STANDARD 99)

add_executable(ctest test.c)
target_link_libraries(ctest PRIVATE ls_cparser)