#include "aur_helper.h"
#include "../utils/logger.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>
#include <QProcess>
#include <QDateTime>
#include <QEventLoop>

AurHelper::AurHelper(QObject* parent)
    : QObject(parent)
    , m_networkManager(std::make_unique<QNetworkAccessManager>(this)) {
}

AurHelper::~AurHelper() = default;

void AurHelper::searchPackages(const QString& query) {
    Logger::debug(QString("Searching AUR for: %1").arg(query));
    
    QUrl url("https://aur.archlinux.org/rpc/");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("v", "5");
    urlQuery.addQueryItem("type", "search");
    urlQuery.addQueryItem("arg", query);
    url.setQuery(urlQuery);
    
    QNetworkRequest request(url);
    auto* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, &AurHelper::onSearchFinished);
}

void AurHelper::onSearchFinished() {
    auto* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        Logger::error(QString("AUR search error: %1").arg(reply->errorString()));
        emit error(reply->errorString());
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        Logger::error("Invalid AUR response format");
        emit error("Invalid response from AUR");
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray results = root["results"].toArray();
    
    QVector<PackageInfo> packages;
    for (const auto& result : results) {
        packages.push_back(parseAurPackage(result.toObject()));
    }
    
    Logger::info(QString("Found %1 AUR packages").arg(packages.size()));
    emit searchCompleted(packages);
}

void AurHelper::getPackageInfo(const QString& packageName) {
    Logger::debug(QString("Getting AUR package info for: %1").arg(packageName));
    
    QUrl url("https://aur.archlinux.org/rpc/");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("v", "5");
    urlQuery.addQueryItem("type", "info");
    urlQuery.addQueryItem("arg", packageName);
    url.setQuery(urlQuery);
    
    QNetworkRequest request(url);
    auto* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, &AurHelper::onPackageInfoFinished);
}

void AurHelper::onPackageInfoFinished() {
    auto* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        Logger::error(QString("AUR package info error: %1").arg(reply->errorString()));
        emit error(reply->errorString());
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        emit error("Invalid response from AUR");
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray results = root["results"].toArray();
    
    if (results.isEmpty()) {
        emit error("Package not found in AUR");
        return;
    }
    
    PackageInfo info = parseAurPackage(results[0].toObject());
    emit packageInfoReceived(info);
}

PackageInfo AurHelper::parseAurPackage(const QJsonObject& obj) {
    PackageInfo info;
    info.name = obj["Name"].toString();
    info.version = obj["Version"].toString();
    info.description = obj["Description"].toString();
    info.repository = "AUR";
    info.maintainer = obj["Maintainer"].toString();
    info.upstreamUrl = obj["URL"].toString();
    
    qint64 lastModified = obj["LastModified"].toInteger();
    info.lastUpdated = QDateTime::fromSecsSinceEpoch(lastModified);
    
    // Parse dependencies
    QJsonArray depends = obj["Depends"].toArray();
    for (const auto& dep : depends) {
        info.dependList.append(dep.toString());
    }
    
    // Also add make dependencies if available
    QJsonArray makeDepends = obj["MakeDepends"].toArray();
    for (const auto& dep : makeDepends) {
        QString depStr = dep.toString() + " (make)";
        info.dependList.append(depStr);
    }
    
    return info;
}

QVector<UpdateInfo> AurHelper::checkAurUpdates() {
    QVector<UpdateInfo> updates;
    
    // Get list of foreign (AUR) packages
    QProcess process;
    process.start("pacman", QStringList() << "-Qm");
    process.waitForFinished();
    
    if (process.exitCode() != 0) {
        Logger::warning("Failed to get list of foreign packages");
        return updates;
    }
    
    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const auto& line : lines) {
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 2) continue;
        
        QString name = parts[0];
        QString version = parts[1];
        
        // Query AUR for latest version
        QUrl url("https://aur.archlinux.org/rpc/");
        QUrlQuery urlQuery;
        urlQuery.addQueryItem("v", "5");
        urlQuery.addQueryItem("type", "info");
        urlQuery.addQueryItem("arg", name);
        url.setQuery(urlQuery);
        
        QNetworkRequest request(url);
        auto reply = m_networkManager->get(request);
        
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject root = doc.object();
            QJsonArray results = root["results"].toArray();
            
            if (!results.isEmpty()) {
                QString newVersion = results[0].toObject()["Version"].toString();
                if (newVersion != version) {
                    UpdateInfo update;
                    update.name = name;
                    update.oldVersion = version;
                    update.newVersion = newVersion;
                    update.repository = "AUR";
                    update.downloadSize = 0;
                    updates.push_back(std::move(update));
                }
            }
        }
        
        reply->deleteLater();
    }
    
    return updates;
}
