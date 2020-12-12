pegasus_cxx_test(test_PegasusMediaProvider)

qtquick_compiler_add_resources(TEST_RESOURCES data/data.qrc)
target_sources(test_PegasusMediaProvider PRIVATE ${TEST_RESOURCES})
