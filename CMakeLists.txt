cmake_minimum_required(VERSION 3.0)

project(QtMixer VERSION 0.0)

# ECM setup
include(FeatureSummary)
find_package(ECM 5.47.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})


include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

# Dependencies
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

set(REQUIRED_QT_VERSION 5.8.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets Multimedia)

ecm_setup_version(PROJECT VARIABLE_PREFIX QTMIXER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/qtmixer_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/QtMixerConfigVersion.cmake"
                        SOVERSION 0)
# Subdirectories
add_subdirectory(qtmixer)
add_subdirectory(example)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/QtMixer")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/QtMixerConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/QtMixerConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/QtMixerConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/QtMixerConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT QtMixerTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE QtMixerTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtmixer_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel)


feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
