CONFIG += c++11
QT += multimedia
TEMPLATE = lib
DEFINES += QTMIXER_LIBRARY
TARGET = $$qtLibraryTarget(QtMixer)
VER_MAJ = 2

SOURCES += \
	qaudiodecoderstream.cpp \
	qmixerstream.cpp \
	qmixerstreamhandle.cpp \
	qmixerstream_p.cpp

INSTALL_HEADERS += \
	qmixerstream.h \
	qmixerstreamhandle.h \
	qtmixer.h \
	qmixerstream \
	qmixerstreamhandle

PRIVATE_HEADERS += \
	qaudiodecoderstream.h \
	qabstractmixerstream.h \
	qmixerstream_p.h

HEADERS = \
	$${INSTALL_HEADERS} \
	$${PRIVATE_HEADERS}

OTHER_FILES += \
	mixer.prf

headers.files = $${INSTALL_HEADERS}
headers.path = $$[QT_INSTALL_HEADERS]/QtMixer

target.path = $$[QT_INSTALL_LIBS]

features.files = mixer.prf
features.path = $$[QMAKE_MKSPECS]/features

INSTALLS += headers target features
