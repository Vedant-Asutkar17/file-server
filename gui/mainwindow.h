#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include "fileclient.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnect();
    void onUpload();
    void onDownload();
    void onDelete();
    void onRename();
    void onRefresh();
    void updateUploadProgress(int percent);
    void updateDownloadProgress(int percent);

private:
    // Client
    FileClient *client;

    // Connection widgets
    QLineEdit *hostInput;
    QLineEdit *portInput;
    QLineEdit *userInput;
    QLineEdit *passInput;
    QPushButton *connectBtn;

    // File list
    QListWidget *fileList;

    // Buttons
    QPushButton *uploadBtn;
    QPushButton *downloadBtn;
    QPushButton *deleteBtn;
    QPushButton *renameBtn;
    QPushButton *refreshBtn;

    // Status
    QLabel *statusLabel;
    QProgressBar *progressBar;

    // Helper
    void setButtonsEnabled(bool enabled);
    void refreshFileList();
    QString getSelectedFile();
};

#endif