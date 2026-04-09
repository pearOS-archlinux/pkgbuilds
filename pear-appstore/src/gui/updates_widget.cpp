#include "updates_widget.h"
#include "loading_overlay.h"
#include "scroll_bar_fade.h"
#include "../core/alpm_wrapper.h"
#include "../core/aur_helper.h"
#include "../core/package_manager.h"
#include "../utils/logger.h"
#include "../utils/app_cache.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextCursor>
#include <QtConcurrent>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QPushButton>

class UpdateItem : public QWidget {
    Q_OBJECT

public:
    UpdateItem(const UpdateInfo& info, QWidget* parent = nullptr)
        : QWidget(parent), m_info(info) {

        setProperty("class", "package-card");
        setCursor(Qt::ArrowCursor);
        setMinimumHeight(88);
        setMinimumWidth(280);

        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 12, 16, 0);
        mainLayout->setSpacing(0);

        const int iconSize = 56;
        QLabel* iconLabel = new QLabel(this);
        iconLabel->setFixedSize(iconSize, iconSize);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet(
            "background-color: #27272a; border-radius: 12px; border: none;"
        );
        setPlaceholderIcon(iconLabel, iconSize);

        QLabel* nameLabel = new QLabel(m_info.name, this);
        nameLabel->setWordWrap(false);
        auto nameFont = nameLabel->font();
        nameFont.setPointSize(15);
        nameFont.setBold(true);
        nameLabel->setFont(nameFont);
        nameLabel->setStyleSheet("color: #f9fafb;");

        QString descText = QString("%1 → %2").arg(m_info.oldVersion, m_info.newVersion);
        if (m_info.downloadSize > 0) {
            descText += QString(" • %1").arg(formatSize(m_info.downloadSize));
        }
        descText += QString(" • %1").arg(m_info.repository);
        QLabel* descLabel = new QLabel(descText, this);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet("color: #a1a1aa; font-size: 12px;");
        descLabel->setMaximumHeight(36);

        auto* textColumn = new QVBoxLayout();
        textColumn->setContentsMargins(0, 0, 0, 0);
        textColumn->setSpacing(2);
        textColumn->addWidget(nameLabel, 0);
        textColumn->addWidget(descLabel, 0);

        QPushButton* updateButton = new QPushButton(tr("Update"), this);
        updateButton->setCursor(Qt::ArrowCursor);
        updateButton->setObjectName("discoverDownloadButton");
        updateButton->setFixedSize(58, 28);
        connect(updateButton, &QPushButton::clicked, this, [this]() {
            emit updateRequested(m_info.name, m_info.repository);
        });

        auto* rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(14);
        rowLayout->addWidget(iconLabel, 0);
        rowLayout->addLayout(textColumn, 1);
        rowLayout->addWidget(updateButton, 0, Qt::AlignVCenter);
        mainLayout->addLayout(rowLayout, 0);

        QFrame* separator = new QFrame(this);
        separator->setObjectName("discoverCardSeparator");
        separator->setFixedHeight(1);
        separator->setStyleSheet("QFrame#discoverCardSeparator { background-color: rgba(255,255,255,0.08); border: none; }");
        mainLayout->addWidget(separator, 0);
        mainLayout->setContentsMargins(16, 12, 16, 12);

        setLayout(mainLayout);
    }

signals:
    void updateRequested(const QString& packageName, const QString& repository);

private:
    UpdateInfo m_info;

    void setPlaceholderIcon(QLabel* label, int size) {
        QPixmap px(size, size);
        px.fill(Qt::transparent);
        QPainter p(&px);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        QColor bg(QStringLiteral("#52525b"));
        p.setBrush(bg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(0, 0, size, size, size / 4, size / 4);
        QString letter = m_info.name.isEmpty() ? QLatin1String("?") : m_info.name.left(1).toUpper();
        p.setPen(QColor(QStringLiteral("#a1a1aa")));
        QFont f = p.font();
        f.setPointSize(size / 2);
        f.setBold(true);
        p.setFont(f);
        p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, letter);
        p.end();
        label->setPixmap(px);
    }

    QString formatSize(qint64 bytes) {
        const qint64 KB = 1024;
        const qint64 MB = KB * 1024;
        const qint64 GB = MB * 1024;

        if (bytes >= GB) {
            return QString("%1 GB").arg(bytes / static_cast<double>(GB), 0, 'f', 2);
        } else if (bytes >= MB) {
            return QString("%1 MB").arg(bytes / static_cast<double>(MB), 0, 'f', 1);
        } else if (bytes >= KB) {
            return QString("%1 KB").arg(bytes / static_cast<double>(KB), 0, 'f', 0);
        }
        return QString("%1 B").arg(bytes);
    }
};

