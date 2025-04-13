pegasus_cxx_test(bench_ConfigFile)

qt_add_resources(TEST_RESOURCES data.qrc)
target_sources(bench_ConfigFile PRIVATE ${TEST_RESOURCES})
