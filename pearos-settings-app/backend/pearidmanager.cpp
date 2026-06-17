#include "pearidmanager.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QVariantMap>

static QString findScriptDir() {
    QString installed = "/usr/share/extras/system-settings/pearID";
    if (QDir(installed).exists()) return installed;
    QString src = QDir::homePath() + "/Desktop/pkgbuilds/pearos-settings-app/pearID";
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

    // Core info: first_name, last_name, email (plain lines, no labels)
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

    // Avatar download
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

void PearIDManager::fetchExtendedInfo() {
    QString infoScript = m_scriptDir + "/get_user_info.sh";
    if (!QFile::exists(infoScript)) return;

    // Script outputs in fixed order regardless of flag order: phone, billing, birthdate
    run("bash \"" + infoScript + "\" --phone --birthdate --billing-address", [this](QString out, int) {
        QStringList lines;
        for (const QString &l : out.split('\n')) {
            QString t = l.trimmed();
            if (!t.isEmpty()) lines << t;
        }
        m_phone          = lines.value(0);
        m_billingAddress = lines.value(1);
        m_birthdate      = lines.value(2);
        emit userInfoChanged();
    });
}

void PearIDManager::fetchDevices() {
    QString infoScript = m_scriptDir + "/get_user_info.sh";
    if (!QFile::exists(infoScript)) return;

    run("bash \"" + infoScript + "\" --devices", [this](QString out, int) {
        m_devices.clear();
        for (const QString &line : out.split('\n')) {
            QString t = line.trimmed();
            if (!t.isEmpty()) {
                QVariantMap entry;
                entry["name"] = t;
                m_devices.append(entry);
            }
        }
        emit devicesChanged();
    });
}

void PearIDManager::fetchApps() {
    QString infoScript = m_scriptDir + "/get_user_info.sh";
    if (!QFile::exists(infoScript)) return;

    run("bash \"" + infoScript + "\" --apps", [this](QString out, int) {
        m_apps.clear();
        for (const QString &line : out.split('\n')) {
            QString t = line.trimmed();
            if (!t.isEmpty()) {
                QVariantMap entry;
                entry["name"] = t;
                m_apps.append(entry);
            }
        }
        emit appsChanged();
    });
}

void PearIDManager::updateName(const QString &firstName, const QString &lastName) {
    QString updateScript = m_scriptDir + "/update_user_info.sh";
    if (!QFile::exists(updateScript)) {
        emit updateResult("name", false, "Script not found");
        return;
    }
    QString cmd = "bash \"" + updateScript + "\" --first-name \"" +
                  firstName.trimmed() + "\" --last-name \"" + lastName.trimmed() + "\"";
    run(cmd, [this, firstName, lastName](QString out, int code) {
        bool ok = (code == 0);
        if (ok) {
            m_userName = (firstName + " " + lastName).trimmed();
            emit userInfoChanged();
        }
        emit updateResult("name", ok, ok ? QString() : out.trimmed());
    });
}

void PearIDManager::updatePhone(const QString &phone) {
    QString updateScript = m_scriptDir + "/update_user_info.sh";
    if (!QFile::exists(updateScript)) {
        emit updateResult("phone", false, "Script not found");
        return;
    }
    run("bash \"" + updateScript + "\" --phone \"" + phone + "\"", [this, phone](QString out, int code) {
        bool ok = (code == 0);
        if (ok) {
            m_phone = phone;
            emit userInfoChanged();
        }
        emit updateResult("phone", ok, ok ? QString() : out.trimmed());
    });
}

void PearIDManager::updateBillingAddress(const QString &address) {
    QString updateScript = m_scriptDir + "/update_user_info.sh";
    if (!QFile::exists(updateScript)) {
        emit updateResult("billing", false, "Script not found");
        return;
    }
    run("bash \"" + updateScript + "\" --billing-address \"" + address + "\"", [this, address](QString out, int code) {
        bool ok = (code == 0);
        if (ok) {
            m_billingAddress = address;
            emit userInfoChanged();
        }
        emit updateResult("billing", ok, ok ? QString() : out.trimmed());
    });
}

void PearIDManager::changePassword(const QString &oldPw, const QString &newPw) {
    QString updateScript = m_scriptDir + "/update_user_info.sh";
    if (!QFile::exists(updateScript)) {
        emit updateResult("password", false, "Script not found");
        return;
    }
    QString safe_old = QString(oldPw).replace("\"", "\\\"");
    QString safe_new = QString(newPw).replace("\"", "\\\"");
    QString cmd = "bash \"" + updateScript + "\" --old-password \"" + safe_old + "\" --new-password \"" + safe_new + "\"";
    run(cmd, [this](QString out, int code) {
        bool ok = (code == 0);
        emit updateResult("password", ok, ok ? QString() : out.trimmed());
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
        m_avatarPath.clear();
        m_phone.clear();
        m_birthdate.clear();
        m_billingAddress.clear();
        m_devices.clear();
        m_apps.clear();
        emit stateChanged();
        emit userInfoChanged();
        emit devicesChanged();
        emit appsChanged();
    };
    if (QFile::exists(exitScript)) {
        run("bash \"" + exitScript + "\"", finish);
    } else {
        finish({}, 0);
    }
}
