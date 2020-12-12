pegasus_qml_test(test_apng)

qtquick_compiler_add_resources(TEST_RESOURCES data.qrc)
target_sources(test_apng PRIVATE ${TEST_RESOURCES})
