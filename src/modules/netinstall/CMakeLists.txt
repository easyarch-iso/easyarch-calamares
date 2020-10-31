calamares_add_plugin( netinstall
    TYPE viewmodule
    EXPORT_MACRO PLUGINDLLEXPORT_PRO
    SOURCES
        Config.cpp
        NetInstallViewStep.cpp
        HostWidget.h
        HostWidget.cpp
        NetInstallPage.h
        NetInstallPage.cpp
        NoNetInstallPage.h
        NoNetInstallPage.cpp
        PackageTreeItem.cpp
        PackageModel.cpp
    UI
        page_netinst.ui
        page_nonetinst.ui
    LINK_PRIVATE_LIBRARIES
        calamaresui
        Qt5::Network
        yamlcpp
    SHARED_LIB
)

calamares_add_test(
    netinstalltest
    SOURCES
        Tests.cpp
        PackageTreeItem.cpp
        PackageModel.cpp
    LIBRARIES
        Qt5::Gui
        yamlcpp
)

