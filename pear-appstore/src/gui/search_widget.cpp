#include "search_widget.h"
#include "package_card.h"
#include "package_details_dialog.h"
#include "loading_overlay.h"
#include "scroll_bar_fade.h"
#include "../core/alpm_wrapper.h"
#include "../core/aur_helper.h"
#include "../utils/logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QtConcurrent>
#include <QTimer>
#include <QResizeEvent>

SearchWidget::SearchWidget(QWidget* parent)
    : QWidget(parent)
    , m_searchInput(new QLineEdit(this))
    , m_searchButton(new QPushButton("Search", this))
    , m_filterCombo(new QComboBox(this))
    , m_scrollArea(new QScrollArea(this))
    , m_contentWidget(new QWidget())
    , m_gridLayout(new QGridLayout(m_contentWidget))
    , m_statusLabel(new QLabel(this))
    , m_aurHelper(std::make_unique<AurHelper>(this)) {
    
    setupUi();
    
    connect(m_aurHelper.get(), &AurHelper::searchCompleted,
            this, &SearchWidget::onAurSearchCompleted);
}

void SearchWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    m_searchInput->setVisible(false);
    connect(m_searchInput, &QLineEdit::returnPressed, this, &SearchWidget::onSearchClicked);

    // Title: "Results for \"query\"" (actualizat la setSearchQuery / la afișare rezultate)
    m_titleLabel = new QLabel(tr("Search"), this);
    auto titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    mainLayout->addWidget(m_titleLabel);

    // Rând: label "Filters" + dropdown (All, Core, Extra, AUR, etc.)
    auto* filterRow = new QHBoxLayout();
    auto* filterLabel = new QLabel(tr("Filters"), this);
    filterLabel->setStyleSheet("color: #a1a1aa; font-size: 14px;");
    filterRow->addWidget(filterLabel);
    m_filterCombo->addItem(tr("All"), "all");
    m_filterCombo->addItem("Core", "core");
    m_filterCombo->addItem("Extra", "extra");
    m_filterCombo->addItem("AUR", "AUR");
    m_filterCombo->setMinimumHeight(32);
    m_filterCombo->setMinimumWidth(140);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SearchWidget::onFilterChanged);
    filterRow->addWidget(m_filterCombo);
    filterRow->addStretch();
    mainLayout->addLayout(filterRow);

    m_searchButton->setVisible(false);
    
    m_contentStack = new QStackedWidget(this);
    m_contentStack->addWidget(new LoadingOverlay(this));
    
    auto* resultsPage = new QWidget(this);
    auto* resultsLayout = new QVBoxLayout(resultsPage);
    resultsLayout->setContentsMargins(0, 0, 0, 0);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->hide();
    resultsLayout->addWidget(m_statusLabel);
    
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gridLayout->setSpacing(15);
    m_gridLayout->setContentsMargins(10, 10, 10, 10);
    resultsLayout->addWidget(m_scrollArea);
    applyScrollBarFade(m_scrollArea);
    m_contentStack->addWidget(resultsPage);
    mainLayout->addWidget(m_contentStack);
    setLayout(mainLayout);
}

void SearchWidget::onSearchClicked() {
    QString query = m_searchInput->text().trimmed();

    if (query.isEmpty()) {
        if (m_titleLabel) m_titleLabel->setText(tr("Search"));
        m_statusLabel->setText(tr("Enter a search term in the sidebar"));
        m_statusLabel->show();
        if (m_contentStack) m_contentStack->setCurrentIndex(1);
        return;
    }

    if (m_titleLabel) m_titleLabel->setText(tr("Results for \"%1\"").arg(query));
    m_searchButton->setEnabled(false);
    m_searchButton->setText(tr("Searching..."));
    m_statusLabel->setText("Searching...");
    m_statusLabel->show();
    if (m_contentStack) {
        m_contentStack->setCurrentIndex(0);
    }
    clearResults();
    QTimer::singleShot(0, this, [this]() {
        performSearch();
    });
}

