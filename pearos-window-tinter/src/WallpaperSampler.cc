/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "WallpaperSampler.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

namespace
{
constexpr auto kConfigName = "plasma-org.kde.plasma.desktop-appletsrc";
constexpr auto kImagePlugin = "org.kde.image";

// Plasma's org.kde.image FillMode enum (Image.qml).
enum FillMode {
    Stretch = 0,
    PreserveAspectFit = 1,
    PreserveAspectCrop = 2,
    Tile = 3,
    TileVertically = 4,
    TileHorizontally = 5,
    Pad = 6,
};
}

WallpaperSampler::WallpaperSampler(QObject *parent)
    : QObject(parent)
{
    m_configPath = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QLatin1String(kConfigName));

    // Plasma rewrites the file rather than editing it in place, which drops
    // the inotify watch -- so re-arm the watch on every notification.
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this] {
        QTimer::singleShot(200, this, [this] {
            watchConfigFile();
            reload();
        });
    });

    watchConfigFile();
    reload();
}

void WallpaperSampler::watchConfigFile()
{
    if (m_configPath.isEmpty()) {
        m_configPath = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QLatin1String(kConfigName));
    }
    if (!m_configPath.isEmpty() && !m_watcher.files().contains(m_configPath)) {
        m_watcher.addPath(m_configPath);
    }
}

void WallpaperSampler::setSampleResolution(int longEdge)
{
    if (m_sampleResolution == longEdge) {
        return;
    }
    m_sampleResolution = longEdge;
    m_byScreen.clear();
    reload();
}

