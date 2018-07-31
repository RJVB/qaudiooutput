#include <QDebug>
#include <QByteArray>
#include <QAudioDecoder>
#include <QBuffer>

#include "qmixerstream.h"
#include "qaudiodecoderstream.h"
#include "qmixerstreamhandle.h"
#include "qabstractmixerstream.h"
#include "qmixerstream_p.h"

QMixerStream::QMixerStream(const QAudioFormat &format, QObject *parent)
    : QIODevice(parent)
    , d_ptr(new QMixerStreamPrivate(format))
{
    setOpenMode(QIODevice::ReadOnly);
}

QMixerStream::~QMixerStream()
{
    qWarning() << Q_FUNC_INFO << this;
}

QAudioFormat QMixerStream::formatForFile(const QString &fileName)
{
    QAudioDecoder dec;
    dec.setSourceFilename(fileName);
    dec.start();
    QAudioFormat format = dec.audioFormat();
    dec.stop();
    return format;
}

QMixerStreamHandle QMixerStream::openStream(const QString &fileName)
{
    QAudioDecoderStream *stream = new QAudioDecoderStream(fileName, d_ptr->m_format);
    QMixerStreamHandle handle(stream);
    if (stream) {
        d_ptr->m_streams << stream;

        connect(stream, &QAbstractMixerStream::stateChanged, this, &QMixerStream::stateChanged);
        connect(stream, &QAbstractMixerStream::decodingFinished, this, &QMixerStream::decodingFinished);
    }

    return handle;
}

void QMixerStream::closeStream(const QMixerStreamHandle &handle)
{
    QAbstractMixerStream *stream = handle.m_stream;

    if (stream) {
        stream->stop();
        stream->removeFrom(d_ptr->m_streams);
        stream->deleteLater();
    }
}

qint64 QMixerStream::readData(char *data, qint64 maxlen)
{
    const QList<QAbstractMixerStream *> streams = d_ptr->m_streams;
    const int nStreams = streams.size();

    if (nStreams == 1) {
        // 1 stream only, fast codepath
        QAbstractMixerStream *stream = streams.at(0);
        maxlen = stream->readData(data, maxlen);
        if (stream->atEnd()) {
            stream->stop();
        }
    } else {
        memset(data, 0, maxlen);
        const qint16 depth = sizeof(qint16);
        const qint16 samples = maxlen / depth;
        qint64 nRead = 0;
        for (QAbstractMixerStream *stream : streams) {
            qint16 *cursor = (qint16 *)data;
            qint16 sample;

            for (int i = 0; i < samples; i++, cursor++) {
                if (qint64 n = stream->read((char *)&sample, depth)) {
                    nRead += n;
                    *cursor = mix(*cursor, sample);
                }
            }

            if (stream->atEnd()) {
                stream->stop();
            }
        }
        maxlen = nRead / nStreams;
    }

    return maxlen;
}

qint64 QMixerStream::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint16 QMixerStream::mix(qint32 sample1, qint32 sample2)
{
    const qint32 result = sample1 + sample2;

    if (Range::max() < result) {
        return Range::max();
    }

    if (Range::min() > result) {
        return Range::min();
    }

    return result;
}
