TEMPLATE = subdirs

SUBDIRS += \
    app \
    backend \
    thirdparty \

app.depends = backend
backend.depends = thirdparty
