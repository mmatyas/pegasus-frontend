pegasus_cxx_test(test_LaunchBoxProvider)

qtquick_compiler_add_resources(TEST_RESOURCES data/data.qrc)
target_sources(test_LaunchBoxProvider PRIVATE ${TEST_RESOURCES})
