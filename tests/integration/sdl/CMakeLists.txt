create_cxx_test(test_SDL)

find_package(SDL2 REQUIRED)
target_include_directories(test_SDL PRIVATE ${SDL2_INCLUDE_DIRS})
