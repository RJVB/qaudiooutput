/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QAudioDecoder>
#include <QDebug>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QEventLoop>
#include <QTimer>
#include <QElapsedTimer>
#include <qmath.h>
#include <qendian.h>

#include "audiooutput.h"

#define PUSH_MODE_LABEL "Enable push mode"
#define PULL_MODE_LABEL "Enable pull mode"
#define SUSPEND_LABEL   "Suspend playback"
#define RESUME_LABEL    "Resume playback"
#define VOLUME_LABEL    "Volume:"

const int DurationSeconds = 1;
const int ToneSampleRateHz = 392;
const int DataSampleRateHz = 44100;
const int BufferSize      = 32768;

class QSlumber
{
public:
    QSlumber(qreal seconds)
    {
        if (seconds > 0) {
            QEventLoop loop;
            QTimer::singleShot(seconds * 1000, &loop, SLOT(quit()));
            loop.exec();
        }
    }
    static qreal during(qreal seconds)
    {
        QElapsedTimer duration;
        duration.start();
        QSlumber sleep(seconds);
        return duration.elapsed() / 1000.0;
    }
};

Generator::Generator(const QAudioFormat &format,
                     qint64 durationUs,
                     int sampleRate,
                     QObject *parent)
    :   QIODevice(parent)
    ,   m_pos(0)
{
    if (format.isValid())
        generateData(format, durationUs, sampleRate);
}

Generator::~Generator()
{

}

void Generator::start()
{
    open(QIODevice::ReadOnly);
}

void Generator::stop()
{
    m_pos = 0;
    close();
}

void Generator::generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate)
{
    const int channelBytes = format.sampleSize() / 8;
    const int sampleBytes = format.channelCount() * channelBytes;

    qint64 length = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
                        * durationUs / 100000;

    Q_ASSERT(length % sampleBytes == 0);
    Q_UNUSED(sampleBytes) // suppress warning in release builds

    m_buffer.resize(length);
    unsigned char *ptr = reinterpret_cast<unsigned char *>(m_buffer.data());
    int sampleIndex = 0;

    while (length) {
        const qreal x = qSin(2 * M_PI * sampleRate * qreal(sampleIndex % format.sampleRate()) / format.sampleRate());
        for (int i=0; i<format.channelCount(); ++i) {
            if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) {
                const quint8 value = static_cast<quint8>((1.0 + x) / 2 * 255);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::SignedInt) {
                const qint8 value = static_cast<qint8>(x * 127);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::UnSignedInt) {
                quint16 value = static_cast<quint16>((1.0 + x) / 2 * 65535);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<quint16>(value, ptr);
                else
                    qToBigEndian<quint16>(value, ptr);
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
                qint16 value = static_cast<qint16>(x * 32767);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<qint16>(value, ptr);
                else
                    qToBigEndian<qint16>(value, ptr);
            }

            ptr += channelBytes;
            length -= channelBytes;
        }
        ++sampleIndex;
    }
}

qint64 Generator::readData(char *data, qint64 len)
{
    qint64 total = 0;
    if (!m_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
            memcpy(data + total, m_buffer.constData() + m_pos, chunk);
            m_pos = (m_pos + chunk) % m_buffer.size();
            total += chunk;
        }
    }
    return total;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 Generator::bytesAvailable() const
{
    return m_buffer.size() + QIODevice::bytesAvailable();
}

AudioTest::AudioTest()
    :   m_pushTimer(new QTimer(this))
    ,   m_modeButton(0)
    ,   m_suspendResumeButton(0)
    ,   m_deviceBox(0)
    ,   m_device(QAudioDeviceInfo::defaultOutputDevice())
    ,   m_generator(0)
    ,   m_audioOutput(0)
    ,   m_output(0)
    ,   m_pullMode(true)
    ,   m_buffer(BufferSize, 0)
    ,   m_fileStream(nullptr)
{
    initializeWindow();
    initializeAudio();
}