UpdatesWidget::UpdatesWidget(QWidget* parent)
    : QWidget(parent)
    , m_searchInput(new QLineEdit(this))
    , m_scrollArea(new QScrollArea(this))
    , m_contentWidget(new QWidget())
    , m_gridLayout(new QGridLayout(m_contentWidget))
    , m_statusLabel(new QLabel(this))
    , m_countLabel(new QLabel(this))
    , m_updateAllButton(new QPushButton("Update All", this))
    , m_checkButton(new QPushButton("Check for Updates", this))
    , m_progressWidget(new QWidget(this))
    , m_progressBar(new QProgressBar(this))
    , m_progressLabel(new QLabel(this))
    , m_toggleLogButton(new QPushButton("Show Logs", this))
    , m_logWidget(new QWidget(this))
    , m_logViewer(new QTextEdit(this)) {
    
    setupUi();
    
    // Connect to PackageManager signals
    connect(&PackageManager::instance(), &PackageManager::operationStarted,
            this, &UpdatesWidget::onOperationStarted);
    connect(&PackageManager::instance(), &PackageManager::operationOutput,
            this, &UpdatesWidget::onOperationOutput);
    connect(&PackageManager::instance(), &PackageManager::operationCompleted,
            this, &UpdatesWidget::onOperationCompleted);
    connect(&PackageManager::instance(), &PackageManager::operationError,
            this, &UpdatesWidget::onOperationError);
}

void UpdatesWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Header
    auto* headerLayout = new QHBoxLayout();
    
    auto* titleLabel = new QLabel("Available Updates", this);
    auto titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch();
    
    m_countLabel->setStyleSheet("font-size: 14px; color: #888;");
    headerLayout->addWidget(m_countLabel);
    
    m_checkButton->setMinimumHeight(35);
    connect(m_checkButton, &QPushButton::clicked, this, &UpdatesWidget::checkForUpdates);
    headerLayout->addWidget(m_checkButton);
    
    m_updateAllButton->setMinimumHeight(35);
    m_updateAllButton->setMinimumWidth(120);
    m_updateAllButton->setEnabled(false);
    connect(m_updateAllButton, &QPushButton::clicked, this, &UpdatesWidget::onUpdateAll);
    headerLayout->addWidget(m_updateAllButton);
    
    mainLayout->addLayout(headerLayout);
    
    m_contentStack = new QStackedWidget(this);
    m_contentStack->addWidget(new LoadingOverlay(this));
    
    auto* contentPage = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentPage);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    
    m_searchInput->setCursor(Qt::ArrowCursor);
    m_searchInput->setPlaceholderText("Search updates...");
    m_searchInput->setMinimumHeight(35);
    m_searchInput->setClearButtonEnabled(true);
    m_searchInput->setEnabled(false);
    connect(m_searchInput, &QLineEdit::textChanged, this, &UpdatesWidget::onSearchTextChanged);
    contentLayout->addWidget(m_searchInput);
    
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setText("Click 'Check for Updates' to scan for available updates");
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
    
    // Progress bar section (hidden by default)
    auto* progressLayout = new QVBoxLayout(m_progressWidget);
    progressLayout->setContentsMargins(20, 0, 20, 20);
    progressLayout->setSpacing(8);
    
    m_progressLabel->setStyleSheet("color: #a1a1aa; font-size: 12px;");
    m_progressLabel->setAlignment(Qt::AlignCenter);
    progressLayout->addWidget(m_progressLabel);
    
    m_progressBar->setMinimumHeight(20);
    m_progressBar->setMaximumHeight(20);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p%");
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "    border: none;"
        "    border-radius: 4px;"
        "    background-color: #27272a;"
        "    color: #fafafa;"
        "    text-align: center;"
        "    font-size: 11px;"
        "}"
        "QProgressBar::chunk {"
        "    border-radius: 4px;"
        "    background-color: #3b82f6;"
        "}"
    );
    progressLayout->addWidget(m_progressBar);
    
    // Toggle log button
    m_toggleLogButton->setStyleSheet(
        "QPushButton {"
        "    background: none;"
        "    border: none;"
        "    color: #3b82f6;"
        "    text-decoration: underline;"
        "    font-size: 11px;"
        "    padding: 4px;"
        "}"
        "QPushButton:hover {"
        "    color: #60a5fa;"
        "}"
    );
    connect(m_toggleLogButton, &QPushButton::clicked, this, &UpdatesWidget::toggleLogViewer);
    progressLayout->addWidget(m_toggleLogButton, 0, Qt::AlignCenter);
    
    m_progressWidget->hide();
    mainLayout->addWidget(m_progressWidget, 0);
    
    // Log viewer section (hidden by default)
    auto* logLayout = new QVBoxLayout(m_logWidget);
    logLayout->setContentsMargins(20, 0, 20, 20);
    logLayout->setSpacing(8);
    
    m_logViewer->setCursor(Qt::ArrowCursor);
    m_logViewer->setReadOnly(true);
    m_logViewer->setMaximumHeight(200);
    m_logViewer->setStyleSheet(
        "QTextEdit {"
        "    background-color: #1e2123;"
        "    border: 1px solid #27272a;"
        "    border-radius: 6px;"
        "    color: #d4d4d8;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    font-size: 11px;"
        "    padding: 8px;"
        "}"
    );
    logLayout->addWidget(m_logViewer);
    
    m_logWidget->hide();
    mainLayout->addWidget(m_logWidget, 0);
    
    setLayout(mainLayout);
}

void UpdatesWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (m_updates.isEmpty()) {
        QVector<UpdateInfo> cached = AppCache::loadUpdatesCache();
        if (!cached.isEmpty()) {
            m_updates = cached;
            m_filteredUpdates = cached;
            emit updatesCountChanged(cached.size());
            if (m_contentStack) m_contentStack->setCurrentIndex(1);
            m_statusLabel->hide();
            m_countLabel->setText(QString("%1 updates available (cached)").arg(cached.size()));
            m_updateAllButton->setEnabled(true);
            m_searchInput->setEnabled(true);
            displayUpdates(cached);
        } else {
            checkForUpdates();
        }
    }
}

void UpdatesWidget::checkForUpdates() {
    if (m_contentStack) {
        m_contentStack->setCurrentIndex(0);
    }
    m_statusLabel->setText("Checking for updates...");
    m_statusLabel->show();
    m_checkButton->setEnabled(false);
    m_checkButton->setText("Checking...");
    m_updateAllButton->setEnabled(false);
    
    clearUpdates();
    
    (void)QtConcurrent::run([this]() {
        auto updates = AlpmWrapper::instance().getAvailableUpdates();
        AurHelper aurHelper;
        auto aurUpdates = aurHelper.checkAurUpdates();
        updates.append(aurUpdates);
        
        AppCache::saveUpdatesCache(updates);
        QMetaObject::invokeMethod(this, [this, updates]() {
            m_updates = updates;
            m_filteredUpdates = updates;
            emit updatesCountChanged(updates.size());
            if (m_contentStack) {
                m_contentStack->setCurrentIndex(1);
            }
            m_checkButton->setEnabled(true);
            m_checkButton->setText("Check for Updates");
            
            if (updates.isEmpty()) {
                m_statusLabel->setText("Your system is up to date!");
                m_statusLabel->show();
                m_countLabel->clear();
                m_searchInput->setEnabled(false);
                emit updatesCountChanged(0);
            } else {
                m_statusLabel->hide();
                m_countLabel->setText(QString("%1 updates available")
                                     .arg(updates.size()));
                m_updateAllButton->setEnabled(true);
                m_searchInput->setEnabled(true);
                QString searchText = m_searchInput->text();
                if (!searchText.isEmpty()) {
                    filterUpdates(searchText);
                } else {
                    displayUpdates(updates);
                }
            }
            Logger::info(QString("Found %1 updates").arg(updates.size()));
        }, Qt::QueuedConnection);
    });
}

