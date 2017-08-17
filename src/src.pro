TEMPLATE = subdirs

SUBDIRS += \
    app \
    backend

OTHER_FILES += \
    *.pri \
    frontend/*.qml \
    frontend/menuitems/*.qml \
    frontend/gamepad/*.qml \
    frontend/gamepad/preview/*.qml \
    frontend/settings/*.qml \
    themes/pegasus-grid/*.qml \
    themes/pegasus-grid/filter_panel/*.qml

app.depends = backend
