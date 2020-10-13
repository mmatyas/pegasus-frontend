find_package(Qt5 REQUIRED COMPONENTS Qml)

add_library(QtQmlTricks
    QQmlObjectListModel.cpp
    QQmlObjectListModel.h
)
target_link_libraries(QtQmlTricks PUBLIC Qt5::Qml)
