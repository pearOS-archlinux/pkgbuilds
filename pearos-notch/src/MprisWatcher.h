#pragma once
#include <QObject>
#include <QPixmap>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QVariantMap>
#include <QStringList>

struct MediaInfo {
    QString title;
    QString artist;
    QString album;
    QPixmap art;
    qint64  lengthUs = 0;
    qint64  posUs    = 0;
    bool    playing  = false;
};

class MprisWatcher : public QObject {
    Q_OBJECT
public:
    explicit MprisWatcher(QObject* parent = nullptr);

    const MediaInfo& info() const { return m_info; }

    void previous();
    void playPause();
    void next();

signals:
    void infoChanged();
    void positionChanged(qint64 posUs);

private slots:
    void refresh();
    void refreshPosition();
    void onPropertiesChanged(const QString& iface,
                             const QVariantMap& changed,
                             const QStringList& invalidated);
    void onArtDownloaded(QNetworkReply* reply);

private:
    QString     activePlayer() const;
    QVariant    playerProp(const QString& player, const QString& prop) const;
    QString     variantToString(const QVariant& v) const;
    void        loadArt(const QString& url, const QString& cacheKey);
    void        playerCall(const QString& method);

    MediaInfo   m_info;
    QString     m_lastArtKey;
    QString     m_pendingArtUrl;
    QString     m_subscribedPlayer;

    QTimer*               m_metaTimer = nullptr;
    QTimer*               m_posTimer  = nullptr;
    QNetworkAccessManager* m_nam      = nullptr;
};