QString WallpaperSampler::resolveImagePath(const QString &configured)
{
    if (configured.isEmpty()) {
        return QString();
    }

    QString path = configured.startsWith(QLatin1String("file://")) ? QUrl(configured).toLocalFile() : configured;

    QFileInfo info(path);
    if (info.isFile()) {
        return path;
    }
    if (!info.isDir()) {
        return QString();
    }

    // Wallpaper packages point at a directory; the actual images live under
    // contents/images/ named after the resolution they were made for. Pick
    // the largest one -- it gets downscaled to the sample grid anyway.
    QDir imagesDir(path + QLatin1String("/contents/images"));
    if (!imagesDir.exists()) {
        return QString();
    }
    const QStringList entries = imagesDir.entryList({QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"), QStringLiteral("*.webp")}, QDir::Files);
    QString best;
    qint64 bestSize = -1;
    for (const QString &entry : entries) {
        const QFileInfo entryInfo(imagesDir.filePath(entry));
        if (entryInfo.size() > bestSize) {
            bestSize = entryInfo.size();
            best = entryInfo.absoluteFilePath();
        }
    }
    return best;
}

void WallpaperSampler::reload()
{
    auto config = KSharedConfig::openConfig(QLatin1String(kConfigName), KConfig::NoGlobals);
    config->reparseConfiguration();

    KConfigGroup containments = config->group(QStringLiteral("Containments"));

    QHash<int, Wallpaper> updated;
    bool dirty = false;

    const QStringList ids = containments.groupList();
    for (const QString &id : ids) {
        KConfigGroup containment = containments.group(id);

        // formfactor 0 is a desktop containment; panels are 2/3 and have a
        // wallpaperplugin entry too, which we must not mistake for a desktop.
        if (containment.readEntry("formfactor", -1) != 0) {
            continue;
        }
        if (containment.readEntry("wallpaperplugin", QString()) != QLatin1String(kImagePlugin)) {
            continue; // video/slideshow-of-color/third-party plugins: no image to sample
        }

        const int screen = containment.readEntry("lastScreen", -1);
        if (screen < 0) {
            continue;
        }

        KConfigGroup wallpaperGroup = containment.group(QStringLiteral("Wallpaper")).group(QLatin1String(kImagePlugin)).group(QStringLiteral("General"));
        // A slideshow writes the current image back into the same Image key,
        // so this covers both the static and the slideshow case.
        const QString path = resolveImagePath(wallpaperGroup.readEntry("Image", QString()));
        if (path.isEmpty()) {
            continue;
        }

        Wallpaper wallpaper;
        wallpaper.path = path;
        wallpaper.fillMode = wallpaperGroup.readEntry("FillMode", int(PreserveAspectCrop));

        const auto existing = m_byScreen.constFind(screen);
        if (existing != m_byScreen.constEnd() && existing->path == path && !existing->image.isNull()) {
            wallpaper.image = existing->image;
            if (existing->fillMode != wallpaper.fillMode) {
                dirty = true;
            }
        } else {
            QImage image;
            if (!image.load(path)) {
                continue;
            }
            // One cheap downscale up front; every later sample is a crop of
            // this, so a wallpaper change costs one decode, not one per frame.
            wallpaper.image = image.scaled(m_sampleResolution, m_sampleResolution,
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation)
                                  .convertToFormat(QImage::Format_RGB32);
            dirty = true;
        }

        updated.insert(screen, wallpaper);
    }

    if (updated.size() != m_byScreen.size()) {
        dirty = true;
    }

    m_byScreen = updated;

    if (dirty) {
        Q_EMIT changed();
    }
}

QRect WallpaperSampler::mapToImage(const Wallpaper &wallpaper, const QRect &localRect, const QSize &screenSize)
{
    const QSize imageSize = wallpaper.image.size();
    if (imageSize.isEmpty() || screenSize.isEmpty()) {
        return QRect();
    }

    // Source rectangle of the image that ends up covering the whole screen.
    QRectF source(QPointF(0, 0), QSizeF(imageSize));

    switch (wallpaper.fillMode) {
    case PreserveAspectCrop: {
        const qreal scale = qMax(qreal(screenSize.width()) / imageSize.width(),
                                 qreal(screenSize.height()) / imageSize.height());
        const QSizeF visible(screenSize.width() / scale, screenSize.height() / scale);
        source = QRectF(QPointF((imageSize.width() - visible.width()) / 2.0,
                                (imageSize.height() - visible.height()) / 2.0),
                        visible);
        break;
    }
    case Stretch:
    case Tile:
    case TileVertically:
    case TileHorizontally:
    case PreserveAspectFit:
    case Pad:
    default:
        // Stretch maps exactly. For fit/pad/tile the mapping is only an
        // approximation -- the letterbox bars and tile seams are ignored and
        // the whole image is treated as covering the screen. Good enough for
        // an average color, and it degrades gracefully.
        break;
    }

    const qreal sx = source.width() / screenSize.width();
    const qreal sy = source.height() / screenSize.height();

    const QRectF mapped(source.x() + localRect.x() * sx,
                        source.y() + localRect.y() * sy,
                        localRect.width() * sx,
                        localRect.height() * sy);

    return mapped.toAlignedRect().intersected(QRect(QPoint(0, 0), imageSize));
}

QColor WallpaperSampler::averageColor(int screenIndex, const QRect &localRect, const QSize &screenSize) const
{
    auto it = m_byScreen.constFind(screenIndex);
    if (it == m_byScreen.constEnd()) {
        // Single-wallpaper setups are the common case; if the screen index
        // doesn't line up with Plasma's, fall back to whatever we have.
        if (m_byScreen.isEmpty()) {
            return QColor();
        }
        it = m_byScreen.constBegin();
    }

    const QRect sampleRect = mapToImage(*it, localRect, screenSize);
    if (sampleRect.isEmpty()) {
        return QColor();
    }

    // Downsample the region to a single pixel; the smooth transform is the
    // averaging (same trick as Filer's averageColorForRect()).
    const QImage onePixel = it->image.copy(sampleRect).scaled(1, 1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (onePixel.isNull()) {
        return QColor();
    }
    return onePixel.pixelColor(0, 0);
}
