#ifndef QMIXERSTREAMHANDLE_H
#define QMIXERSTREAMHANDLE_H

#include "qtmixer.h"

class QAbstractMixerStream;
class QAudioDecoderStream;
class QMixerStream;

class QTMIXER_EXPORT QMixerStreamHandle
{
    friend class QMixerStream;
    friend class QAudioDecoderStream;

public:
    QMixerStreamHandle();

    void play();
    void pause();
    void stop();

    QtMixer::State state() const;

    int loops() const;
    void setLoops(int loops);

    int position() const;
    void setPosition(int position);
    bool atEnd();

    int length() const;
    bool isValid() const;

    bool operator ==(const QMixerStreamHandle &other) const;

private:
    QMixerStreamHandle(QAbstractMixerStream *stream);

    QAbstractMixerStream *m_stream;
};

#endif // QMIXERSTREAMHANDLE_H
