set(qtmixer_LIB_SRCS
    qabstractmixerstream.cpp
    qaudiodecoderstream.cpp
    qmixerstream.cpp
    qmixerstreamhandle.cpp
    qmixerstream_p.cpp
)

ecm_qt_declare_logging_category(qtmixer_LIB_SRCS HEADER logging.h IDENTIFIER QTMIXER CATEGORY_NAME org.kde.kf5.qtmixer)

add_library(QtMixer SHARED ${qtmixer_LIB_SRCS})
generate_export_header(QtMixer BASE_NAME QtMixer)
add_library(KF5::QtMixer ALIAS QtMixer)
add_library(QtMixerStatic STATIC ${qtmixer_LIB_SRCS})
set_target_properties(QtMixerStatic PROPERTIES OUTPUT_NAME QtMixer_static)

target_include_directories(QtMixer INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/QtMixer>")
target_include_directories(QtMixerStatic INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/QtMixer>")

target_link_libraries(QtMixer PUBLIC Qt5::Core Qt5::Multimedia)
target_link_libraries(QtMixerStatic PUBLIC Qt5::Core Qt5::Multimedia)

set_target_properties(QtMixer PROPERTIES  VERSION ${QTMIXER_VERSION_STRING}
                                          SOVERSION ${QTMIXER_SOVERSION}
                                          EXPORT_NAME QtMixer
)

ecm_generate_headers(QtMixer_HEADERS
  HEADER_NAMES
  QtMixer

  REQUIRED_HEADERS QtMixer_HEADERS
)


install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/qtmixer_export.h
  ${QtMixer_HEADERS}
  qmixerstream.h
  qmixerstreamhandle.h
  qtmixer.h
  QMixerStream
  QMixerStreamHandle
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/QtMixer COMPONENT Devel
)

install(
    FILES
        qaudiodecoderstream.h
        qabstractmixerstream.h
        qmixerstream_p.h
    DESTINATION
        ${KDE_INSTALL_INCLUDEDIR_KF5}/QtMixer/private
    COMPONENT
        Devel
)

install(TARGETS QtMixer EXPORT QtMixerTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS QtMixerStatic EXPORT ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME QtMixer LIB_NAME QtMixer DEPS "core multimedia" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/QtMixer)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
install(FILES mixer.prf DESTINATION ${ECM_MKSPECS_INSTALL_DIR}/../features)
