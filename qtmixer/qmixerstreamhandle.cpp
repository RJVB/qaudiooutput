#include "qmixerstreamhandle.h"
#include "qabstractmixerstream.h"

QMixerStreamHandle::QMixerStreamHandle()
    : m_stream(nullptr)
{

}

QMixerStreamHandle::QMixerStreamHandle(QAbstractMixerStream *stream)
    : m_stream(stream)
{

}

void QMixerStreamHandle::play()
{
    if (m_stream) {
        m_stream->play();
    }
}

void QMixerStreamHandle::pause()
{
    if (m_stream) {
        m_stream->pause();
    }
}

void QMixerStreamHandle::stop()
{
    if (m_stream) {
        m_stream->stop();
    }
}

QtMixer::State QMixerStreamHandle::state() const
{
    if (m_stream) {
        return m_stream->state();
    } else {
        return QtMixer::Unknown;
    }
}

int QMixerStreamHandle::loops() const
{
    if (m_stream) {
        return m_stream->loops();
    } else {
        return -1;
    }
}

void QMixerStreamHandle::setLoops(int loops)
{
    if (m_stream) {
        m_stream->setLoops(loops);
    }
}

// FIXME: always returns 0?!
int QMixerStreamHandle::position() const
{
    if (m_stream) {
        return m_stream->position();
    } else {
        return -1;
    }
}

void QMixerStreamHandle::setPosition(int position)
{
    if (m_stream) {
        m_stream->setPosition(position);
    }
}

bool QMixerStreamHandle::atEnd()
{
    return m_stream ? m_stream->done() : false;
}

int QMixerStreamHandle::length() const
{
    if (m_stream) {
        return m_stream->length();
    } else {
        return -1;
    }
}

bool QMixerStreamHandle::isValid() const
{
    return m_stream != nullptr;
}

bool QMixerStreamHandle::operator ==(const QMixerStreamHandle &other) const
{
    return other.m_stream == m_stream;
}
