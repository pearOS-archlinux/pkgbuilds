#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

// Wraps `balooctl6 config` — the real KDE Search / file indexing backend
// behind Plasma's Spotlight-equivalent (KRunner + Baloo).
class SpotlightManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool indexingEnabled READ indexingEnabled NOTIFY changed)
    Q_PROPERTY(bool indexHidden     READ indexHidden     NOTIFY changed)
    Q_PROPERTY(bool indexContents   READ indexContents   NOTIFY changed)
    Q_PROPERTY(QVariantList includeFolders READ includeFolders NOTIFY changed)
    Q_PROPERTY(QVariantList excludeFolders READ excludeFolders NOTIFY changed)
    Q_PROPERTY(QString indexedFileCount    READ indexedFileCount    NOTIFY statusChanged)
    Q_PROPERTY(QString indexSize           READ indexSize           NOTIFY statusChanged)

public:
    explicit SpotlightManager(QObject *parent = nullptr);

    bool indexingEnabled() const { return m_indexingEnabled; }
    bool indexHidden()     const { return m_indexHidden; }
    bool indexContents()   const { return m_indexContents; }
    QVariantList includeFolders() const { return m_includeFolders; }
    QVariantList excludeFolders() const { return m_excludeFolders; }
    QString indexedFileCount() const { return m_indexedFileCount; }
    QString indexSize()        const { return m_indexSize; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshStatus();
    Q_INVOKABLE void setIndexingEnabled(bool v);
    Q_INVOKABLE void setIndexHidden(bool v);
    Q_INVOKABLE void setIndexContents(bool v);
    Q_INVOKABLE void addExcludeFolder(const QString &path);
    Q_INVOKABLE void removeExcludeFolder(const QString &path);
    Q_INVOKABLE void addIncludeFolder(const QString &path);
    Q_INVOKABLE void removeIncludeFolder(const QString &path);

signals:
    void changed();
    void statusChanged();

private:
    bool m_indexingEnabled = true;
    bool m_indexHidden     = false;
    bool m_indexContents   = true;
    QVariantList m_includeFolders;
    QVariantList m_excludeFolders;
    QString m_indexedFileCount;
    QString m_indexSize;

    void run(const QString &cmd, std::function<void(QString)> cb);
};
