TEMPLATE = subdirs

SUBDIRS += \
    app \
    backend

OTHER_FILES += \
    frontend/*.qml \
    frontend/menuitems/*.qml \
    frontend/gamepad/*.qml \
    frontend/gamepad/preview/*.qml \
    themes/pegasus-grid/*.qml

app.depends = backend
