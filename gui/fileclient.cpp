#include "fileclient.h"
#include <QThread>

FileClient::FileClient(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);
}

bool FileClient::connectToServer(QString host, int port) {
    socket->connectToHost(host, port);
    return socket->waitForConnected(5000);
}

bool FileClient::isConnected() {
    return socket->state() == QAbstractSocket::ConnectedState;
}

void FileClient::disconnect() {
    socket->disconnectFromHost();
}

void FileClient::sendMsg(QString msg) {
    socket->write(msg.toUtf8());
    socket->flush();
    socket->waitForBytesWritten(3000);
}

QString FileClient::recvMsg() {
    socket->waitForReadyRead(5000);
    QByteArray data = socket->readAll();
    return QString::fromUtf8(data);
}

QString FileClient::login(QString username, QString password) {
    // Receive "=== File Server ==="
    recvMsg();

    // Receive "Username: "
    recvMsg();
    sendMsg(username);

    // Receive "Password: "
    recvMsg();
    sendMsg(password);

    // Receive result
    QString result = recvMsg();
    return result;
}

QString FileClient::listFiles() {
    sendMsg("1");
    QString files = recvMsg();
    return files;
}

bool FileClient::uploadFile(QString filepath) {
    sendMsg("2");

    // Wait for UPLOAD_START
    QString signal = recvMsg();
    if (!signal.contains("UPLOAD_START")) {
        return false;
    }

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        sendMsg("CANCEL");
        QThread::msleep(100);
        sendMsg("0");
        return false;
    }

    QFileInfo info(filepath);
    QString filename = info.fileName();
    qint64 filesize = info.size();

    // Send filename and size
    sendMsg(filename);
    QThread::msleep(200);
    sendMsg(QString::number(filesize));
    QThread::msleep(200);

    // Wait for READY
    QString ready = recvMsg();
    if (!ready.contains("READY")) {
        return false;
    }

    // Send file in chunks
    char buffer[4096];
    qint64 sent = 0;
    while (sent < filesize) {
        qint64 n = file.read(buffer, sizeof(buffer));
        if (n <= 0) break;
        socket->write(buffer, n);
        socket->flush();
        socket->waitForBytesWritten(3000);
        sent += n;
        emit uploadProgress((int)(sent * 100 / filesize));
    }
    file.close();

    // Get confirmation
    QString result = recvMsg();
    return result.contains("successful");
}

bool FileClient::downloadFile(QString filename, QString savepath) {
    sendMsg("3");

    // Wait for "Enter filename to download: "
    recvMsg();
    sendMsg(filename);

    // Get file info
    QString info = recvMsg();
    if (info.contains("ERROR")) {
        return false;
    }

    // Parse filesize — format "OK:12345"
    qint64 filesize = info.mid(3).toLongLong();

    // Receive file
    QFile file(savepath);
    file.open(QIODevice::WriteOnly);
    qint64 received = 0;

    while (received < filesize) {
        socket->waitForReadyRead(5000);
        QByteArray chunk = socket->read(filesize - received);
        file.write(chunk);
        received += chunk.size();
        emit downloadProgress((int)(received * 100 / filesize));
    }

    file.close();
    return true;
}

bool FileClient::deleteFile(QString filename) {
    sendMsg("4");

    // Wait for "Enter filename to delete: "
    recvMsg();
    sendMsg(filename);

    QString result = recvMsg();
    return result.contains("successfully");
}

bool FileClient::renameFile(QString oldname, QString newname) {
    sendMsg("5");

    // Wait for "Enter current filename: "
    recvMsg();
    sendMsg(oldname);

    // Wait for "Enter new filename: "
    recvMsg();
    sendMsg(newname);

    QString result = recvMsg();
    return result.contains("successfully");
}