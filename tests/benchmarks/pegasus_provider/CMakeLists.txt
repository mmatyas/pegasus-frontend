pegasus_cxx_test(bench_PegasusProvider)

qt_add_resources(TEST_RESOURCES data/data.qrc)
target_sources(bench_PegasusProvider PRIVATE ${TEST_RESOURCES})
