#include "tray_icon.h"
#include <QApplication>
#include <QIcon>

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent),
      m_recorder(nullptr),
      m_toggleAction(new QAction("Start Recording", this)),
      m_quitAction(new QAction("Quit", this)) {

    QMenu *menu = new QMenu();
    menu->addAction(m_toggleAction);
    menu->addAction(m_quitAction);

    setContextMenu(menu);
    setIcon(QIcon::fromTheme("audio-input-microphone"));

    connect(m_toggleAction, &QAction::triggered, this, &TrayIcon::toggleRecording);
    connect(m_quitAction, &QAction::triggered, this, &TrayIcon::quitApplication);
}

void TrayIcon::setRecorder(AudioRecorder *recorder) {
    m_recorder = recorder;
    connect(m_recorder, &AudioRecorder::recordingStarted, this, [this] {
        m_toggleAction->setText("Stop Recording");
    });
    connect(m_recorder, &AudioRecorder::recordingStopped, this, [this] {
        m_toggleAction->setText("Start Recording");
    });
}

void TrayIcon::toggleRecording() {
    if (m_recorder->isRecording()) {
        m_recorder->stopRecording();
    } else {
        m_recorder->startRecording();
    }
}

void TrayIcon::quitApplication() {
    QApplication::quit();
}
