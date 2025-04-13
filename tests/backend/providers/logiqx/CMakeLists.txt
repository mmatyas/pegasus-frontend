pegasus_cxx_test(test_LogiqxProvider)

qt_add_resources(TEST_RESOURCES data/data.qrc)
target_sources(test_LogiqxProvider PRIVATE ${TEST_RESOURCES})
