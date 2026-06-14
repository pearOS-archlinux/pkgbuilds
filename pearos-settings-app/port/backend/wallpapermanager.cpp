#include "wallpapermanager.h"
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QColor>

WallpaperManager::WallpaperManager(QObject *parent) : QObject(parent) {}

void WallpaperManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void WallpaperManager::refreshWallpapers() {
    QStringList dirs = {
        "/usr/share/extras/wallpapers",
        QDir::homePath() + "/.local/share/wallpapers",
        "/usr/share/wallpapers",
    };
    m_wallpapers.clear();

    // Build categories map: category name → list of {path, name}
    QMap<QString, QVariantList> byCategory;

    for (const QString &dir : dirs) {
        QDir d(dir);
        // Flat files at top level → "General" category
        for (const QFileInfo &fi : d.entryInfoList({"*.jpg","*.jpeg","*.png","*.webp"}, QDir::Files)) {
            QVariantMap entry;
            entry["path"] = fi.absoluteFilePath();
            entry["name"] = fi.baseName();
            m_wallpapers.append(entry);
            byCategory["General"].append(entry);
        }
        // Subdirectory → category name from dir name
        for (const QFileInfo &sub : d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QString catName = sub.baseName();
            QDir subDir(sub.absoluteFilePath());
            // Direct images in subdir
            for (const QFileInfo &fi : subDir.entryInfoList({"*.jpg","*.jpeg","*.png","*.webp"}, QDir::Files)) {
                QVariantMap entry;
                entry["path"] = fi.absoluteFilePath();
                entry["name"] = fi.baseName();
                m_wallpapers.append(entry);
                byCategory[catName].append(entry);
            }
            // KDE style: subdir/contents/images/
            QDir imgDir(sub.absoluteFilePath() + "/contents/images");
            for (const QFileInfo &fi : imgDir.entryInfoList({"*.jpg","*.jpeg","*.png"}, QDir::Files)) {
                QVariantMap entry;
                entry["path"] = fi.absoluteFilePath();
                entry["name"] = sub.baseName();
                m_wallpapers.append(entry);
                byCategory[catName].append(entry);
            }
        }
    }

    // Build ordered categories list
    m_categories.clear();
    for (const QString &catName : byCategory.keys()) {
        QVariantMap cat;
        cat["name"] = catName;
        cat["wallpapers"] = byCategory[catName];
        m_categories.append(cat);
    }

    emit wallpapersChanged();

    run("plasma-apply-wallpaperimage --list-wallpapers 2>/dev/null | head -1", [this](QString out) {
        if (!out.trimmed().isEmpty()) {
            m_current = out.trimmed();
            emit currentChanged();
            computeTint(m_current);
        }
    });
}

void WallpaperManager::computeTint(const QString &wallpaperPath) {
    // Sample a small portion of the wallpaper to get average color
    QString path = wallpaperPath;
    if (path.startsWith("file://")) path = path.mid(7);
    if (path.isEmpty()) { m_tintColor = "transparent"; emit tintColorChanged(); return; }

    QImage img(path);
    if (img.isNull()) { m_tintColor = "transparent"; emit tintColorChanged(); return; }

    // Scale down to 50x50 for fast average
    QImage small = img.scaled(50, 50, Qt::IgnoreAspectRatio, Qt::FastTransformation)
                      .convertToFormat(QImage::Format_RGB888);
    long r = 0, g = 0, b = 0;
    int n = small.width() * small.height();
    for (int y = 0; y < small.height(); ++y) {
        const uchar *row = small.constScanLine(y);
        for (int x = 0; x < small.width(); ++x) {
            r += row[x * 3 + 0];
            g += row[x * 3 + 1];
            b += row[x * 3 + 2];
        }
    }
    r /= n; g /= n; b /= n;
    // Return as hex; QML will apply alpha via Qt.rgba()
    m_tintColor = QString("#%1%2%3")
        .arg((int)r, 2, 16, QChar('0'))
        .arg((int)g, 2, 16, QChar('0'))
        .arg((int)b, 2, 16, QChar('0'));
    emit tintColorChanged();
}

void WallpaperManager::setWallpaper(const QString &path) {
    auto *proc = new QProcess(this);
    proc->start("plasma-apply-wallpaperimage", {path});
    connect(proc, &QProcess::finished, this, [this, proc, path](int code, QProcess::ExitStatus) {
        proc->deleteLater();
        if (code == 0) { m_current = path; emit currentChanged(); computeTint(path); emit wallpaperSet(true, {}); }
        else emit wallpaperSet(false, QString::fromUtf8(proc->readAllStandardError()));
    });
    connect(proc, &QProcess::errorOccurred, this, [this, proc](QProcess::ProcessError) {
        emit wallpaperSet(false, "Failed to launch plasma-apply-wallpaperimage");
        proc->deleteLater();
    });
}
