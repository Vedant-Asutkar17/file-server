#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QFile>
#include <QFileInfo>

class FileClient : public QObject {
    Q_OBJECT

public:
    explicit FileClient(QObject *parent = nullptr);

    // Connect to server
    bool connectToServer(QString host, int port);

    // Login
    QString login(QString username, QString password);

    // File operations
    QString listFiles();
    bool uploadFile(QString filepath);
    bool downloadFile(QString filename, QString savepath);
    bool deleteFile(QString filename);
    bool renameFile(QString oldname, QString newname);

    // Disconnect
    void disconnect();

    bool isConnected();

signals:
    void uploadProgress(int percent);
    void downloadProgress(int percent);

private:
    QTcpSocket *socket;

    // Send and receive helpers
    void sendMsg(QString msg);
    QString recvMsg();
    void sendRaw(const char* data, qint64 size);
};

#endif