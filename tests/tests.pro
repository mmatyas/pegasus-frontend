TEMPLATE = subdirs

SUBDIRS += \
    backend \
    benchmarks

requires(qtHaveModule(testlib))