void SearchWidget::performSearch() {
    QString query = m_searchInput->text().trimmed().toLower().replace(' ', '-');
    QString aurQuery = m_searchInput->text().trimmed();
    
    if (query.isEmpty()) {
        m_searchButton->setEnabled(true);
        m_searchButton->setText("Search");
        if (m_contentStack) m_contentStack->setCurrentIndex(1);
        return;
    }
    
    auto it = m_searchCache.constFind(query);
    if (it != m_searchCache.constEnd()) {
        QVector<PackageInfo> cached = it.value();
        m_allResults = cached;
        m_aurHelper->searchPackages(aurQuery);
        return;
    }
    
    (void)QtConcurrent::run([this, query, aurQuery]() {
        QVector<PackageInfo> results = AlpmWrapper::instance().searchPackages(query);
        QMetaObject::invokeMethod(this, [this, results, query, aurQuery]() {
            if (m_searchCache.size() >= kMaxSearchCacheSize) {
                m_searchCache.erase(m_searchCache.begin());
            }
            m_searchCache.insert(query, results);
            m_allResults = results;
            m_aurHelper->searchPackages(aurQuery);
        }, Qt::QueuedConnection);
    });
}

void SearchWidget::onAurSearchCompleted(const QVector<PackageInfo>& results) {
    m_allResults.append(results);
    m_searchButton->setEnabled(true);
    m_searchButton->setText("Search");
    if (m_contentStack) {
        m_contentStack->setCurrentIndex(1);
    }
    if (m_allResults.isEmpty()) {
        m_statusLabel->setText("No results found");
        m_statusLabel->show();
        return;
    }
    m_statusLabel->hide();
    onFilterChanged(m_filterCombo->currentIndex());
    
    Logger::info(QString("Search completed: %1 results").arg(m_allResults.size()));
}

void SearchWidget::onFilterChanged(int index) {
    QString filter = m_filterCombo->itemData(index).toString();
    
    if (filter == "all") {
        displayResults(m_allResults);
    } else {
        QVector<PackageInfo> filtered;
        for (const auto& pkg : m_allResults) {
            if (pkg.repository == filter) {
                filtered.append(pkg);
            }
        }
        displayResults(filtered);
    }
}

void SearchWidget::displayResults(const QVector<PackageInfo>& results) {
    clearResults();
    m_currentResults = results;

    if (results.isEmpty()) {
        m_statusLabel->setText("No results found for selected filter");
        m_statusLabel->show();
        return;
    }

    for (const auto& pkg : results) {
        auto* card = new PackageCard(pkg, m_contentWidget, PackageCard::SearchResults);
        connect(card, &PackageCard::clicked, this, &SearchWidget::onPackageClicked);
        m_resultCards.append(card);
        card->loadAppStreamAsync();
    }
    m_columnCount = columnCountForWidth(m_scrollArea->viewport()->width());
    refreshGridLayout();
}

int SearchWidget::columnCountForWidth(int width) {
    const int minCardWidth = 280;
    const int spacing = 15;
    const int margins = 20;
    int n = (width - margins + spacing) / (minCardWidth + spacing);
    return qMax(1, qMin(2, n));
}

void SearchWidget::refreshGridLayout() {
    while (QLayoutItem* item = m_gridLayout->takeAt(0)) {
        delete item;
    }
    int row = 0, col = 0;
    for (auto* card : m_resultCards) {
        m_gridLayout->addWidget(card, row, col);
        col++;
        if (col >= m_columnCount) {
            col = 0;
            row++;
        }
    }
    m_gridLayout->setRowStretch(row + 1, 1);
}

void SearchWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_resultCards.isEmpty()) return;
    int cols = columnCountForWidth(m_scrollArea->viewport()->width());
    if (cols != m_columnCount) {
        m_columnCount = cols;
        refreshGridLayout();
    }
}

void SearchWidget::clearResults() {
    for (auto* card : m_resultCards) {
        card->deleteLater();
    }
    m_resultCards.clear();
    while (QLayoutItem* item = m_gridLayout->takeAt(0)) {
        delete item;
    }
}

