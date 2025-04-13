pegasus_cxx_test(test_PegasusProvider)

qt_add_resources(TEST_RESOURCES data/data.qrc)
target_sources(test_PegasusProvider PRIVATE ${TEST_RESOURCES})
