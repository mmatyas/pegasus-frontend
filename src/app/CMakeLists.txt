include(PegasusQtUtils)
pegasus_require_qt(COMPONENTS
    Core
    Qml
    QmlImportScanner
    LinguistTools
)


# Find TS files, compile them to QM and generate a QRC file collecting them

file(GLOB PEGASUS_TS_FILES LIST_DIRECTORIES false "${PROJECT_SOURCE_DIR}/lang/pegasus_*.ts")
qt_add_translation(PEGASUS_QM_FILES "${PEGASUS_TS_FILES}")

set(PEGASUS_LOCALE_QRC "${CMAKE_CURRENT_BINARY_DIR}/locales.qrc")
file(WRITE "${PEGASUS_LOCALE_QRC}" "<!DOCTYPE RCC><RCC version=\"1.0\"><qresource prefix=\"i18n\">\n")

foreach(QM_FILE IN LISTS PEGASUS_QM_FILES)
    get_filename_component(QM_FILENAME "${QM_FILE}" NAME)
    file(APPEND "${PEGASUS_LOCALE_QRC}" "<file>${QM_FILENAME}</file>\n")
endforeach()

file(APPEND "${PEGASUS_LOCALE_QRC}" "</qresource></RCC>")
qt_add_resources(PEGASUS_LOCALE_SRC "${PEGASUS_LOCALE_QRC}")

add_library(pegasus-locales "${PEGASUS_LOCALE_SRC}")


# Main runtime

if("${QT_VERSION_MAJOR}" GREATER_EQUAL 6)
    qt_add_executable(pegasus-fe MANUAL_FINALIZATION WIN32 main.cpp)
else()
    add_executable(pegasus-fe WIN32 main.cpp)
endif()
target_include_directories(pegasus-fe PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
)
target_link_libraries(pegasus-fe PUBLIC
    Qt::Core
    pegasus-assets
    pegasus-backend
    pegasus-locales
    pegasus-qml
)
qt_import_qml_plugins(pegasus-fe)

include(PegasusCommonProps)
pegasus_add_common_props_optimized(pegasus-fe)

# TODO: Workaround for asset differences for now
target_compile_definitions(pegasus-fe PRIVATE PEGASUS_USING_CMAKE)

if(PEGASUS_ENABLE_APNG)
    target_compile_definitions(pegasus-fe PUBLIC WITH_APNG_SUPPORT)
endif()

if("${QT_VERSION_MAJOR}" GREATER_EQUAL 6)
    qt_finalize_executable(pegasus-fe)
endif()


# Install

include(PegasusTargetPlatform)

if(INSTALL_BINDIR)
    install(TARGETS pegasus-fe RUNTIME DESTINATION "${INSTALL_BINDIR}")
endif()

if(INSTALL_DOCDIR)
    install(FILES
        "${PROJECT_SOURCE_DIR}/README.md"
        "${PROJECT_SOURCE_DIR}/LICENSE.md"
        DESTINATION "${INSTALL_DOCDIR}"
    )
endif()

if(PEGASUS_ON_X11)
    if(INSTALL_ICONDIR)
        install(FILES platform/linux/icons/16/org.pegasus_frontend.Pegasus.png DESTINATION "${INSTALL_ICONDIR}/16x16/apps/")
        install(FILES platform/linux/icons/32/org.pegasus_frontend.Pegasus.png DESTINATION "${INSTALL_ICONDIR}/32x32/apps/")
        install(FILES platform/linux/icons/48/org.pegasus_frontend.Pegasus.png DESTINATION "${INSTALL_ICONDIR}/48x48/apps/")
        install(FILES platform/linux/icons/64/org.pegasus_frontend.Pegasus.png DESTINATION "${INSTALL_ICONDIR}/64x64/apps/")
        install(FILES platform/linux/icons/128/org.pegasus_frontend.Pegasus.png DESTINATION "${INSTALL_ICONDIR}/128x128/apps/")
    endif()
    if(INSTALL_DESKTOPDIR AND INSTALL_BINDIR)
        configure_file(
            platform/linux/org.pegasus_frontend.Pegasus.desktop.in
            org.pegasus_frontend.Pegasus.desktop
        )
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.pegasus_frontend.Pegasus.desktop" DESTINATION "${INSTALL_DESKTOPDIR}")
    endif()
    if(INSTALL_APPSTREAMDIR)
        install(FILES platform/linux/org.pegasus_frontend.Pegasus.metainfo.xml DESTINATION "${INSTALL_APPSTREAMDIR}")
    endif()
endif()


# Print config

include(PegasusPrettyPrint)
pegasus_print_box(
    TITLE "App Settings"
    CONTENT
        "Build type: ${CMAKE_BUILD_TYPE}"
        "Git revision: ${PEGASUS_GIT_REVISION}"
        "Git date: ${PEGASUS_GIT_DATE}"
        "Link to static stdc++: ${PEGASUS_STATIC_CXX}"
        "Deployment paths:"
        "  Binaries:          ${INSTALL_BINDIR}"
        "  License/Readme:    ${INSTALL_DOCDIR}"
        "  X11 icons:         ${INSTALL_ICONDIR}"
        "  X11 desktop file:  ${INSTALL_DESKTOPDIR}"
        "  X11 AppStream:     ${INSTALL_APPSTREAMDIR}"
)
