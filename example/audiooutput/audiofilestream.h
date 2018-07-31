// https://stackoverflow.com/questions/41197576/how-to-play-mp3-file-using-qaudiooutput-and-qaudiodecoder

#ifndef AUDIOFILESTREAM_H

#include <QIODevice>
#include <QBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QFile>

// Class for decoding audio files and pushing the decoded audio data to a QOutputDevice (e.g. a speaker).
// Decoding to the desired format is done via QAudioDecoder and QAudioFormat.
// based on: https://github.com/Znurre/QtMixer
class AudioFileStream : public QIODevice
{
    Q_OBJECT

public:
    AudioFileStream();
    bool init(const QAudioFormat& format);

    enum State { Playing, Stopped };

    void play(const QString &filePath);
    void stop();

    bool atEnd() const override;

protected:
    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char* data, qint64 len) override;

private:
    QFile m_file;
    QBuffer m_input;
    QBuffer m_output;
    QByteArray m_data;
    QAudioDecoder m_decoder;
    QAudioFormat m_format;

    State m_state;

    bool isInited;
    bool isDecodingFinished;

    void clear();

private slots:
    void bufferReady();
    void finished();

signals:
    void stateChanged(AudioFileStream::State state);
    void newData(const QByteArray& data);
};

#define AUDIOFILESTREAM_H
#endif // AUDIOFILESTREAM_H