void AudioTest::initializeWindow()
{
    QScopedPointer<QWidget> window(new QWidget);
    QScopedPointer<QVBoxLayout> layout(new QVBoxLayout);

    m_deviceBox = new QComboBox(this);
    const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    m_deviceBox->addItem(defaultDeviceInfo.deviceName(), qVariantFromValue(defaultDeviceInfo));
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        if (deviceInfo != defaultDeviceInfo)
            m_deviceBox->addItem(deviceInfo.deviceName(), qVariantFromValue(deviceInfo));
    }
    connect(m_deviceBox,SIGNAL(activated(int)),SLOT(deviceChanged(int)));
    layout->addWidget(m_deviceBox);

    m_modeButton = new QPushButton(this);
    m_modeButton->setText(tr(PUSH_MODE_LABEL));
    connect(m_modeButton, SIGNAL(clicked()), SLOT(toggleMode()));
    layout->addWidget(m_modeButton);

    m_suspendResumeButton = new QPushButton(this);
    m_suspendResumeButton->setText(tr(SUSPEND_LABEL));
    connect(m_suspendResumeButton, SIGNAL(clicked()), SLOT(toggleSuspendResume()));
    layout->addWidget(m_suspendResumeButton);

    m_playFile = new QPushButton(this);
    m_playFile->setText(tr("Play File"));
    connect(m_playFile, SIGNAL(clicked()), SLOT(playFile()));
    layout->addWidget(m_playFile);

    QHBoxLayout *volumeBox = new QHBoxLayout;
    m_volumeLabel = new QLabel;
    m_volumeLabel->setText(tr(VOLUME_LABEL));
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(100);
    m_volumeSlider->setSingleStep(10);
    connect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(volumeChanged(int)));
    volumeBox->addWidget(m_volumeLabel);
    volumeBox->addWidget(m_volumeSlider);
    layout->addLayout(volumeBox);

    window->setLayout(layout.data());
    layout.take(); // ownership transferred

    setCentralWidget(window.data());
    QWidget *const windowPtr = window.take(); // ownership transferred
    windowPtr->show();
}

void AudioTest::initializeAudio()
{
    connect(m_pushTimer, SIGNAL(timeout()), SLOT(pushTimerExpired()));

    m_format.setSampleRate(DataSampleRateHz);
    m_format.setChannelCount(2);
    m_format.setSampleSize(16);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(m_device);
    if (!info.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        m_format = info.nearestFormat(m_format);
    }

    if (m_generator)
        delete m_generator;
    m_generator = new Generator(m_format, DurationSeconds*1000000, ToneSampleRateHz, this);

    createAudioOutput();
}

void AudioTest::createAudioOutput()
{
    if (m_audioOutput) {
        m_audioOutput->deleteLater();
    }
    m_audioOutput = new QAudioOutput(m_device, m_format, this);
    m_generator->start();
    m_audioOutput->start(m_generator);

    qreal initialVolume = QAudio::convertVolume(m_audioOutput->volume(),
                                                QAudio::LinearVolumeScale,
                                                QAudio::LogarithmicVolumeScale);
    m_volumeSlider->setValue(qRound(initialVolume * 100));

    m_audioOutput->suspend();
    m_suspendResumeButton->setText(tr(RESUME_LABEL));
}

AudioTest::~AudioTest()
{
}

void AudioTest::deviceChanged(int index)
{
    const auto currentState = m_audioOutput->state();
    m_pushTimer->stop();
    m_generator->stop();
    m_audioOutput->stop();
    m_audioOutput->disconnect(this);
    m_device = m_deviceBox->itemData(index).value<QAudioDeviceInfo>();
    initializeAudio();
    if (currentState == QAudio::ActiveState && m_fileName.isEmpty()) {
        toggleSuspendResume();
    }
}

void AudioTest::volumeChanged(int value)
{
    if (m_volumeSlider->value() != value) {
        m_volumeSlider->setValue(value);
    } else if (m_audioOutput) {
        qreal linearVolume =  QAudio::convertVolume(value / qreal(100),
                                                    QAudio::LogarithmicVolumeScale,
                                                    QAudio::LinearVolumeScale);

        qWarning() << "New volume:" << linearVolume;
        m_audioOutput->setVolume(linearVolume);
    }
}

void AudioTest::pushTimerExpired()
{
    if (m_audioOutput && m_audioOutput->state() != QAudio::StoppedState) {
        int chunks = m_audioOutput->bytesFree()/m_audioOutput->periodSize();
        while (chunks) {
           const qint64 len = m_generator->read(m_buffer.data(), m_audioOutput->periodSize());
           if (len)
               m_output->write(m_buffer.data(), len);
           if (len != m_audioOutput->periodSize())
               break;
           --chunks;
        }
    }
}

void AudioTest::toggleMode()
{
    m_pushTimer->stop();
    m_audioOutput->stop();

    if (m_pullMode) {
        //switch to push mode (periodically push to QAudioOutput using a timer)
        m_modeButton->setText(tr(PULL_MODE_LABEL));
        m_output = m_audioOutput->start();
        m_pullMode = false;
        m_pushTimer->start(20);
    } else {
        //switch to pull mode (QAudioOutput pulls from Generator as needed)
        m_modeButton->setText(tr(PUSH_MODE_LABEL));
        m_pullMode = true;
        m_audioOutput->start(m_generator);
    }

    m_audioOutput->suspend();
    m_suspendResumeButton->setText(tr(RESUME_LABEL));
}

