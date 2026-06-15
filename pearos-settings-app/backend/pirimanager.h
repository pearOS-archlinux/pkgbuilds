#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class PiriManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool    modelExists READ modelExists NOTIFY statusChanged)
    Q_PROPERTY(bool    downloading READ downloading NOTIFY statusChanged)
    Q_PROPERTY(int     progress    READ progress    NOTIFY statusChanged)
    Q_PROPERTY(QString statusText  READ statusText  NOTIFY statusChanged)
    Q_PROPERTY(QString errorText   READ errorText   NOTIFY statusChanged)
    Q_PROPERTY(bool    showIcon    READ showIcon    WRITE setShowIcon NOTIFY showIconChanged)

public:
    explicit PiriManager(QObject *parent = nullptr);

    bool    modelExists() const { return m_modelExists; }
    bool    downloading() const { return m_downloading; }
    int     progress()    const { return m_progress; }
    QString statusText()  const { return m_statusText; }
    QString errorText()   const { return m_errorText; }
    bool    showIcon()    const { return m_showIcon; }
    void    setShowIcon(bool v);

    Q_INVOKABLE void checkModelExists();
    Q_INVOKABLE void downloadModel();
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void removeModel();

signals:
    void statusChanged();
    void showIconChanged();

private:
    static const QString MODEL_URL;
    static const QString MODEL_DIR;
    static const QString ZIP_TMP;
    static const QString EXTRACT_TMP;
    static const QString SHOW_ICON_FILE;

    bool    m_modelExists = false;
    bool    m_downloading = false;
    int     m_progress    = 0;
    QString m_statusText;
    QString m_errorText;
    bool    m_showIcon = false;

    QNetworkAccessManager m_nam;
    QNetworkReply        *m_reply = nullptr;

    void extractAndInstall();
};
