#include "pearidmanager.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QCoreApplication>

// Persisted "was last known logged in" marker, so a network hiccup on the
// very first check of a session doesn't have to guess between logged-in and
// logged-out — it can fall back to what was last confirmed by the server.
static QString lastOkMarkerPath() {
    QString dir = QDir::homePath() + "/.cache/pearos-contacts";
    QDir().mkpath(dir);
    return dir + "/pearid_last_ok";
}

static QString findScriptDir() {
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList candidates = {
        appDir.absoluteFilePath("../pearID"),
        appDir.absoluteFilePath("pearID"),
        "/usr/share/pearos-contacts/pearID",
        "/usr/share/pearos-appstore/pearID"
    };
    for (const QString &path : candidates) {
        if (QDir(path).exists()) return path;
    }
    return candidates.last();
}

PearIDManager::PearIDManager(QObject *parent) : QObject(parent) {
    m_scriptDir = findScriptDir();
}

void PearIDManager::run(const QString &cmd, std::function<void(QString, int)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int code, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()), code);
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, this, [proc, cb](QProcess::ProcessError) {
        cb(QString(), -1);
        proc->deleteLater();
    });
    proc->start("bash", {"-c", cmd});
}

void PearIDManager::checkState() {
    m_state = "loading";
    emit stateChanged();
    QString stateScript = m_scriptDir + "/state.sh";
    if (!QFile::exists(stateScript)) {
        m_state = "loggedout";
        emit stateChanged();
        return;
    }
    run("bash \"" + stateScript + "\"", [this](QString out, int) {
        QString result = out.trimmed();
        if (result == "true") {
            m_state = "loggedin";
            QFile marker(lastOkMarkerPath());
            (void)marker.open(QIODevice::WriteOnly);
            emit stateChanged();
            fetchUserInfo();
        } else if (result == "false") {
            // Explicit rejection from the server (or no token at all) — really logged out.
            m_state = "loggedout";
            QFile::remove(lastOkMarkerPath());
            emit stateChanged();
        } else {
            // "300" (no network) / "500" (server error) / anything else: state.sh
            // couldn't actually verify the token either way. Keep whatever we last
            // knew to be true instead of flipping a valid session to "logged out"
            // on a slow API response or DNS hiccup.
            if (m_state == "loading") {
                m_state = QFile::exists(lastOkMarkerPath()) ? "loggedin" : "loggedout";
            }
            emit stateChanged();
        }
    });
}

void PearIDManager::fetchUserInfo() {
    QString infoScript = m_scriptDir + "/get_user_info.sh";
    if (!QFile::exists(infoScript)) return;

    run("bash \"" + infoScript + "\" --first-name --last-name --email", [this](QString out, int) {
        QStringList lines;
        for (const QString &l : out.split('\n')) {
            QString t = l.trimmed();
            if (!t.isEmpty()) lines << t;
        }
        m_userName  = (lines.value(0) + " " + lines.value(1)).trimmed();
        if (m_userName.isEmpty()) m_userName = lines.value(2);
        m_userEmail = lines.value(2);
        emit userInfoChanged();
    });

    run("bash \"" + infoScript + "\" --avatar 2>/dev/null", [this](QString, int code) {
        if (code == 0) {
            QString path = QDir::homePath() + "/.pearid_avatars/avatar.webp";
            if (QFile::exists(path)) {
                m_avatarPath = path;
                emit userInfoChanged();
            }
        }
    });
}

void PearIDManager::login(const QString &email, const QString &password) {
    QString loginScript = m_scriptDir + "/login_and_sync.sh";
    if (!QFile::exists(loginScript)) {
        emit loginResult(false, "Login script not found");
        return;
    }
    QString safeEmail    = QString(email).replace("\"", "\\\"");
    QString safePassword = QString(password).replace("\"", "\\\"");
    run("bash \"" + loginScript + "\" \"" + safeEmail + "\" \"" + safePassword + "\"",
        [this](QString out, int code) {
        bool ok = (code == 0) || out.contains("Authentication successful") || out.contains("Authenticated");
        if (ok) {
            m_state = "loggedin";
            QFile marker(lastOkMarkerPath());
            (void)marker.open(QIODevice::WriteOnly);
            emit stateChanged();
            fetchUserInfo();
        }
        emit loginResult(ok, ok ? QString() : "Login failed. Check your credentials.");
    });
}

void PearIDManager::logout() {
    QString exitScript = m_scriptDir + "/exit.sh";
    auto finish = [this](QString, int) {
        m_state = "loggedout";
        QFile::remove(lastOkMarkerPath());
        m_userName.clear();
        m_userEmail.clear();
        m_avatarPath.clear();
        emit stateChanged();
        emit userInfoChanged();
    };
    if (QFile::exists(exitScript)) {
        run("bash \"" + exitScript + "\"", finish);
    } else {
        finish({}, 0);
    }
}
