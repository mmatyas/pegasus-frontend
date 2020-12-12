pegasus_cxx_test(test_PlayniteProvider)

qtquick_compiler_add_resources(TEST_RESOURCES data/data.qrc)
target_sources(test_PlayniteProvider PRIVATE ${TEST_RESOURCES})
