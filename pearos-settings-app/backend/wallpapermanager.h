#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <functional>

class WallpaperManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList wallpapers           READ wallpapers           NOTIFY wallpapersChanged)
    Q_PROPERTY(QVariantList wallpaperCategories  READ wallpaperCategories  NOTIFY wallpapersChanged)
    Q_PROPERTY(QString currentWallpaper READ currentWallpaper NOTIFY currentChanged)
    Q_PROPERTY(QString tintColor READ tintColor NOTIFY tintColorChanged)

public:
    explicit WallpaperManager(QObject *parent = nullptr);
    QVariantList wallpapers()           const { return m_wallpapers; }
    QVariantList wallpaperCategories()  const { return m_categories; }
    QString currentWallpaper()    const { return m_current; }
    QString tintColor()           const { return m_tintColor; }

    Q_INVOKABLE void refreshWallpapers();
    Q_INVOKABLE void refreshTint();
    Q_INVOKABLE void setWallpaper(const QString &path);

signals:
    void wallpapersChanged();
    void currentChanged();
    void tintColorChanged();
    void wallpaperSet(bool success, const QString &error);

private:
    QVariantList m_wallpapers;
    QVariantList m_categories;
    QString m_current;
    QString m_tintColor = "transparent";
    void run(const QString &cmd, std::function<void(QString)> cb);
    void computeTint(const QString &wallpaperPath);
};
