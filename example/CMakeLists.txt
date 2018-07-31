add_executable(audiooutput audiooutput/main.cpp audiooutput/audiooutput.cpp)
target_link_libraries(audiooutput QtMixerStatic Qt5::Widgets)
install(TARGETS audiooutput EXPORT ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

add_executable(audiodecoder audiodecoder/main.cpp audiodecoder/audiodecoder.cpp audiodecoder/wavefilewriter.cpp)
target_link_libraries(audiodecoder Qt5::Multimedia)
install(TARGETS audiodecoder EXPORT ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
