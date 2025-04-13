pegasus_cxx_test(test_FavoriteDB)

qt_add_resources(TEST_RESOURCES data/data.qrc)
target_sources(test_FavoriteDB PRIVATE ${TEST_RESOURCES})
