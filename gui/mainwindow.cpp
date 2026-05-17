#include "mainwindow.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

    client = new FileClient(this);
    connect(client, &FileClient::uploadProgress, this, &MainWindow::updateUploadProgress);
    connect(client, &FileClient::downloadProgress, this, &MainWindow::updateDownloadProgress);

    // Main widget
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ── Connection Group ──────────────────────────────
    QGroupBox *connGroup = new QGroupBox("Server Connection");
    QGridLayout *connLayout = new QGridLayout(connGroup);

    hostInput = new QLineEdit("localhost");
    portInput = new QLineEdit("9999");
    userInput = new QLineEdit("admin");
    passInput = new QLineEdit();
    passInput->setEchoMode(QLineEdit::Password);
    passInput->setPlaceholderText("password");
    connectBtn = new QPushButton("Connect");

    connLayout->addWidget(new QLabel("Host:"), 0, 0);
    connLayout->addWidget(hostInput, 0, 1);
    connLayout->addWidget(new QLabel("Port:"), 0, 2);
    connLayout->addWidget(portInput, 0, 3);
    connLayout->addWidget(new QLabel("Username:"), 1, 0);
    connLayout->addWidget(userInput, 1, 1);
    connLayout->addWidget(new QLabel("Password:"), 1, 2);
    connLayout->addWidget(passInput, 1, 3);
    connLayout->addWidget(connectBtn, 2, 0, 1, 4);

    mainLayout->addWidget(connGroup);

    // ── File List ─────────────────────────────────────
    QGroupBox *fileGroup = new QGroupBox("Files on Server");
    QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);
    fileList = new QListWidget();
    fileList->setMinimumHeight(200);
    fileLayout->addWidget(fileList);
    mainLayout->addWidget(fileGroup);

    // ── Buttons ───────────────────────────────────────
    QHBoxLayout *btnLayout = new QHBoxLayout();
    uploadBtn   = new QPushButton("⬆ Upload");
    downloadBtn = new QPushButton("⬇ Download");
    deleteBtn   = new QPushButton("🗑 Delete");
    renameBtn   = new QPushButton("✏ Rename");
    refreshBtn  = new QPushButton("↻ Refresh");

    btnLayout->addWidget(uploadBtn);
    btnLayout->addWidget(downloadBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(renameBtn);
    btnLayout->addWidget(refreshBtn);
    mainLayout->addLayout(btnLayout);

    // ── Progress Bar ──────────────────────────────────
    progressBar = new QProgressBar();
    progressBar->setValue(0);
    mainLayout->addWidget(progressBar);

    // ── Status Bar ────────────────────────────────────
    statusLabel = new QLabel("Not connected");
    mainLayout->addWidget(statusLabel);

    // Disable buttons until connected
    setButtonsEnabled(false);

    // Connect signals
    connect(connectBtn, &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(uploadBtn,  &QPushButton::clicked, this, &MainWindow::onUpload);
    connect(downloadBtn,&QPushButton::clicked, this, &MainWindow::onDownload);
    connect(deleteBtn,  &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(renameBtn,  &QPushButton::clicked, this, &MainWindow::onRename);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefresh);

    // Window settings
    setWindowTitle("File Server Client");
    setMinimumSize(600, 500);
}

MainWindow::~MainWindow() {}

void MainWindow::setButtonsEnabled(bool enabled) {
    uploadBtn->setEnabled(enabled);
    downloadBtn->setEnabled(enabled);
    deleteBtn->setEnabled(enabled);
    renameBtn->setEnabled(enabled);
    refreshBtn->setEnabled(enabled);
}

void MainWindow::onConnect() {
    QString host = hostInput->text();
    int port = portInput->text().toInt();
    QString user = userInput->text();
    QString pass = passInput->text();

    if (!client->connectToServer(host, port)) {
        statusLabel->setText("Connection failed!");
        return;
    }

    QString result = client->login(user, pass);

    if (result.contains("Welcome")) {
        statusLabel->setText("Connected as: " + user);
        connectBtn->setEnabled(false);
        setButtonsEnabled(true);
        refreshFileList();
    } else {
        statusLabel->setText("Login failed: " + result);
        client->disconnect();
    }
}

void MainWindow::refreshFileList() {
    fileList->clear();
    QString files = client->listFiles();
    QStringList lines = files.split("\n", Qt::SkipEmptyParts);
    for (QString line : lines) {
        if (!line.trimmed().isEmpty()) {
            fileList->addItem(line.trimmed());
        }
    }
    statusLabel->setText("File list refreshed");
}

void MainWindow::onRefresh() {
    refreshFileList();
}

QString MainWindow::getSelectedFile() {
    QListWidgetItem *item = fileList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Select File", "Please select a file first.");
        return "";
    }
    // Extract filename — format is "filename (size bytes)"
    QString text = item->text();
    return text.split(" ").first();
}

void MainWindow::onUpload() {
    QString filepath = QFileDialog::getOpenFileName(
        this, "Select file to upload");
    if (filepath.isEmpty()) return;

    statusLabel->setText("Uploading...");
    progressBar->setValue(0);

    bool success = client->uploadFile(filepath);

    if (success) {
        statusLabel->setText("Upload successful!");
        refreshFileList();
    } else {
        statusLabel->setText("Upload failed.");
    }
    progressBar->setValue(0);
}

void MainWindow::onDownload() {
    QString filename = getSelectedFile();
    if (filename.isEmpty()) return;

    QString savepath = QFileDialog::getSaveFileName(
        this, "Save file as", filename);
    if (savepath.isEmpty()) return;

    statusLabel->setText("Downloading...");
    progressBar->setValue(0);

    bool success = client->downloadFile(filename, savepath);

    if (success) {
        statusLabel->setText("Download complete: " + savepath);
    } else {
        statusLabel->setText("Download failed.");
    }
    progressBar->setValue(0);
}

void MainWindow::onDelete() {
    QString filename = getSelectedFile();
    if (filename.isEmpty()) return;

    int confirm = QMessageBox::question(
        this, "Confirm Delete",
        "Delete " + filename + "?",
        QMessageBox::Yes | QMessageBox::No);

    if (confirm != QMessageBox::Yes) return;

    bool success = client->deleteFile(filename);
    if (success) {
        statusLabel->setText("Deleted: " + filename);
        refreshFileList();
    } else {
        statusLabel->setText("Delete failed.");
    }
}

void MainWindow::onRename() {
    QString filename = getSelectedFile();
    if (filename.isEmpty()) return;

    QString newname = QInputDialog::getText(
        this, "Rename File",
        "New name for " + filename + ":");
    if (newname.isEmpty()) return;

    bool success = client->renameFile(filename, newname);
    if (success) {
        statusLabel->setText("Renamed to: " + newname);
        refreshFileList();
    } else {
        statusLabel->setText("Rename failed.");
    }
}

void MainWindow::updateUploadProgress(int percent) {
    progressBar->setValue(percent);
}

void MainWindow::updateDownloadProgress(int percent) {
    progressBar->setValue(percent);
}