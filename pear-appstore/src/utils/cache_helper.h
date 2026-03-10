#ifndef CACHE_HELPER_H
#define CACHE_HELPER_H

#include <QString>

/**
 * Cache: încearcă /var/cache/pearos-appstore (dacă există și e writable),
 * altfel ~/.cache/pearos-appstore. Conține icoane, screenshot-uri, liste (updates, installed, featured).
 */
namespace CacheHelper {
    /** Rădăcina cache: /var/cache/pearos-appstore sau ~/.cache/pearos-appstore */
    QString cacheRoot();
    /** Creează rădăcina cache dacă nu există; returnează true dacă e ok. */
    bool ensureCacheRoot();
    /** Director pentru icoane extrase din pachete */
    QString iconsDir();
    /** Director pentru screenshot-uri descărcate (creat la nevoie) */
    QString screenshotsDir();
    /** Cale fișier cache pentru un URL (hash al URL-ului + .png). */
    QString pathForUrl(const QString& url);
    /** Asigură că directorul pentru screenshot-uri există; returnează true dacă e ok. */
    bool ensureScreenshotsDir();
    /** Fișier cache pentru lista de updates (updates_cache.json) */
    QString updatesCacheFile();
    /** Fișier cache pentru lista de pachete instalate (installed_cache.json) */
    QString installedCacheFile();
    /** Fișier cache pentru lista featured Discover (featured_cache.json) */
    QString featuredCacheFile();
}

#endif // CACHE_HELPER_H

