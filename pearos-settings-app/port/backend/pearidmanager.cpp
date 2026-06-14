#include "pearidmanager.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

static QString findScriptDir() {
    // Installed path
    QString installed = "/usr/share/extras/system-settings/pearID";
    if (QDir(installed).exists()) return installed;
    // Source tree path (development)
    QString src = QDir::homePath() + "/Desktop/pkgbuilds/pearos-settings-app/system-settings/pearID";
    if (QDir(src).exists()) return src;
    return installed;
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
            emit stateChanged();
            fetchUserInfo();
        } else {
            m_state = "loggedout";
            emit stateChanged();
        }
    });
}

void PearIDManager::fetchUserInfo() {
    QString infoScript = m_scriptDir + "/get_user_info.sh";
    if (!QFile::exists(infoScript)) return;
    run("bash \"" + infoScript + "\" --first-name --last-name --email", [this](QString out, int) {
        QString fname, lname, email;
        for (const QString &line : out.split('\n')) {
            if (line.startsWith("First Name:")) fname = line.mid(11).trimmed();
            else if (line.startsWith("Last Name:"))  lname = line.mid(10).trimmed();
            else if (line.startsWith("Email:"))       email = line.mid(6).trimmed();
        }
        m_userName  = (fname + " " + lname).trimmed();
        if (m_userName.isEmpty()) m_userName = email;
        m_userEmail = email;
        emit userInfoChanged();
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
    QString cmd = "bash \"" + loginScript + "\" " +
                  "\"" + safeEmail + "\" " +
                  "\"" + safePassword + "\"";
    run(cmd, [this](QString out, int code) {
        bool ok = (code == 0) || out.contains("Authentication successful") || out.contains("Authenticated");
        if (ok) {
            m_state = "loggedin";
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
        m_userName.clear();
        m_userEmail.clear();
        emit stateChanged();
        emit userInfoChanged();
    };
    if (QFile::exists(exitScript)) {
        run("bash \"" + exitScript + "\"", finish);
    } else {
        finish({}, 0);
    }
}
