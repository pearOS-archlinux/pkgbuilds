/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QColor>
#include <QFileSystemWatcher>
#include <QHash>
#include <QImage>
#include <QObject>
#include <QRect>

/**
 * Reads Plasma's wallpaper configuration and answers "what color is the
 * wallpaper under this rectangle".
 *
 * Same idea as Filer's DesktopWindow::averageColorForRect() (see
 * ~/Desktop/Filer/src/desktopwindow.cpp:595): downsample the requested
 * region to a single pixel and let bilinear filtering do the averaging.
 * The difference is that Filer owns the desktop window and therefore owns
 * the wallpaper pixmap; a KWin effect does not, so the image is loaded
 * from plasma-org.kde.plasma.desktop-appletsrc instead.
 */
class WallpaperSampler : public QObject
{
    Q_OBJECT

public:
    explicit WallpaperSampler(QObject *parent = nullptr);

    /**
     * Re-reads the Plasma config and reloads any wallpaper whose path
     * changed. Emits changed() if anything actually moved.
     */
    void reload();

    void setSampleResolution(int longEdge);

    /**
     * Average wallpaper color under @p localRect, which is in logical
     * coordinates relative to the top left corner of screen @p screenIndex.
     * Returns an invalid color when that screen has no usable wallpaper
     * (video wallpapers, plugins we don't parse, missing files).
     */
    QColor averageColor(int screenIndex, const QRect &localRect, const QSize &screenSize) const;

Q_SIGNALS:
    void changed();

private:
    struct Wallpaper
    {
        QString path;
        QImage image;
        int fillMode = 2; // Plasma default: PreserveAspectCrop
    };

    void watchConfigFile();
    static QString resolveImagePath(const QString &configured);

    /**
     * Maps @p localRect (logical screen coordinates) into image coordinates
     * for the wallpaper's fill mode.
     */
    static QRect mapToImage(const Wallpaper &wallpaper, const QRect &localRect, const QSize &screenSize);

    QHash<int, Wallpaper> m_byScreen; // keyed by Plasma's lastScreen index
    QFileSystemWatcher m_watcher;
    QString m_configPath;
    int m_sampleResolution = 256;
};
