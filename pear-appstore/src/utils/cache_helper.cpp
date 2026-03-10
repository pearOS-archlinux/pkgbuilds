#include "cache_helper.h"
#include "logger.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCryptographicHash>

static const char* s_cacheSubdir = "pearos-appstore";
static bool s_cacheRootLogged = false;

/** Verifică dacă putem scrie efectiv în director (evită probleme cu isWritable() pe unele fs/SELinux). */
static bool canWriteInDir(const QString& dirPath) {
    QDir d(dirPath);
    if (!d.exists()) return false;
    QString testFile = dirPath + QLatin1String("/.write_test_alg");
    QFile f(testFile);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.close();
    if (!f.remove()) return false;
    return true;
}

QString CacheHelper::cacheRoot() {
    QString systemCache = QStringLiteral("/var/cache/") + QLatin1String(s_cacheSubdir);
    if (canWriteInDir(systemCache)) {
        if (!s_cacheRootLogged) {
            s_cacheRootLogged = true;
            Logger::info(QString("Cache root: %1").arg(systemCache));
        }
        return systemCache;
    }
    QString home = QDir::homePath();
    QString homeCache = home + QLatin1String("/.cache/") + QLatin1String(s_cacheSubdir);
    if (!s_cacheRootLogged) {
        s_cacheRootLogged = true;
        Logger::info(QString("Cache root: %1 (fallback; /var/cache/... inexistent sau read-only)").arg(homeCache));
    }
    return homeCache;
}

bool CacheHelper::ensureCacheRoot() {
    QString root = cacheRoot();
    if (QDir().mkpath(root)) {
        QDir().mkpath(iconsDir());
        QDir().mkpath(screenshotsDir());
        return true;
    }
    return false;
}

QString CacheHelper::iconsDir() {
    return cacheRoot() + QLatin1String("/icons");
}

QString CacheHelper::screenshotsDir() {
    return cacheRoot() + QLatin1String("/screenshots");
}

QString CacheHelper::pathForUrl(const QString& url) {
    QByteArray hash = QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Sha1);
    return screenshotsDir() + QLatin1Char('/') + QString::fromUtf8(hash.toHex()) + QLatin1String(".png");
}

bool CacheHelper::ensureScreenshotsDir() {
    return QDir().mkpath(screenshotsDir());
}

QString CacheHelper::updatesCacheFile() {
    return cacheRoot() + QLatin1String("/updates_cache.json");
}

QString CacheHelper::installedCacheFile() {
    return cacheRoot() + QLatin1String("/installed_cache.json");
}

QString CacheHelper::featuredCacheFile() {
    return cacheRoot() + QLatin1String("/featured_cache.json");
}
