set(audiooutput_SRCS main.cpp audiooutput.cpp)
add_executable(audiooutput ${audiooutput_SRCS})
target_link_libraries(audiooutput QtMixerStatic Qt5::Widgets)

install(TARGETS audiooutput EXPORT ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
