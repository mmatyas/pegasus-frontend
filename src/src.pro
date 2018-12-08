TEMPLATE = subdirs

SUBDIRS += \
    app \
    backend

app.depends = backend
