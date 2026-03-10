#include "app_cache.h"
#include "cache_helper.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

namespace {

const int kInstalledMaxAgeHours = 24;
const int kUpdatesMaxAgeHours = 2;
const int kFeaturedMaxAgeHours = 24;

bool isCacheValid(const QString& path, int maxAgeHours) {
    QFileInfo fi(path);
    if (!fi.exists()) return false;
    return fi.lastModified().secsTo(QDateTime::currentDateTime()) <= maxAgeHours * 3600;
}

QJsonObject packageInfoToJson(const PackageInfo& p) {
    QJsonObject o;
    o.insert(QLatin1String("name"), p.name);
    o.insert(QLatin1String("version"), p.version);
    o.insert(QLatin1String("description"), p.description);
    o.insert(QLatin1String("repository"), p.repository);
    o.insert(QLatin1String("maintainer"), p.maintainer);
    o.insert(QLatin1String("upstreamUrl"), p.upstreamUrl);
    QJsonArray depArr;
    for (const QString& s : p.dependList) depArr.append(s);
    o.insert(QLatin1String("dependList"), depArr);
    o.insert(QLatin1String("lastUpdated"), p.lastUpdated.toString(Qt::ISODate));
    return o;
}

PackageInfo packageInfoFromJson(const QJsonObject& o) {
    PackageInfo p;
    p.name = o.value(QLatin1String("name")).toString();
    p.version = o.value(QLatin1String("version")).toString();
    p.description = o.value(QLatin1String("description")).toString();
    p.repository = o.value(QLatin1String("repository")).toString();
    p.maintainer = o.value(QLatin1String("maintainer")).toString();
    p.upstreamUrl = o.value(QLatin1String("upstreamUrl")).toString();
    QJsonArray arr = o.value(QLatin1String("dependList")).toArray();
    for (const QJsonValue& v : arr) p.dependList.append(v.toString());
    p.lastUpdated = QDateTime::fromString(o.value(QLatin1String("lastUpdated")).toString(), Qt::ISODate);
    return p;
}

QJsonObject updateInfoToJson(const UpdateInfo& u) {
    QJsonObject o;
    o.insert(QLatin1String("name"), u.name);
    o.insert(QLatin1String("oldVersion"), u.oldVersion);
    o.insert(QLatin1String("newVersion"), u.newVersion);
    o.insert(QLatin1String("repository"), u.repository);
    o.insert(QLatin1String("downloadSize"), u.downloadSize);
    return o;
}

UpdateInfo updateInfoFromJson(const QJsonObject& o) {
    UpdateInfo u;
    u.name = o.value(QLatin1String("name")).toString();
    u.oldVersion = o.value(QLatin1String("oldVersion")).toString();
    u.newVersion = o.value(QLatin1String("newVersion")).toString();
    u.repository = o.value(QLatin1String("repository")).toString();
    u.downloadSize = o.value(QLatin1String("downloadSize")).toInteger(0);
    return u;
}

} // namespace

namespace AppCache {

QVector<PackageInfo> loadInstalledCache() {
    QString path = CacheHelper::installedCacheFile();
    if (!isCacheValid(path, kInstalledMaxAgeHours)) return {};
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) return {};
    QJsonObject root = doc.object();
    QJsonArray arr = root.value(QLatin1String("packages")).toArray();
    QVector<PackageInfo> list;
    list.reserve(arr.size());
    for (const QJsonValue& v : arr) {
        list.append(packageInfoFromJson(v.toObject()));
    }
    return list;
}

bool saveInstalledCache(const QVector<PackageInfo>& packages) {
    if (!CacheHelper::ensureCacheRoot()) return false;
    QJsonObject root;
    root.insert(QLatin1String("updated"), QDateTime::currentDateTime().toString(Qt::ISODate));
    QJsonArray arr;
    for (const PackageInfo& p : packages) arr.append(packageInfoToJson(p));
    root.insert(QLatin1String("packages"), arr);
    QFile f(CacheHelper::installedCacheFile());
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

QVector<UpdateInfo> loadUpdatesCache() {
    QString path = CacheHelper::updatesCacheFile();
    if (!isCacheValid(path, kUpdatesMaxAgeHours)) return {};
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) return {};
    QJsonObject root = doc.object();
    QJsonArray arr = root.value(QLatin1String("updates")).toArray();
    QVector<UpdateInfo> list;
    list.reserve(arr.size());
    for (const QJsonValue& v : arr) {
        list.append(updateInfoFromJson(v.toObject()));
    }
    return list;
}

bool saveUpdatesCache(const QVector<UpdateInfo>& updates) {
    if (!CacheHelper::ensureCacheRoot()) return false;
    QJsonObject root;
    root.insert(QLatin1String("updated"), QDateTime::currentDateTime().toString(Qt::ISODate));
    QJsonArray arr;
    for (const UpdateInfo& u : updates) arr.append(updateInfoToJson(u));
    root.insert(QLatin1String("updates"), arr);
    QFile f(CacheHelper::updatesCacheFile());
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

QVector<PackageInfo> loadFeaturedCache() {
    QString path = CacheHelper::featuredCacheFile();
    if (!isCacheValid(path, kFeaturedMaxAgeHours)) return {};
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) return {};
    QJsonObject root = doc.object();
    QJsonArray arr = root.value(QLatin1String("packages")).toArray();
    QVector<PackageInfo> list;
    list.reserve(arr.size());
    for (const QJsonValue& v : arr) {
        list.append(packageInfoFromJson(v.toObject()));
    }
    return list;
}

bool saveFeaturedCache(const QVector<PackageInfo>& packages) {
    if (!CacheHelper::ensureCacheRoot()) return false;
    QJsonObject root;
    root.insert(QLatin1String("updated"), QDateTime::currentDateTime().toString(Qt::ISODate));
    QJsonArray arr;
    for (const PackageInfo& p : packages) arr.append(packageInfoToJson(p));
    root.insert(QLatin1String("packages"), arr);
    QFile f(CacheHelper::featuredCacheFile());
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

} // namespace AppCache