void UpdatesWidget::displayUpdates(const QVector<UpdateInfo>& updates) {
    clearUpdates();
    int row = 0;
    int col = 0;
    const int columns = 2;
    for (const auto& update : updates) {
        auto* item = new UpdateItem(update, m_contentWidget);
        connect(item, &UpdateItem::updateRequested,
                this, &UpdatesWidget::onUpdateSingle);
        m_gridLayout->addWidget(item, row, col);
        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
    m_gridLayout->setRowStretch(row + 1, 1);
}

void UpdatesWidget::clearUpdates() {
    while (auto* item = m_gridLayout->takeAt(0)) {
        if (auto* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void UpdatesWidget::filterUpdates(const QString& searchText) {
    if (searchText.isEmpty()) {
        m_filteredUpdates = m_updates;
        displayUpdates(m_filteredUpdates);
        m_countLabel->setText(QString("%1 updates available").arg(m_updates.size()));
        return;
    }
    
    QString lowerSearch = searchText.toLower();
    m_filteredUpdates.clear();
    
    for (const auto& update : m_updates) {
        if (update.name.toLower().contains(lowerSearch)) {
            m_filteredUpdates.append(update);
        }
    }
    
    displayUpdates(m_filteredUpdates);
    
    // Update count label to show filtered count
    if (m_filteredUpdates.size() == m_updates.size()) {
        m_countLabel->setText(QString("%1 updates available").arg(m_updates.size()));
    } else {
        m_countLabel->setText(QString("%1 of %2 updates").arg(m_filteredUpdates.size()).arg(m_updates.size()));
    }
}

void UpdatesWidget::onSearchTextChanged(const QString& text) {
    filterUpdates(text);
}

void UpdatesWidget::onUpdateAll() {
    PackageManager::instance().updateAllPackages();
}

void UpdatesWidget::onUpdateSingle(const QString& packageName, const QString& repository) {
    PackageManager::instance().updatePackage(packageName, repository);
}

QString UpdatesWidget::formatSize(qint64 bytes) {
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / static_cast<double>(GB), 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / static_cast<double>(MB), 0, 'f', 1);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / static_cast<double>(KB), 0, 'f', 0);
    }
    return QString("%1 B").arg(bytes);
}

void UpdatesWidget::showProgress(const QString& message) {
    m_progressLabel->setText(message);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressWidget->show();
    m_progressBar->show();
    m_progressLabel->show();
    m_logViewer->clear();
}

void UpdatesWidget::hideProgress() {
    // Hide the progress bar and label, but keep the widget and toggle button visible
    m_progressBar->hide();
    m_progressLabel->hide();
    // Don't hide m_progressWidget - keeps the toggle button visible
    // Don't hide the log widget or reset log visibility
    // This allows users to review logs after operation completes
}

void UpdatesWidget::toggleLogViewer() {
    m_logVisible = !m_logVisible;
    if (m_logVisible) {
        m_logWidget->show();
        m_toggleLogButton->setText("Hide Logs");
    } else {
        m_logWidget->hide();
        m_toggleLogButton->setText("Show Logs");
    }
}

void UpdatesWidget::onOperationStarted(const QString& message) {
    showProgress(message);
    m_updateAllButton->setEnabled(false);
    m_checkButton->setEnabled(false);
    
    // Disable all individual update buttons
    for (int i = 0; i < m_gridLayout->count(); ++i) {
        if (auto* item = m_gridLayout->itemAt(i)) {
            if (auto* widget = item->widget()) {
                widget->setEnabled(false);
            }
        }
    }
}

void UpdatesWidget::onOperationOutput(const QString& output) {
    m_logViewer->append(output);
    
    // Auto-scroll to bottom
    auto cursor = m_logViewer->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logViewer->setTextCursor(cursor);
    
    // Try to parse progress information from output
    // This is a simple implementation - could be enhanced
    if (output.contains("downloading", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Downloading packages...");
        m_progressBar->setRange(0, 0); // Indeterminate
    } else if (output.contains("installing", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Installing packages...");
        m_progressBar->setRange(0, 0); // Indeterminate
    }
}

void UpdatesWidget::onOperationCompleted(bool success, const QString& message) {
    // Refresh ALPM state so subsequent queries reflect the changes
    AlpmWrapper::instance().release();
    AlpmWrapper::instance().initialize();
    
    hideProgress();
    
    m_updateAllButton->setEnabled(!m_updates.isEmpty());
    m_checkButton->setEnabled(true);
    
    // Re-enable all individual update buttons
    for (int i = 0; i < m_gridLayout->count(); ++i) {
        if (auto* item = m_gridLayout->itemAt(i)) {
            if (auto* widget = item->widget()) {
                widget->setEnabled(true);
            }
        }
    }
    
    if (success) {
        QMessageBox::information(this, "Success", message);
        checkForUpdates();
    } else {
        QMessageBox::warning(this, "Operation Failed", message);
    }
}

void UpdatesWidget::onOperationError(const QString& error) {
    // Refresh ALPM state (best-effort)
    AlpmWrapper::instance().release();
    AlpmWrapper::instance().initialize();
    
    hideProgress();
    
    m_updateAllButton->setEnabled(!m_updates.isEmpty());
    m_checkButton->setEnabled(true);
    
    // Re-enable all individual update buttons
    for (int i = 0; i < m_gridLayout->count(); ++i) {
        if (auto* item = m_gridLayout->itemAt(i)) {
            if (auto* widget = item->widget()) {
                widget->setEnabled(true);
            }
        }
    }
    
    QMessageBox::critical(this, "Error", error);
}

#include "updates_widget.moc"
