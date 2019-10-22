TEMPLATE = subdirs

SUBDIRS += \
    app \
    backend \
    frontend

app.depends = backend frontend
