#pragma once

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "audio_recorder.h"

class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT

public:
    explicit TrayIcon(QObject *parent = nullptr);
    void setRecorder(AudioRecorder *recorder);

private slots:
    void toggleRecording();
    void quitApplication();

private:
    AudioRecorder *m_recorder;
    QAction *m_toggleAction;
    QAction *m_quitAction;
};
