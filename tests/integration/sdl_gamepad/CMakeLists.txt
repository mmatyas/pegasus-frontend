pegasus_cxx_test(test_SdlGamepad)

find_package(SDL2 REQUIRED)
target_include_directories(test_SdlGamepad PRIVATE ${SDL2_INCLUDE_DIRS})
