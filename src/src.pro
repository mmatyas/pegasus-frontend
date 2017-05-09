TEMPLATE = subdirs

SUBDIRS += \
    app \
    backend

OTHER_FILES += \
    frontend/*.qml \
    frontend/menuitems/*.qml \
    themes/pegasus-grid/*.qml

app.depends = backend
