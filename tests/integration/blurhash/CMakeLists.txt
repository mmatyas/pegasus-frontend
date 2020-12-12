pegasus_qml_test(test_Blurhash)

qtquick_compiler_add_resources(TEST_RESOURCES data.qrc)
target_sources(test_Blurhash PRIVATE ${TEST_RESOURCES})
