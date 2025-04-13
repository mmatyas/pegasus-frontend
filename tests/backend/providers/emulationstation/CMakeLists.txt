pegasus_cxx_test(test_EmulationStationProvider)

qt_add_resources(TEST_RESOURCES data/data.qrc)
target_sources(test_EmulationStationProvider PRIVATE ${TEST_RESOURCES})
