pegasus_cxx_test(bench_ConfigFile)

qtquick_compiler_add_resources(TEST_RESOURCES data.qrc)
target_sources(bench_ConfigFile PRIVATE ${TEST_RESOURCES})
