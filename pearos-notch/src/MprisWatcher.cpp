#include "MprisWatcher.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QNetworkRequest>
#include <QUrl>
#include <QRegularExpression>
#include <QDebug>
#include <unistd.h>

// ── D-Bus helper ──────────────────────────────────────────────────────────────

static QDBusConnection sessionBus() {
    if (QDBusConnection::sessionBus().isConnected())
        return QDBusConnection::sessionBus();

    QString addr = qEnvironmentVariable("DBUS_SESSION_BUS_ADDRESS");
    if (addr.isEmpty()) {
        addr = "unix:path=/run/user/" + QString::number(getuid()) + "/bus";
        qputenv("DBUS_SESSION_BUS_ADDRESS", addr.toUtf8());
    }
    return QDBusConnection::connectToBus(addr, "pearos-notch-session");
}

// ── Constructor ───────────────────────────────────────────────────────────────

MprisWatcher::MprisWatcher(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    m_metaTimer = new QTimer(this);
    m_metaTimer->setInterval(1500);
    connect(m_metaTimer, &QTimer::timeout, this, &MprisWatcher::refresh);
    m_metaTimer->start();

    m_posTimer = new QTimer(this);
    m_posTimer->setInterval(1000);
    connect(m_posTimer, &QTimer::timeout, this, &MprisWatcher::refreshPosition);
    m_posTimer->start();

    connect(m_nam, &QNetworkAccessManager::finished,
            this, &MprisWatcher::onArtDownloaded);

    // Defer first refresh so NotchWindow signals are connected first
    QTimer::singleShot(300, this, &MprisWatcher::refresh);
}

// ── D-Bus helpers ─────────────────────────────────────────────────────────────

QString MprisWatcher::activePlayer() const {
    QDBusInterface dbus("org.freedesktop.DBus", "/",
                        "org.freedesktop.DBus", sessionBus());
    QDBusReply<QStringList> reply = dbus.call("ListNames");
    if (!reply.isValid()) return {};

    for (const QString& name : reply.value())
        if (name.startsWith("org.mpris.MediaPlayer2."))
            return name;
    return {};
}

QVariant MprisWatcher::playerProp(const QString& player, const QString& prop) const {
    QDBusInterface iface(player, "/org/mpris/MediaPlayer2",
                         "org.freedesktop.DBus.Properties", sessionBus());
    QDBusReply<QVariant> r = iface.call("Get",
        "org.mpris.MediaPlayer2.Player", prop);
    return r.isValid() ? r.value() : QVariant{};
}

QString MprisWatcher::variantToString(const QVariant& v) const {
    if (v.userType() == qMetaTypeId<QDBusObjectPath>())
        return v.value<QDBusObjectPath>().path();
    if (v.canConvert<QStringList>()) {
        QStringList list = v.value<QStringList>();
        return list.isEmpty() ? QString() : list.first();
    }
    return v.toString();
}

// ── Art loading ───────────────────────────────────────────────────────────────

void MprisWatcher::loadArt(const QString& url, const QString& cacheKey) {
    if (url.isEmpty() || cacheKey == m_lastArtKey) return;
    m_pendingArtUrl = url;

    // YouTube thumbnail extraction
    QString fetchUrl = url;
    if (url.contains("youtube.com") || url.contains("youtu.be") || url.contains("music.youtube")) {
        static QRegularExpression re("[?&]v=([A-Za-z0-9_\\-]{11})");
        QRegularExpressionMatch m = re.match(url);
        if (m.hasMatch()) {
            fetchUrl = "https://img.youtube.com/vi/" + m.captured(1) + "/hqdefault.jpg";
            qDebug() << "[MPRIS] YouTube thumb:" << fetchUrl;
        }
    }

    QNetworkRequest req;
    req.setUrl(QUrl{fetchUrl});
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    m_nam->get(req);
    m_lastArtKey = cacheKey;
}

void MprisWatcher::onArtDownloaded(QNetworkReply* reply) {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[MPRIS] Art download failed:" << reply->errorString();
        return;
    }
    QPixmap pm;
    if (pm.loadFromData(reply->readAll())) {
        m_info.art = pm;
        emit infoChanged();
    }
}

// ── Refresh ───────────────────────────────────────────────────────────────────

void MprisWatcher::refresh() {
    QString player = activePlayer();
    if (player.isEmpty()) {
        if (!m_info.title.isEmpty()) {
            m_info = {};
            emit infoChanged();
        }
        return;
    }
    qDebug() << "[MPRIS] Active player:" << player;

    // Subscribe to PropertiesChanged if we switched players
    if (player != m_subscribedPlayer) {
        if (!m_subscribedPlayer.isEmpty()) {
            sessionBus().disconnect(m_subscribedPlayer,
                "/org/mpris/MediaPlayer2",
                "org.freedesktop.DBus.Properties",
                "PropertiesChanged",
                this, SLOT(onPropertiesChanged(QString,QVariantMap,QStringList)));
        }
        sessionBus().connect(player,
            "/org/mpris/MediaPlayer2",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            this, SLOT(onPropertiesChanged(QString,QVariantMap,QStringList)));
        m_subscribedPlayer = player;
    }

    QVariant metaVar  = playerProp(player, "Metadata");
    QVariant statusVar = playerProp(player, "PlaybackStatus");

    QVariantMap meta;
    if (metaVar.canConvert<QDBusArgument>()) {
        metaVar.value<QDBusArgument>() >> meta;
    } else {
        meta = metaVar.toMap();
    }

    MediaInfo info;
    info.title   = variantToString(meta.value("xesam:title"));
    info.artist  = variantToString(meta.value("xesam:artist"));
    info.album   = variantToString(meta.value("xesam:album"));
    info.lengthUs = meta.value("mpris:length").toLongLong();
    info.playing  = (statusVar.toString() == "Playing");
    info.art      = m_info.art;  // preserve existing art

    qDebug() << "[MPRIS] title=" << info.title << "playing=" << info.playing;

    QString artUrl   = variantToString(meta.value("mpris:artUrl"));
    QString trackKey = info.title + "|" + info.artist;

    bool infoChanged = (info.title   != m_info.title   ||
                        info.artist  != m_info.artist  ||
                        info.playing != m_info.playing ||
                        info.lengthUs != m_info.lengthUs);

    m_info = info;

    if (infoChanged)
        emit this->infoChanged();

    loadArt(artUrl, trackKey);
    refreshPosition();
}

void MprisWatcher::refreshPosition() {
    QString player = activePlayer();
    if (player.isEmpty()) return;

    QVariant posVar = playerProp(player, "Position");
    qint64 pos = posVar.toLongLong();
    if (pos != m_info.posUs) {
        m_info.posUs = pos;
        emit positionChanged(pos);
    }
}

void MprisWatcher::onPropertiesChanged(const QString& /*iface*/,
                                        const QVariantMap& /*changed*/,
                                        const QStringList& /*invalidated*/) {
    refresh();
}

// ── Controls ──────────────────────────────────────────────────────────────────

void MprisWatcher::playerCall(const QString& method) {
    QString player = activePlayer();
    if (player.isEmpty()) return;
    QDBusInterface iface(player, "/org/mpris/MediaPlayer2",
                         "org.mpris.MediaPlayer2.Player", sessionBus());
    iface.asyncCall(method);
}

void MprisWatcher::previous()  { playerCall("Previous"); }
void MprisWatcher::playPause() { playerCall("PlayPause"); }
void MprisWatcher::next()      { playerCall("Next"); }
