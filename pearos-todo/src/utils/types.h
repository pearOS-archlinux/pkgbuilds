#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDateTime>
#include <memory>

struct PackageInfo {
    QString name;
    QString version;
    QString description;
    QString repository;
    QString maintainer;
    QString upstreamUrl;
    QStringList dependList;
    QDateTime lastUpdated;
    
    PackageInfo() = default;
    
    PackageInfo(const QString& name, const QString& version, 
                const QString& description, const QString& repository)
        : name(name), version(version), description(description), 
          repository(repository) {}
};

struct UpdateInfo {
    QString name;
    QString oldVersion;
    QString newVersion;
    QString repository;
    qint64 downloadSize;
    
    UpdateInfo() : downloadSize(0) {}
    
    UpdateInfo(const QString& name, const QString& oldVersion,
               const QString& newVersion, const QString& repository,
               qint64 downloadSize = 0)
        : name(name)
        , oldVersion(oldVersion)
        , newVersion(newVersion)
        , repository(repository)
        , downloadSize(downloadSize) {}
};

struct TodoItem {
    QString text;
    bool completed = false;
};

struct TodoList {
    QString id;
    QString name;
    QVector<TodoItem> items;
};

#endif // TYPES_H
