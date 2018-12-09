TEMPLATE = subdirs

SUBDIRS += \
    backend \
    benchmarks \
    integration \

requires(qtHaveModule(testlib))
