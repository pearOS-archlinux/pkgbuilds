#ifndef ALPM_WRAPPER_H
#define ALPM_WRAPPER_H

#include <alpm.h>
#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>
#include <mutex>
#include "../utils/types.h"

/**
 * @brief Singleton wrapper for libalpm (Arch Linux Package Manager library).
 * 
 * Memory Management:
 * - m_handle: Raw pointer to libalpm handle, manually managed via initialize()/release()
 * - m_syncDbs: Raw pointer to libalpm list, managed by libalpm internally
 * - Thread-safe via m_mutex
 * 
 * Note: libalpm uses C-style memory management, so smart pointers are not
 * directly applicable to the alpm types.
 */
class AlpmWrapper {
public:
    static AlpmWrapper& instance();
    
    ~AlpmWrapper();
    
    // Disable copy and move
    AlpmWrapper(const AlpmWrapper&) = delete;
    AlpmWrapper& operator=(const AlpmWrapper&) = delete;
    AlpmWrapper(AlpmWrapper&&) = delete;
    AlpmWrapper& operator=(AlpmWrapper&&) = delete;
    
    bool initialize();
    void release();
    void refreshDatabases();
    
    QVector<PackageInfo> searchPackages(const QString& query);
    QVector<PackageInfo> getInstalledPackages();
    bool isPackageInstalled(const QString& packageName);
    PackageInfo getPackageInfo(const QString& packageName);
    /** Returns list of paths (relative to root) of files in the package, or empty if not found. */
    QStringList getPackageFileList(const QString& packageName);
    /** Returns sync package filename (e.g. firefox-131.0-1-x86_64.pkg.tar.zst) for cache lookup, or empty. */
    QString getPackageFilename(const QString& packageName);
    QVector<UpdateInfo> getAvailableUpdates();
    
private:
    AlpmWrapper();
    
    alpm_handle_t* m_handle = nullptr;
    alpm_list_t* m_syncDbs = nullptr;
    std::mutex m_mutex;
    bool m_initialized = false;
    
    QStringList convertDependList(alpm_list_t* deps);
    void searchInDatabase(alpm_db_t* db, const QString& query, 
                         QVector<PackageInfo>& results);
    QStringList getEnabledRepositories() const;
};

#endif // ALPM_WRAPPER_H
