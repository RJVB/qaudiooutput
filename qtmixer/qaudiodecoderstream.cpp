#include <QDebug>
#include <QFileInfo>

#include "qaudiodecoderstream.h"
#include "qmixerstreamhandle.h"

QAudioDecoderStream::QAudioDecoderStream(const QString &fileName, const QAudioFormat &format)
    : m_file(fileName)
    , m_input(&m_data)
    , m_output(&m_data)
    , m_format(format)
    , m_state(QtMixer::Stopped)
    , m_loops(0)
    , m_remainingLoops(0)
{
    QFileInfo finfo(fileName);

    if (!finfo.exists() || !finfo.isReadable()) {
        m_state = QtMixer::Unknown;
        qCritical() << "File" << fileName << "doesn't exist or isn't readable";
        return;
    }

    setOpenMode(QIODevice::ReadOnly);

    const bool valid =
        m_file.open(QIODevice::ReadOnly) &&
        m_output.open(QIODevice::ReadOnly) &&
        m_input.open(QIODevice::WriteOnly);

    if (valid) {
        m_bufferSize = finfo.size();
        m_data.reserve(m_bufferSize);
        m_currentLength = 0;

        m_decoder.setNotifyInterval(10);
        m_decoder.setAudioFormat(format);
        m_decoder.setSourceDevice(&m_file);
        m_decoder.start();

        if (!m_decoder.error()) {
            connect(&m_decoder, &QAudioDecoder::bufferReady, this, &QAudioDecoderStream::bufferReady);
            connect(&m_decoder, static_cast<void(QAudioDecoder::*)(QAudioDecoder::Error)>(&QAudioDecoder::error),
                    this, &QAudioDecoderStream::error);
            connect(&m_decoder, &QAudioDecoder::finished, this, &QAudioDecoderStream::finished);
        } else {
            qCritical() << "Decoder error" << m_decoder.errorString() << "in QAudioDecoderStream";
            m_decoder.stop();
            m_state = QtMixer::Unknown;
        }
    } else {
        m_state = QtMixer::Unknown;
        qCritical() << "File or buffer initialisation failure in QAudioDecoderStream";
    }
}

qint64 QAudioDecoderStream::readData(char *data, qint64 maxlen)
{
    memset(data, 0, maxlen);

    if (m_state == QtMixer::Playing) {
        maxlen = m_output.read(data, maxlen);

        if (m_output.size() &&
                m_output.atEnd()) {
            if (m_loops != 0) {
                if (m_loops > 0) {
                    if ((--m_remainingLoops) > 0) {
                        rewind();
                    }
                } else {
                    rewind();
                }
            }
        }
    }

    return maxlen;
}

qint64 QAudioDecoderStream::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

void QAudioDecoderStream::rewind()
{
    if (m_state != QtMixer::Unknown) {
        m_output.seek(0);
    }
}

void QAudioDecoderStream::bufferReady()
{
    if (m_state != QtMixer::Unknown) {
        const QAudioBuffer &buffer = m_decoder.read();

        const int length = buffer.byteCount();
        const char *data = buffer.data<char>();

        m_currentLength += length;
        if (m_currentLength > m_bufferSize) {
            m_bufferSize *= 2;
            m_data.resize(m_bufferSize);
        }

        m_input.write(data, length);
    }
}

void QAudioDecoderStream::error(QAudioDecoder::Error error)
{
    qDebug() << Q_FUNC_INFO << m_decoder.errorString();
    emit decodingError(this, error, m_decoder.errorString());
}

void QAudioDecoderStream::finished()
{
    m_input.close();
    qWarning() << "Decoding done; reserved,actual bufSize=" << m_bufferSize << m_data.size()
        << "m_input,m_output len=" << m_input.size() << m_output.size()
        << "m_input,m_output pos=" << m_input.pos() << m_output.pos()
        << "format:" << m_decoder.audioFormat();
    emit decodingFinished(this);
}

bool QAudioDecoderStream::atEnd() const
{
    if (m_state != QtMixer::Unknown) {
        return m_output.size()
               && m_output.atEnd();
    } else {
        return true;
    }
}

bool QAudioDecoderStream::done() const
{
    return m_state != QtMixer::Unknown && m_output.size()
           && m_decoder.state() != QAudioDecoder::DecodingState;
}

void QAudioDecoderStream::play()
{
    if (m_state != QtMixer::Unknown) {
        m_state = QtMixer::Playing;

        emit stateChanged(this, m_state);
    }
}

void QAudioDecoderStream::pause()
{
    if (m_state == QtMixer::Playing) {
        m_state = QtMixer::Paused;

        emit stateChanged(this, m_state);
    }
}

void QAudioDecoderStream::stop()
{
    if (m_state != QtMixer::Unknown && m_state != QtMixer::Stopped) {
        qDebug() << Q_FUNC_INFO << "reserved,actual bufSize=" << m_bufferSize << m_data.size()
            << "m_input,m_output len=" << m_input.size() << m_output.size()
            << "m_input,m_output pos=" << m_input.pos() << m_output.pos() << position()
            << "m_output.atEnd=" << m_output.atEnd();
        m_state = QtMixer::Stopped;
        m_remainingLoops = m_loops;

        rewind();

        emit stateChanged(this, m_state);
    }
}

QtMixer::State QAudioDecoderStream::state() const
{
    return m_state;
}

int QAudioDecoderStream::loops() const
{
    return m_loops;
}

void QAudioDecoderStream::setLoops(int loops)
{
    m_loops = loops;
    m_remainingLoops = loops;
}

int QAudioDecoderStream::position() const
{
    if (m_state != QtMixer::Unknown && m_format.isValid()) {
        return m_output.pos()
           / (m_format.sampleSize() / 8)
           / (m_format.sampleRate() / 1000)
           / (m_format.channelCount());
    } else {
        return -1;
    }
}

void QAudioDecoderStream::setPosition(int position)
{
    if (m_state != QtMixer::Unknown && m_format.isValid()) {
        const int target = position
                       * (m_format.sampleSize() / 8)
                       * (m_format.sampleRate() / 1000)
                       * (m_format.channelCount());
        m_output.seek(target);
    }
}

int QAudioDecoderStream::length()
{
    if (m_state != QtMixer::Unknown && m_format.isValid()) {
        return m_output.size()
           / (m_format.sampleSize() / 8)
           / (m_format.sampleRate() / 1000)
           / (m_format.channelCount());
    } else {
        return -1;
    }
}
