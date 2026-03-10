#ifndef INSTALLED_WIDGET_H
#define INSTALLED_WIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLabel>
#include <QVector>
#include "../utils/types.h"

/**
 * @brief Widget displaying installed packages.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 * - Package cards are dynamically created/destroyed in displayPackages/clearResults
 */
class InstalledWidget : public QWidget {
    Q_OBJECT
    
signals:
    void openPackageRequested(const PackageInfo& info);
    
public:
    explicit InstalledWidget(QWidget* parent = nullptr);
    ~InstalledWidget() override = default;

    void refreshPackages();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUi();
    void loadInstalledPackages();
    void displayPackages(const QVector<PackageInfo>& packages);
    void addNextBatch();
    void filterPackages(const QString& query);
    void clearResults();
    
    // Qt parent-child managed widgets (non-owning pointers)
    QLineEdit* m_filterInput = nullptr;
    QStackedWidget* m_contentStack = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QGridLayout* m_gridLayout = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_countLabel = nullptr;
    
    QVector<PackageInfo> m_allPackages;
    QVector<PackageInfo> m_filteredPackages;
    bool m_initialLoadDone = false;
    QVector<PackageInfo> m_displayQueue;
    int m_displayIndex = 0;
    int m_displayGeneration = 0;
    static const int kBatchSize = 60;

private slots:
    void onPackageClicked(const PackageInfo& info);
    void onFilterTextChanged(const QString& text);
};

#endif // INSTALLED_WIDGET_H
