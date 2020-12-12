pegasus_cxx_test(test_Playtime)

qtquick_compiler_add_resources(TEST_RESOURCES data.qrc)
target_sources(test_Playtime PRIVATE ${TEST_RESOURCES})
