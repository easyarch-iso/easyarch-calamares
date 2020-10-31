include_directories( ${PROJECT_BINARY_DIR}/src/libcalamaresui )

calamares_add_plugin( installtype
    TYPE viewmodule
    EXPORT_MACRO PLUGINDLLEXPORT_PRO
    SOURCES
        InstallTypePage.h
        InstallTypePage.cpp
        InstallTypeViewStep.h
        InstallTypeViewStep.cpp
        MirrorList.h
        MirrorList.cpp
        UpdateMirrorListJob.h
        UpdateMirrorListJob.cpp
    UI
        InstallTypePage.ui
    LINK_PRIVATE_LIBRARIES
        calamaresui
    SHARED_LIB
)