void SearchWidget::onPackageClicked(const PackageInfo& info) {
    Logger::info(QString("Package clicked: %1").arg(info.name));
    
    PackageInfo fullInfo = info;
    
    // For AUR packages, fetch complete details including maintainer, URL, dependencies
    if (info.repository.toLower() == "aur") {
        auto* aurHelper = new AurHelper(this);
        QEventLoop loop;
        
        connect(aurHelper, &AurHelper::packageInfoReceived, [&](const PackageInfo& detailedInfo) {
            fullInfo = detailedInfo;
            loop.quit();
        });
        
        connect(aurHelper, &AurHelper::error, [&](const QString& errorMsg) {
            Logger::warning(QString("Failed to fetch AUR details for %1: %2").arg(info.name, errorMsg));
            loop.quit();
        });
        
        aurHelper->getPackageInfo(info.name);
        loop.exec();
        
        aurHelper->deleteLater();
    }
    
    emit openPackageRequested(fullInfo);
}

void SearchWidget::updateRepositoryList(bool multilibEnabled, bool chaoticAurEnabled) {
    // Save the current selection
    int currentIndex = m_filterCombo->currentIndex();
    QString currentFilter = m_filterCombo->itemData(currentIndex).toString();
    
    // Check if multilib already exists in the list
    bool multilibExists = false;
    for (int i = 0; i < m_filterCombo->count(); ++i) {
        if (m_filterCombo->itemData(i).toString() == "multilib") {
            multilibExists = true;
            break;
        }
    }
    
    // Check if chaotic-aur already exists in the list
    bool chaoticAurExists = false;
    for (int i = 0; i < m_filterCombo->count(); ++i) {
        if (m_filterCombo->itemData(i).toString() == "chaotic-aur") {
            chaoticAurExists = true;
            break;
        }
    }
    
    // Handle multilib
    if (multilibEnabled && !multilibExists) {
        // Add multilib to the dropdown (insert before AUR)
        int aurIndex = m_filterCombo->findData("AUR");
        if (aurIndex != -1) {
            m_filterCombo->insertItem(aurIndex, "Multilib", "multilib");
        } else {
            m_filterCombo->addItem("Multilib", "multilib");
        }
        Logger::info("Added multilib repository to search filter");
    } else if (!multilibEnabled && multilibExists) {
        // Remove multilib from the dropdown
        int multilibIndex = m_filterCombo->findData("multilib");
        if (multilibIndex != -1) {
            m_filterCombo->removeItem(multilibIndex);
            Logger::info("Removed multilib repository from search filter");
        }
    }
    
    // Handle chaotic-aur
    if (chaoticAurEnabled && !chaoticAurExists) {
        // Add chaotic-aur to the dropdown (insert before AUR)
        int aurIndex = m_filterCombo->findData("AUR");
        if (aurIndex != -1) {
            m_filterCombo->insertItem(aurIndex, "Chaotic-AUR", "chaotic-aur");
        } else {
            m_filterCombo->addItem("Chaotic-AUR", "chaotic-aur");
        }
        Logger::info("Added chaotic-aur repository to search filter");
    } else if (!chaoticAurEnabled && chaoticAurExists) {
        // Remove chaotic-aur from the dropdown
        int chaoticAurIndex = m_filterCombo->findData("chaotic-aur");
        if (chaoticAurIndex != -1) {
            m_filterCombo->removeItem(chaoticAurIndex);
            Logger::info("Removed chaotic-aur repository from search filter");
        }
    }
    
    // Restore previous selection if it still exists
    int newIndex = m_filterCombo->findData(currentFilter);
    if (newIndex != -1) {
        m_filterCombo->setCurrentIndex(newIndex);
    } else {
        // If previous selection was removed, select "All"
        m_filterCombo->setCurrentIndex(0);
    }
}

void SearchWidget::setSearchQuery(const QString& query) {
    m_searchInput->setText(query);
    if (!query.isEmpty()) {
        m_titleLabel->setText(tr("Results for \"%1\"").arg(query));
        onSearchClicked();
    } else if (m_titleLabel) {
        m_titleLabel->setText(tr("Search"));
    }
}
