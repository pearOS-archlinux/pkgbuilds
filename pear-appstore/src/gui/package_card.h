#ifndef PACKAGE_CARD_H
#define PACKAGE_CARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFutureWatcher>
#include "../utils/types.h"
#include "../core/appstream_helper.h"

/**
 * @brief A clickable card widget displaying package information.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 */
class PackageCard : public QWidget {
    Q_OBJECT

public:
    enum DisplayStyle { Compact, Discover, SearchResults };
    explicit PackageCard(const PackageInfo& info, QWidget* parent = nullptr, DisplayStyle style = Compact);
    ~PackageCard() override = default;
    
    const PackageInfo& packageInfo() const { return m_info; }
    void updateInstallStatus(bool installed);
    void checkInstallStatus();
    /** Încarcă date AppStream în background și actualizează icon/descriere când sunt gata. */
    void loadAppStreamAsync();

signals:
    void clicked(const PackageInfo& info);
    
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    
private slots:
    void onAppStreamDataReady();

private:
    void setupUi();
    void setPlaceholderIcon(QLabel* label, int size);
    void applyAppStreamData(const AppStreamData& data);

    PackageInfo m_info;
    QFutureWatcher<AppStreamData>* m_appstreamWatcher = nullptr;
    
    DisplayStyle m_displayStyle = Compact;
    QPushButton* m_downloadButton = nullptr;
    // Qt parent-child managed widgets (non-owning pointers)
    QLabel* m_iconLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    QLabel* m_versionLabel = nullptr;
    QLabel* m_repositoryLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    /** Primul screenshot (doar Discover); afișat deasupra separatorului. */
    QLabel* m_screenshotLabel = nullptr;

    bool m_isInstalled = false;
};

#endif // PACKAGE_CARD_H
