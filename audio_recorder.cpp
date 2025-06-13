#include "audio_recorder.h"
#include <QAudioRecorder>
#include <QMediaPlayer>
#include <QFile>
#include <QTimer>
#include <QDebug>

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject(parent),
      m_audioRecorder(new QAudioRecorder(this)),
      m_mediaPlayer(new QMediaPlayer(this)),
      m_audioFile(nullptr),
      m_playbackTimer(new QTimer(this)),
      m_isRecording(false),
      m_isPlaying(false),
      m_isLooping(false),
      m_playbackDelay(2000) { // 2 seconds delay before playback

    connect(m_audioRecorder, &QAudioRecorder::stateChanged, this, [this](QMediaRecorder::State state) {
        if (state == QMediaRecorder::RecordingState) {
            emit recordingStarted();
        } else if (state == QMediaRecorder::StoppedState) {
            emit recordingStopped();
        }
    });

    connect(m_playbackTimer, &QTimer::timeout, this, &AudioRecorder::playbackTimeout);
}

AudioRecorder::~AudioRecorder() {
    delete m_audioRecorder;
    delete m_mediaPlayer;
    delete m_audioFile;
    delete m_playbackTimer;
}

bool AudioRecorder::isRecording() const {
    return m_isRecording;
}

void AudioRecorder::startRecording() {
    if (m_isRecording) {
        return;
    }

    m_isRecording = true;
    m_audioFile = new QFile("recording.wav", this);
    if (!m_audioFile->open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing";
        return;
    }

    m_audioRecorder->setOutputLocation(QUrl::fromLocalFile(m_audioFile->fileName()));
    m_audioRecorder->record();
}

void AudioRecorder::stopRecording() {
    if (!m_isRecording) {
        return;
    }

    m_isRecording = false;
    m_audioRecorder->stop();
    m_audioFile->close();
}

void AudioRecorder::playbackTimeout() {
    if (m_isPlaying) {
        return;
    }

    m_isPlaying = true;
    m_mediaPlayer->setMedia(QUrl::fromLocalFile(m_audioFile->fileName()));
    m_mediaPlayer->play();

    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            if (m_isLooping) {
                m_mediaPlayer->play();
            } else {
                m_isPlaying = false;
            }
        }
    });
}
