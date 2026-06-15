#include "usermanager.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QRegularExpression>

UserManager::UserManager(QObject *parent) : QObject(parent) {}

void UserManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void UserManager::refresh() {
    // Read autologin from SDDM config
    run("awk -F= '/^AutoLogin[[:space:]]*=/{print $2}' /etc/sddm.conf /etc/sddm.conf.d/*.conf 2>/dev/null | tail -1", [this](QString out) {
        m_autoLoginUser = out.trimmed();
    });

    run("whoami", [this](QString out) {
        m_currentUser = out.trimmed();

        // Avatar: check KDE face path
        QString facePath = QDir::homePath() + "/.face.icon";
        m_avatarPath = QFile::exists(facePath) ? facePath : QString();

        run("getent passwd | awk -F: '$3 >= 1000 && $3 < 65534 {print $1\":\"$5\":\"$6}'", [this](QString pwOut) {
            m_users.clear();
            for (const QString &line : pwOut.split('\n')) {
                if (line.trimmed().isEmpty()) continue;
                QStringList parts = line.split(':');
                QVariantMap u;
                u["username"] = parts.value(0);
                u["fullName"] = parts.value(1).split(',').value(0);
                u["home"]     = parts.value(2);
                u["isCurrent"] = parts.value(0) == m_currentUser;
                QString av = parts.value(2) + "/.face.icon";
                u["avatar"] = QFile::exists(av) ? av : QString();
                m_users.append(u);
            }
            emit usersChanged();
        });
    });
}

void UserManager::setFullName(const QString &username, const QString &fullName) {
    auto *proc = new QProcess(this);
    proc->start("pkexec", {"chfn", "-f", fullName, username});
    connect(proc, &QProcess::finished, this, [this, proc](int code, QProcess::ExitStatus) {
        emit operationResult(code == 0, code == 0 ? QString() : "Failed to change name");
        if (code == 0) refresh();
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
}

void UserManager::setPassword(const QString &username, const QString &password) {
    auto *proc = new QProcess(this);
    proc->start("pkexec", {"chpasswd"});
    connect(proc, &QProcess::started, this, [proc, username, password]() {
        proc->write((username + ":" + password + "\n").toUtf8());
        proc->closeWriteChannel();
    });
    connect(proc, &QProcess::finished, this, [this, proc](int code, QProcess::ExitStatus) {
        emit operationResult(code == 0, code == 0 ? QString() : "Failed to change password");
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
}

void UserManager::setAvatar(const QString &username, const QString &imagePath) {
    QString home;
    for (const QVariant &v : m_users) {
        if (v.toMap()["username"] == username) { home = v.toMap()["home"].toString(); break; }
    }
    if (home.isEmpty()) { emit operationResult(false, "User not found"); return; }
    QString dest = home + "/.face.icon";
    auto *proc = new QProcess(this);
    proc->start("pkexec", {"cp", imagePath, dest});
    connect(proc, &QProcess::finished, this, [this, proc](int code, QProcess::ExitStatus) {
        emit operationResult(code == 0, code == 0 ? QString() : "Failed to set avatar");
        if (code == 0) refresh();
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
}

void UserManager::setAutoLogin(const QString &username) {
    m_autoLoginUser = username;
    emit usersChanged();
    if (username.isEmpty()) {
        run("pkexec bash -c \"sed -i '/^AutoLogin/d' /etc/sddm.conf.d/autologin.conf 2>/dev/null; exit 0\"", [](QString) {});
    } else {
        QString cmd = QString("pkexec bash -c \"mkdir -p /etc/sddm.conf.d && printf '[Autologin]\\nUser=%1\\n' > /etc/sddm.conf.d/autologin.conf\"").arg(username);
        run(cmd, [](QString) {});
    }
}

void UserManager::signOut() {
    run("qdbus org.kde.ksmserver /KSMServer logout 0 0 0 2>/dev/null", [](QString) {});
}
