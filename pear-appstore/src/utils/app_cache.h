#ifndef APP_CACHE_H
#define APP_CACHE_H

#include "types.h"
#include <QVector>

/**
 * Cache pe disc pentru liste de pachete (installed, updates, featured).
 * Folosește CacheHelper pentru paths; icoanele rămân în iconsDir().
 */
namespace AppCache {
    /** Încarcă lista de pachete instalate din cache; goală dacă lipsește sau e expirată (max 24h). */
    QVector<PackageInfo> loadInstalledCache();
    /** Salvează lista de pachete instalate în cache. */
    bool saveInstalledCache(const QVector<PackageInfo>& packages);

    /** Încarcă lista de updates din cache; goală dacă lipsește sau e expirată (max 2h). */
    QVector<UpdateInfo> loadUpdatesCache();
    /** Salvează lista de updates în cache. */
    bool saveUpdatesCache(const QVector<UpdateInfo>& updates);

    /** Încarcă lista featured (Discover) din cache; goală dacă lipsește sau e expirată (max 24h). */
    QVector<PackageInfo> loadFeaturedCache();
    /** Salvează lista featured în cache. */
    bool saveFeaturedCache(const QVector<PackageInfo>& packages);
}

#endif // APP_CACHE_H
