#ifndef QTMIXERGLOBAL_H
#define QTMIXERGLOBAL_H

#include <QtCore/QtGlobal>

#ifdef BUILD_QTMIXER_WITH_QMAKE
#if defined QTMIXER_LIBRARY
#define QTMIXER_EXPORT Q_DECL_EXPORT
#else
#define QTMIXER_EXPORT Q_DECL_IMPORT
#endif
#else
#include <qtmixer_export.h>
#endif

namespace QtMixer
{
    enum State {
        Playing,
        Paused,
        Stopped,
        Unknown
    };
}

#endif // QTMIXERGLOBAL_H
