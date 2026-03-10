#ifndef SEARCH_WIDGET_H
#define SEARCH_WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QVector>
#include <QStackedWidget>
#include <QMap>
#include <memory>
#include "../utils/types.h"
#include "../core/aur_helper.h"

class PackageCard;

/**
 * @brief Widget for searching packages across repositories.
 * 
 * Memory Management:
 * - m_aurHelper: Owned by std::unique_ptr for explicit lifetime management
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 * - Package cards are dynamically created/destroyed in displayResults/clearResults
 */
class SearchWidget : public QWidget {
    Q_OBJECT
    
signals:
    void openPackageRequested(const PackageInfo& info);
    
public:
    explicit SearchWidget(QWidget* parent = nullptr);
    ~SearchWidget() override = default;
    
public slots:
    void updateRepositoryList(bool multilibEnabled, bool chaoticAurEnabled = false);
    void setSearchQuery(const QString& query);
    
protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi();
    void performSearch();
    void displayResults(const QVector<PackageInfo>& results);
    void clearResults();
    void refreshGridLayout();
    static int columnCountForWidth(int width);

    // Qt parent-child managed widgets (non-owning pointers)
    QLineEdit* m_searchInput = nullptr;
    QPushButton* m_searchButton = nullptr;
    QComboBox* m_filterCombo = nullptr;
    QStackedWidget* m_contentStack = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QGridLayout* m_gridLayout = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    
    // Owned resources
    std::unique_ptr<AurHelper> m_aurHelper;
    
    QVector<PackageInfo> m_currentResults;
    QVector<PackageInfo> m_allResults;
    QVector<PackageCard*> m_resultCards;
    QMap<QString, QVector<PackageInfo>> m_searchCache;
    static constexpr int kMaxSearchCacheSize = 30;
    int m_columnCount = 3;
    
private slots:
    void onSearchClicked();
    void onAurSearchCompleted(const QVector<PackageInfo>& results);
    void onFilterChanged(int index);
    void onPackageClicked(const PackageInfo& info);
};

#endif // SEARCH_WIDGET_H
