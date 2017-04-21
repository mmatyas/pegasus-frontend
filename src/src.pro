TEMPLATE = subdirs

SUBDIRS += \
    app \
    backend

OTHER_FILES += \
    frontend/*.qml \
    themes/pegasus-grid/*.qml

app.depends = backend
