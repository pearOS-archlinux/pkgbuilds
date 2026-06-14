#include "touchidmanager.h"
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>

TouchIDManager::TouchIDManager(QObject *parent) : QObject(parent) {
    m_username = qgetenv("USER");
    if (m_username.isEmpty()) m_username = qgetenv("LOGNAME");
}

void TouchIDManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void TouchIDManager::refresh() {
    run("lsusb 2>/dev/null | grep -i -e finger -e fprint -e biom | head -1", [this](QString out) {
        m_hasDevice = !out.trimmed().isEmpty();
        if (!m_hasDevice) {
            m_fingerprints.clear();
            emit changed();
            return;
        }

        run(QString("fprintd-list %1 2>/dev/null").arg(m_username), [this](QString out) {
            m_fingerprints.clear();
            static QRegularExpression fpRe(R"(finger:\s+(\S+))");
            auto it = fpRe.globalMatch(out);
            while (it.hasNext()) {
                auto m = it.next();
                QString finger = m.captured(1);
                QVariantMap fp;
                fp["finger"] = finger;
                fp["label"] = finger.replace('-', ' ');
                m_fingerprints.append(fp);
            }
            emit changed();
        });
    });
}

void TouchIDManager::enrollFingerprint(const QString &finger) {
    if (m_enrolling) return;
    m_enrolling = true;
    m_enrollStatus = "Place your finger on the sensor...";
    emit changed();

    m_enrollProc = new QProcess(this);
    connect(m_enrollProc, &QProcess::readyReadStandardOutput, this, [this]() {
        QString out = QString::fromUtf8(m_enrollProc->readAllStandardOutput());
        if (out.contains("done") || out.contains("enrolled"))
            m_enrollStatus = "Fingerprint enrolled successfully!";
        else if (out.contains("stage") || out.contains("swipe"))
            m_enrollStatus = "Swipe detected — keep going...";
        else if (!out.trimmed().isEmpty())
            m_enrollStatus = out.trimmed();
        emit changed();
    });
    connect(m_enrollProc, &QProcess::readyReadStandardError, this, [this]() {
        QString err = QString::fromUtf8(m_enrollProc->readAllStandardError()).trimmed();
        if (!err.isEmpty()) { m_enrollStatus = err; emit changed(); }
    });
    connect(m_enrollProc, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) {
        m_enrolling = false;
        if (code == 0)
            m_enrollStatus = "Done!";
        else if (!m_enrollStatus.startsWith("Done"))
            m_enrollStatus = "Enrollment failed or cancelled.";
        m_enrollProc = nullptr;
        emit changed();
        refresh();
    });

    m_enrollProc->start("fprintd-enroll", {m_username, finger});
}

void TouchIDManager::deleteFingerprint(const QString &finger) {
    run(QString("fprintd-delete %1 -f %2 2>/dev/null").arg(m_username, finger), [this](QString) {
        refresh();
    });
}

void TouchIDManager::cancelEnroll() {
    if (m_enrollProc) {
        m_enrollProc->terminate();
        m_enrollProc->waitForFinished(1000);
        m_enrollProc = nullptr;
    }
    m_enrolling = false;
    m_enrollStatus = "";
    emit changed();
}
