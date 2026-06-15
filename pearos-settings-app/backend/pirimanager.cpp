#include "pirimanager.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QNetworkRequest>

const QString PiriManager::MODEL_URL      = "https://cdn.pearos.xyz/vosk-model-en-us-0.42-gigaspeech.zip";
const QString PiriManager::MODEL_DIR      = "/usr/share/extras/piri/model";
const QString PiriManager::ZIP_TMP        = "/tmp/piri-model.zip";
const QString PiriManager::EXTRACT_TMP    = "/tmp/piri-extract";
const QString PiriManager::SHOW_ICON_FILE = "/usr/share/extras/piri/show_icon";

PiriManager::PiriManager(QObject *parent) : QObject(parent) {
    checkModelExists();
    QFile f(SHOW_ICON_FILE);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        m_showIcon = QString::fromUtf8(f.readAll()).trimmed().toLower() == "true";
}

void PiriManager::checkModelExists() {
    QDir dir(MODEL_DIR);
    m_modelExists = dir.exists() && !dir.entryList(QDir::Files | QDir::NoDotAndDotDot).isEmpty();
    emit statusChanged();
}

void PiriManager::setShowIcon(bool v) {
    if (m_showIcon == v) return;
    m_showIcon = v;
    QDir().mkpath(QFileInfo(SHOW_ICON_FILE).absolutePath());
    QFile f(SHOW_ICON_FILE);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        f.write(v ? "true" : "false");
    emit showIconChanged();
}

void PiriManager::downloadModel() {
    if (m_downloading) return;
    m_downloading = true;
    m_progress    = 0;
    m_statusText  = "Downloading…";
    m_errorText.clear();
    emit statusChanged();

    QUrl url(MODEL_URL);
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m_nam.get(req);

    auto *file = new QFile(ZIP_TMP, m_reply);
    if (!file->open(QIODevice::WriteOnly)) {
        m_reply->abort();
        m_downloading = false;
        m_errorText = "Cannot write to " + ZIP_TMP;
        emit statusChanged();
        return;
    }

    connect(m_reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
        if (total > 0) {
            m_progress   = (int)(received * 100 / total);
            m_statusText = QString("Downloading… %1%").arg(m_progress);
            emit statusChanged();
        }
    });

    connect(m_reply, &QNetworkReply::readyRead, this, [this, file]() {
        if (m_reply) file->write(m_reply->readAll());
    });

    connect(m_reply, &QNetworkReply::finished, this, [this, file]() {
        file->close();
        if (!m_reply) return;
        auto err = m_reply->error();
        m_reply->deleteLater();
        m_reply = nullptr;

        if (err != QNetworkReply::NoError) {
            if (err != QNetworkReply::OperationCanceledError)
                m_errorText = "Download failed";
            m_downloading = false;
            emit statusChanged();
            return;
        }

        m_progress   = 100;
        m_statusText = "Extracting…";
        emit statusChanged();
        extractAndInstall();
    });
}

void PiriManager::extractAndInstall() {
    QString script = QString(
        "pkexec bash -c '"
        "mkdir -p \"%1\" \"%2\" && "
        "( (command -v unzip >/dev/null 2>&1 && unzip -o \"%3\" -d \"%2\") || "
        "  (command -v bsdtar >/dev/null 2>&1 && bsdtar -xf \"%3\" -C \"%2\") ) && "
        "( count=$(ls -1d \"%2\"/*/ 2>/dev/null | wc -l); "
        "  if [ \"$count\" -eq 1 ]; then cp -r \"%2\"/*/. \"%1\"/; "
        "  else cp -r \"%2\"/* \"%1\"/; fi ) && "
        "rm -rf \"%2\" \"%3\"'"
    ).arg(MODEL_DIR, EXTRACT_TMP, ZIP_TMP);

    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [this, proc](int code, QProcess::ExitStatus) {
        proc->deleteLater();
        m_downloading = false;
        if (code == 0) {
            m_progress   = 100;
            m_statusText = "Installed";
            m_modelExists = true;
        } else {
            m_errorText = "Extraction failed: " +
                          QString::fromUtf8(proc->readAllStandardError()).trimmed();
        }
        emit statusChanged();
    });
    proc->start("bash", {"-c", script});
}

void PiriManager::cancelDownload() {
    if (m_reply) {
        m_reply->abort();
        m_reply = nullptr;
    }
    m_downloading = false;
    m_progress    = 0;
    m_statusText.clear();
    m_errorText.clear();
    QFile::remove(ZIP_TMP);
    emit statusChanged();
}

void PiriManager::removeModel() {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [this, proc](int, QProcess::ExitStatus) {
        proc->deleteLater();
        checkModelExists();
    });
    proc->start("pkexec", {"bash", "-c",
        QString("rm -rf \"%1\"/*").arg(MODEL_DIR)});
}
