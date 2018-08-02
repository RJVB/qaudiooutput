#ifndef QMIXERSTREAM_H
#define QMIXERSTREAM_H

#include <QIODevice>
#include <QAudioFormat>

#include "qtmixer.h"
#include "qmixerstreamhandle.h"

typedef std::numeric_limits<qint16> Range;

class QMixerStreamPrivate;

class QTMIXER_EXPORT QMixerStream : public QIODevice
{
    Q_OBJECT

public:
    QMixerStream(const QAudioFormat &format, QObject *parent=nullptr);
    ~QMixerStream();

    QMixerStreamHandle openStream(const QString &fileName);

    void closeStream(const QMixerStreamHandle &handle);

    // doesn't seem to work/possible?
    static QAudioFormat formatForFile(const QString &fileName);

    bool isValid();
    virtual bool atEnd() const override;
    virtual bool isSequential() const override;
    virtual void close() override;
    virtual qint64 size() const override;
    virtual qint64 bytesAvailable() const override { return size(); }

    bool appendable() const { return m_appendable; }
    void setAppendable(bool enabled = true);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    qint16 mix(qint32 sample1, qint32 sample2);

    QMixerStreamPrivate *d_ptr;

    bool m_appendable = false;

Q_SIGNALS:
    void stateChanged(QMixerStreamHandle handle, QtMixer::State state);
    void decodingFinished(QMixerStreamHandle handle);
};

#endif // QMIXERSTREAM_H
