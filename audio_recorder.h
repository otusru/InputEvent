#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioRecorder>
#include <QFile>
#include <QTimer>

class AudioRecorder : public QObject {
    Q_OBJECT

public:
    explicit AudioRecorder(QObject *parent = nullptr);
    ~AudioRecorder();

    bool isRecording() const;
    void startRecording();
    void stopRecording();

signals:
    void recordingStarted();
    void recordingStopped();

private:
    QAudioRecorder *m_audioRecorder;
    QMediaPlayer *m_mediaPlayer;
    QFile *m_audioFile;
    QTimer *m_playbackTimer;
    bool m_isRecording;
    bool m_isPlaying;
    bool m_isLooping;
    int m_playbackDelay;
};
