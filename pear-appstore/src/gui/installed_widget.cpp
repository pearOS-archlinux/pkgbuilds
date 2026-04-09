#include "installed_widget.h"
#include "package_card.h"
#include "package_details_dialog.h"
#include "loading_overlay.h"
#include "scroll_bar_fade.h"
#include "../core/alpm_wrapper.h"
#include "../utils/logger.h"
#include "../utils/app_cache.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QShowEvent>
#include <QTimer>
#include <QtConcurrent>

InstalledWidget::InstalledWidget(QWidget* parent)
    : QWidget(parent)
    , m_filterInput(new QLineEdit(this))
    , m_scrollArea(new QScrollArea(this))
    , m_contentWidget(new QWidget())
    , m_gridLayout(new QGridLayout(m_contentWidget))
    , m_statusLabel(new QLabel(this))
    , m_countLabel(new QLabel(this)) {
    
    setupUi();
}

void InstalledWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (!m_initialLoadDone) {
        m_initialLoadDone = true;
        loadInstalledPackages();
    }
}

void InstalledWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Header
    auto* headerLayout = new QHBoxLayout();
    
    auto* titleLabel = new QLabel("Installed Packages", this);
    auto titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch();
    
    m_countLabel->setStyleSheet("font-size: 14px; color: #888;");
    headerLayout->addWidget(m_countLabel);
    
    auto* refreshButton = new QPushButton("Refresh", this);
    connect(refreshButton, &QPushButton::clicked, this, &InstalledWidget::refreshPackages);
    headerLayout->addWidget(refreshButton);
    
    mainLayout->addLayout(headerLayout);
    
    // Filter
    m_filterInput->setCursor(Qt::ArrowCursor);
    m_filterInput->setPlaceholderText("Filter installed packages...");
    m_filterInput->setMinimumHeight(35);
    m_filterInput->setClearButtonEnabled(true);
    connect(m_filterInput, &QLineEdit::textChanged, 
            this, &InstalledWidget::onFilterTextChanged);
    mainLayout->addWidget(m_filterInput);
    
    m_contentStack = new QStackedWidget(this);
    m_contentStack->addWidget(new LoadingOverlay(this));
    
    auto* contentPage = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentPage);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setText("Loading installed packages...");
    contentLayout->addWidget(m_statusLabel);
    
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gridLayout->setSpacing(0);
    m_gridLayout->setContentsMargins(10, 0, 10, 0);
    contentLayout->addWidget(m_scrollArea);
    applyScrollBarFade(m_scrollArea);
    m_contentStack->addWidget(contentPage);
    mainLayout->addWidget(m_contentStack);
    setLayout(mainLayout);
}

void InstalledWidget::loadInstalledPackages() {
    if (m_contentStack) m_contentStack->setCurrentIndex(0); // spinner până la încărcare completă
    m_statusLabel->setText(tr("Loading installed packages..."));
    m_statusLabel->show();

    QVector<PackageInfo> cached = AppCache::loadInstalledCache();
    if (!cached.isEmpty()) {
        m_allPackages = cached;
        m_filteredPackages = cached;
        m_countLabel->setText(QString("%1 packages installed").arg(cached.size()));
        displayPackages(cached);
    }

    (void)QtConcurrent::run([this]() {
        auto packages = AlpmWrapper::instance().getInstalledPackages();
        if (!packages.isEmpty()) {
            AppCache::saveInstalledCache(packages);
        }
        QMetaObject::invokeMethod(this, [this, packages]() {
            if (!packages.isEmpty()) {
                m_allPackages = packages;
                m_filteredPackages = packages;
                m_countLabel->setText(QString("%1 packages installed").arg(packages.size()));
                displayPackages(packages);
                Logger::info(QString("Loaded %1 installed packages").arg(packages.size()));
            } else if (m_allPackages.isEmpty()) {
                m_statusLabel->setText(tr("No packages found"));
                if (m_contentStack) m_contentStack->setCurrentIndex(1);
            }
        }, Qt::QueuedConnection);
    });
}

void InstalledWidget::refreshPackages() {
    clearResults();
    loadInstalledPackages();
}

void InstalledWidget::displayPackages(const QVector<PackageInfo>& packages) {
    clearResults();
    if (packages.isEmpty()) {
        m_statusLabel->setText(tr("No packages found"));
        m_statusLabel->show();
        if (m_contentStack) m_contentStack->setCurrentIndex(1);
        return;
    }
    m_displayQueue = packages;
    m_displayIndex = 0;
    m_displayGeneration++;
    m_countLabel->setText(QString("%1 packages installed").arg(packages.size()));
    addNextBatch();
}

void InstalledWidget::addNextBatch() {
    const int gen = m_displayGeneration;
    const int columns = 2;
    const int total = m_displayQueue.size();
    int row = m_displayIndex / columns;
    int col = m_displayIndex % columns;

    for (int n = 0; n < kBatchSize && m_displayIndex < total; n++, m_displayIndex++) {
        const auto& pkg = m_displayQueue.at(m_displayIndex);
        auto* card = new PackageCard(pkg, m_contentWidget, PackageCard::Discover);
        card->updateInstallStatus(true);
        connect(card, &PackageCard::clicked, this, &InstalledWidget::onPackageClicked);
        m_gridLayout->addWidget(card, row, col);
        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }

    if (m_displayIndex >= total) {
        m_gridLayout->setRowStretch(row + 1, 1);
        m_statusLabel->hide();
        if (m_contentStack) m_contentStack->setCurrentIndex(1);
        return;
    }
    if (gen == m_displayGeneration) {
        QTimer::singleShot(0, this, &InstalledWidget::addNextBatch);
    }
}

void InstalledWidget::clearResults() {
    while (auto* item = m_gridLayout->takeAt(0)) {
        if (auto* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void InstalledWidget::filterPackages(const QString& query) {
    if (query.isEmpty()) {
        m_filteredPackages = m_allPackages;
    } else {
        m_filteredPackages.clear();
        QString lowerQuery = query.toLower();
        
        for (const auto& pkg : m_allPackages) {
            if (pkg.name.toLower().contains(lowerQuery) ||
                pkg.description.toLower().contains(lowerQuery)) {
                m_filteredPackages.append(pkg);
            }
        }
    }
    
    m_countLabel->setText(QString("%1 of %2 packages")
                         .arg(m_filteredPackages.size())
                         .arg(m_allPackages.size()));
    
    displayPackages(m_filteredPackages);
}

void InstalledWidget::onFilterTextChanged(const QString& text) {
    filterPackages(text);
}

void InstalledWidget::onPackageClicked(const PackageInfo& info) {
    Logger::info(QString("Package clicked: %1").arg(info.name));
    emit openPackageRequested(info);
}
