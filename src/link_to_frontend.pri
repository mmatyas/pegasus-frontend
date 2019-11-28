# Link the project that includes this file to the frontend files

win32:CONFIG(release, debug|release): LIBS += "-L$${TOP_BUILDDIR}/src/frontend/release/" -lfrontend
else:win32:CONFIG(debug, debug|release): LIBS += "-L$${TOP_BUILDDIR}/src/frontend/debug/" -lfrontend
else:unix: LIBS += "-L$${TOP_BUILDDIR}/src/frontend/" -lfrontend

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/frontend/release/libfrontend.a"
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/frontend/debug/libfrontend.a"
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/frontend/release/frontend.lib"
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/frontend/debug/frontend.lib"
else:unix: PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/frontend/libfrontend.a"
