#include "wallpapermanager.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QColor>
#include <QCryptographicHash>

WallpaperManager::WallpaperManager(QObject *parent) : QObject(parent) {}

QString WallpaperManager::thumbCacheDir() {
    QString dir = QDir::homePath() + "/.cache/pearos-settings/wallpaper-thumbs";
    QDir().mkpath(dir);
    return dir;
}

QString WallpaperManager::thumbPathFor(const QString &fullPath) {
    QString hash = QCryptographicHash::hash(fullPath.toUtf8(), QCryptographicHash::Md5).toHex();
    return thumbCacheDir() + "/" + hash + ".jpg";
}

// Wallpaper packages ship multi-K originals (some 6000x6000+); decoding those
// full-size just to show a 100x56 grid thumb is what made the page crawl.
// Generate small cached JPEGs once in the background via ImageMagick, then
// point the grid at those instead of the originals.
void WallpaperManager::generateMissingThumbnails() {
    QStringList missing;
    for (const QVariant &v : m_wallpapers) {
        QVariantMap m = v.toMap();
        QString path = m["path"].toString();
        QString thumb = thumbPathFor(path);
        if (!QFile::exists(thumb)) missing << path;
    }
    if (missing.isEmpty()) return;

    QStringList cmds;
    for (const QString &path : missing) {
        QString thumb = thumbPathFor(path);
        QString escapedPath  = path;  escapedPath.replace("'", "'\\''");
        QString escapedThumb = thumb; escapedThumb.replace("'", "'\\''");
        cmds << QString("convert '%1[0]' -auto-orient -thumbnail '240x135^' "
                         "-gravity center -extent 240x135 -quality 82 '%2' 2>/dev/null")
                    .arg(escapedPath, escapedThumb);
    }
    // Run sequentially in one shell so we don't spawn 70 ImageMagick processes at once
    run(cmds.join(" ; "), [this](QString) { refreshThumbPaths(); });
}

// Re-point already-loaded entries at newly generated thumbnails without
// re-scanning the wallpaper directories.
void WallpaperManager::refreshThumbPaths() {
    auto patch = [](QVariantList &list) {
        for (QVariant &v : list) {
            QVariantMap m = v.toMap();
            QString thumb = thumbPathFor(m["path"].toString());
            if (QFile::exists(thumb)) m["thumb"] = thumb;
            v = m;
        }
    };
    patch(m_wallpapers);
    for (QVariant &c : m_categories) {
        QVariantMap cat = c.toMap();
        QVariantList wps = cat["wallpapers"].toList();
        patch(wps);
        cat["wallpapers"] = wps;
        c = cat;
    }
    emit wallpapersChanged();
}

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
            QString thumb = thumbPathFor(entry["path"].toString());
            entry["thumb"] = QFile::exists(thumb) ? thumb : entry["path"];
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
                QString thumb = thumbPathFor(entry["path"].toString());
                entry["thumb"] = QFile::exists(thumb) ? thumb : entry["path"];
                m_wallpapers.append(entry);
                byCategory[catName].append(entry);
            }
            // KDE style: subdir/contents/images/ — pick the smallest variant as
            // the thumbnail source since these packages ship 4K+ per resolution
            QDir imgDir(sub.absoluteFilePath() + "/contents/images");
            QFileInfoList imgVariants = imgDir.entryInfoList({"*.jpg","*.jpeg","*.png"}, QDir::Files);
            if (!imgVariants.isEmpty()) {
                QFileInfo smallest = imgVariants.first();
                for (const QFileInfo &fi : imgVariants)
                    if (fi.size() < smallest.size()) smallest = fi;

                QVariantMap entry;
                entry["path"] = smallest.absoluteFilePath();
                entry["name"] = sub.baseName();
                QString thumb = thumbPathFor(entry["path"].toString());
                entry["thumb"] = QFile::exists(thumb) ? thumb : entry["path"];
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
    refreshTint();
    generateMissingThumbnails();
}

void WallpaperManager::refreshTint() {
    // Read current wallpaper from plasmarc usersWallpapers (first entry = most recent)
    run("kreadconfig6 --file plasmarc --group Wallpapers --key usersWallpapers 2>/dev/null",
        [this](QString out) {
            QString list = out.trimmed();
            if (list.isEmpty()) return;

            // Try each path in order — skip .heic (not supported by QImage)
            for (const QString &raw : list.split(',')) {
                QString p = raw.trimmed();
                if (p.startsWith("file://")) p = p.mid(7);
                if (p.isEmpty()) continue;
                QString ext = p.section('.', -1).toLower();
                if (ext == "heic" || ext == "avif") continue;
                if (QFileInfo::exists(p)) {
                    if (m_current.isEmpty()) { m_current = p; emit currentChanged(); }
                    computeTint(p);
                    return;
                }
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
