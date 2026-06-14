#pragma once
#include <QObject>
#include <QProcess>
#include <QVariantList>
#include <functional>

class TouchIDManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool hasDevice      READ hasDevice      NOTIFY changed)
    Q_PROPERTY(QVariantList fingerprints READ fingerprints NOTIFY changed)
    Q_PROPERTY(QString enrollStatus READ enrollStatus  NOTIFY changed)
    Q_PROPERTY(bool enrolling      READ enrolling      NOTIFY changed)

public:
    explicit TouchIDManager(QObject *parent = nullptr);

    bool         hasDevice()     const { return m_hasDevice; }
    QVariantList fingerprints()  const { return m_fingerprints; }
    QString      enrollStatus()  const { return m_enrollStatus; }
    bool         enrolling()     const { return m_enrolling; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void enrollFingerprint(const QString &finger);
    Q_INVOKABLE void deleteFingerprint(const QString &finger);
    Q_INVOKABLE void cancelEnroll();

signals:
    void changed();

private:
    bool         m_hasDevice    = false;
    QVariantList m_fingerprints;
    QString      m_enrollStatus;
    bool         m_enrolling    = false;
    QString      m_username;
    QProcess    *m_enrollProc   = nullptr;

    void run(const QString &cmd, std::function<void(QString)> cb);
};
