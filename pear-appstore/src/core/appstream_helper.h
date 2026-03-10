#ifndef APPSTREAM_HELPER_H
#define APPSTREAM_HELPER_H

#include <QString>
#include <QStringList>
#include <QByteArray>

/**
 * @brief Metadata from AppStream metainfo (summary, icon, screenshots).
 */
struct AppStreamData {
    QString summary;
    QString description;
    /** e.g. Video, Audio from <categories><category> */
    QStringList categories;
    /** Local path to icon (e.g. /usr/share/pixmaps/... or extracted cache path). */
    QString iconPath;
    /** Stock icon name from theme (e.g. firefox, gimp) when type="stock" in metainfo. */
    QString iconStockName;
    /** URLs or local paths for screenshot images. */
    QStringList screenshotUrls;
    /** When loading from package cache: path inside archive for icon (e.g. usr/share/pixmaps/foo.png). */
    QString iconArchiveMember;
    bool isEmpty() const {
        return summary.isEmpty() && description.isEmpty()
               && iconPath.isEmpty() && iconStockName.isEmpty() && screenshotUrls.isEmpty();
    }
};

/**
 * @brief Loads AppStream metadata from metainfo XML (for packages that ship .metainfo.xml / .appdata.xml).
 */
class AppStreamHelper {
public:
    /** Get metadata for a package: installed (metainfo on disk) or from pacman cache if not installed. */
    static AppStreamData getDataForPackage(const QString& packageName);
    /** Parse a metainfo XML file and return AppStreamData. */
    static AppStreamData parseMetainfoFile(const QString& metainfoPath);
    /** Parse metainfo from XML buffer (e.g. extracted from package); sets iconArchiveMember when icon path is inside archive. */
    static AppStreamData parseMetainfoXml(const QByteArray& xml);

private:
    static QString findMetainfoPath(const QString& packageName);
    static AppStreamData getDataFromCache(const QString& packageName);
    static QString extractIconFromPackage(const QString& pkgPath, const QString& memberPath, const QString& packageName);
};

#endif // APPSTREAM_HELPER_H