void AudioTest::toggleSuspendResume()
{
    if (m_audioOutput->state() == QAudio::SuspendedState) {
        m_audioOutput->resume();
        m_suspendResumeButton->setText(tr(SUSPEND_LABEL));
    } else if (m_audioOutput->state() == QAudio::ActiveState) {
        m_audioOutput->suspend();
        m_suspendResumeButton->setText(tr(RESUME_LABEL));
    } else if (m_audioOutput->state() == QAudio::StoppedState) {
        m_audioOutput->resume();
        m_suspendResumeButton->setText(tr(SUSPEND_LABEL));
    } else if (m_audioOutput->state() == QAudio::IdleState) {
        // no-op
    }
}

void AudioTest::playFile()
{
    auto volume = m_volumeSlider->value();
    if (m_fileName.isEmpty()) {
        m_fileName = QFileDialog::getOpenFileName(this, tr("Choose Audio File"));
        if (!m_fileName.isEmpty()) {
            QAudioFormat audioFormat = m_device.preferredFormat();
            audioFormat.setSampleSize(16);
            if (m_audioOutput) {
                m_audioOutput->deleteLater();
            }
            auto newAudioOut = new QAudioOutput(m_device, audioFormat, this);
            // create a new QMixerStream with the new QAudioOutput device as its parent
            // which means we won't have to delete it ourselves (and risk race conditions
            // where the output device thinks the stream is still available).
            if ((m_fileStream = new QMixerStream(audioFormat, newAudioOut))) {
                m_fileStream->setAppendable(false);
                m_filePlay = m_fileStream->openStream(m_fileName);
                if (m_filePlay.isValid()) {
                    m_fileStream->setObjectName(m_fileName);
                    m_generator->stop();
                    m_pushTimer->stop();
                    delete m_audioOutput;
                    m_audioOutput = newAudioOut;
                    volumeChanged(volume);
                    connect(m_fileStream, &QMixerStream::stateChanged, this, &AudioTest::qMixerStateChanged);
                    m_modeButton->setEnabled(false);
                    qWarning() << "Starting playback of" << m_fileName
                        << "via stream" << m_fileStream
                        << audioFormat;
                    m_playFile->setText(QFileInfo(m_fileName).fileName());
                    m_audioOutput->start(m_fileStream);
                    m_filePlay.play();
                } else {
                    delete newAudioOut;
                    newAudioOut = nullptr;
                }
            } else {
                delete newAudioOut;
                newAudioOut = nullptr;
            }
            if (!newAudioOut) {
                m_fileName.clear();
            }
        }
    } else {
        m_fileName.clear();
        m_playFile->setText(tr("Play File"));
        // stop file playback, re-enable generator stuff
        if (m_filePlay.state() != QtMixer::Stopped) {
            qWarning() << "Stopping playback of" << m_fileName;
            m_filePlay.stop();
        }
        qWarning() << "Stopping" << m_audioOutput;
        m_audioOutput->stop();
        m_audioOutput->reset();
//         if (!m_nullStream) {
//             m_nullStream = new QMixerStream(m_format, this);
//             m_nullStream->setObjectName(QStringLiteral("NULL stream"));
//         }
//         // starting the audio device on a new ("null") input stream appears to
//         // be the only reliable way to prevent the underlying output plugin to
//         // try and read data from the stream after the QAudioOutput instance
//         // and thus the stream have been
//         m_audioOutput->start(m_nullStream);
        qWarning() << "Closing stream";
        m_fileStream->close();
        // m_fileStream could be deleted here via deleteLater but
        // handing off ownership to m_audioOutput is an even better
        // solution to prevent the QAudioOutput backend from reading
        // from a stale QMixerStream instance.
        m_fileStream = nullptr;
        qWarning() << "Restoring generator output mode";
        m_modeButton->setEnabled(true);
        createAudioOutput();
        // simulate a mode toggle to reactivate generator sound output
        toggleSuspendResume();
        m_pullMode = !m_pullMode;
        toggleMode();
        qWarning() << "restoring volume to" << volume;
        volumeChanged(volume);
    }
}

void AudioTest::qMixerStateChanged(QMixerStreamHandle handle, QtMixer::State state)
{
    qWarning() << "QtMixer state changed to" << state << "at position" << handle.position() << "of" <<handle.length() << "atEnd=" << handle.atEnd();
    if (state == QtMixer::Stopped && handle.atEnd() && !m_fileName.isEmpty()) {
        // playback terminated
//         qWarning() << "Extra" << QSlumber::during(2) << "seconds to allow playback to finish up";
        playFile();
    }
}
