#include "package_details_dialog.h"
#include "../core/alpm_wrapper.h"
#include "../core/appstream_helper.h"
#include "../utils/cache_helper.h"
#include "../core/package_manager.h"
#include "../utils/logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QIcon>
#include <QFrame>
#include <QScrollArea>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextCursor>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtConcurrent>
#include <QApplication>
#include <QPalette>
#include <QPainter>
#include <QResizeEvent>
#include <QSettings>
#include <QTimer>

PackageDetailsDialog::PackageDetailsDialog(const PackageInfo& info, QWidget* parent, bool embedded)
    : QDialog(parent)
    , m_info(info)
    , m_network(new QNetworkAccessManager(this))
    , m_nameLabel(new QLabel(this))
    , m_versionLabel(new QLabel(this))
    , m_repositoryLabel(new QLabel(this))
    , m_maintainerLabel(new QLabel(this))
    , m_urlLabel(new QLabel(this))
    , m_descriptionText(new QTextEdit(this))
    , m_dependenciesText(new QTextEdit(this))
    , m_lastUpdatedLabel(new QLabel(this))
    , m_installButton(new QPushButton("Get", this))
    , m_uninstallButton(new QPushButton("Uninstall", this))
    , m_launchButton(new QPushButton("Launch", this))
    , m_closeButton(new QPushButton("Close", this))
    , m_statusBadge(new QLabel(this))
    , m_progressBar(new QProgressBar(this))
    , m_progressLabel(new QLabel(this))
    , m_progressWidget(new QWidget(this))
    , m_logViewer(new QTextEdit(this))
    , m_toggleLogButton(new QPushButton("Show Logs", this))
    , m_logWidget(new QWidget(this)) {
    
    if (embedded) {
        setWindowFlags(Qt::Widget);
    }
    setupUi();
    m_descriptionText->setCursor(Qt::IBeamCursor);
    loadAppStreamAsync();
    if (embedded && m_closeButton) {
        m_closeButton->hide();
    }
    checkInstallStatus();
    updateButtonStates();
    
    // Connect to PackageManager signals
    connect(&PackageManager::instance(), &PackageManager::operationStarted,
            this, &PackageDetailsDialog::onOperationStarted);
    connect(&PackageManager::instance(), &PackageManager::operationOutput,
            this, &PackageDetailsDialog::onOperationOutput);
    connect(&PackageManager::instance(), &PackageManager::operationCompleted,
            this, &PackageDetailsDialog::onOperationCompleted);
    connect(&PackageManager::instance(), &PackageManager::operationError,
            this, &PackageDetailsDialog::onOperationError);
}

static const char* s_detailCardStyle =
    "QFrame { background-color: #27272a; border: 1px solid #52525b; border-radius: 14px; padding: 14px; }"
    "QLabel { color: #f9fafb; }";

void PackageDetailsDialog::setupUi() {
    const QString accent = qApp->palette().color(QPalette::Highlight).name();
    const bool embedded = (windowFlags() & Qt::Widget);
    if (!embedded) {
        setWindowTitle("Package Details");
        setMinimumSize(700, 600);
        setModal(true);
        setWindowIcon(QIcon());
    } else {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        setMinimumHeight(200);
    }
    setStyleSheet("PackageDetailsDialog, QDialog { background-color: #1e2123; }");
    setAutoFillBackground(true);

    auto* dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->setSpacing(0);

    m_detailsScrollArea = new QScrollArea(this);
    m_detailsScrollArea->setWidgetResizable(true);
    m_detailsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_detailsScrollArea->setFrameShape(QFrame::NoFrame);
    m_detailsScrollArea->setStyleSheet("QScrollArea { background-color: #1e2123; border: none; }");

    m_detailsContentWidget = new QWidget();
    m_detailsContentWidget->setStyleSheet("QWidget { background-color: #1e2123; }");
    m_contentMainLayout = new QVBoxLayout(m_detailsContentWidget);
    const bool isEmbedded = (windowFlags() & Qt::Widget);
    m_contentMainLayout->setContentsMargins(isEmbedded ? 12 : 24, isEmbedded ? 12 : 20, isEmbedded ? 12 : 24, isEmbedded ? 12 : 24);
    m_contentMainLayout->setSpacing(24);
    auto* mainLayout = m_contentMainLayout;

    // —— Hero: icon stânga, nume + categorie dreapta, buton sub categorie ——
    auto* heroLayout = new QHBoxLayout();
    heroLayout->setAlignment(Qt::AlignTop);
    heroLayout->setSpacing(20);
    const int headerIconSize = 80;
    m_appstreamIconLabel = new QLabel(this);
    m_appstreamIconLabel->setFixedSize(headerIconSize, headerIconSize);
    m_appstreamIconLabel->setAlignment(Qt::AlignCenter);
    m_appstreamIconLabel->setStyleSheet("background-color: #1e2030; border-radius: 18px; border: 1px solid #2a2d3a;");
    m_appstreamIconLabel->setScaledContents(false);
    setPlaceholderIcon(headerIconSize);
    heroLayout->addWidget(m_appstreamIconLabel, 0);

    auto* rightCol = new QWidget(this);
    rightCol->setStyleSheet("background: transparent;");
    auto* rightColLayout = new QVBoxLayout(rightCol);
    rightColLayout->setContentsMargins(0, 0, 0, 0);
    rightColLayout->setSpacing(8);

    auto* nameRow = new QHBoxLayout();
    m_nameLabel->setText(m_info.name);
    auto nameFont = m_nameLabel->font();
    nameFont.setPointSize(22);
    nameFont.setBold(true);
    m_nameLabel->setFont(nameFont);
    m_nameLabel->setStyleSheet("color: #f9fafb;");
    nameRow->addWidget(m_nameLabel);
    nameRow->addStretch();
    rightColLayout->addLayout(nameRow);

    m_categoryLabel = new QLabel(m_info.repository.isEmpty() ? QLatin1String("—") : m_info.repository, this);
    m_categoryLabel->setWordWrap(true);
    m_categoryLabel->setStyleSheet("color: #a1a1aa; font-size: 14px; line-height: 1.4;");
    rightColLayout->addWidget(m_categoryLabel);

    auto* actionRow = new QHBoxLayout();
    actionRow->setAlignment(Qt::AlignLeft);
    actionRow->setSpacing(10);
    m_launchButton->setObjectName("discoverDownloadButton");
    m_launchButton->setFixedSize(58, 28);
    m_launchButton->setStyleSheet(
        "QPushButton#discoverDownloadButton { background-color: #ffffff; color: " + accent + "; border: none; border-radius: 14px; font-size: 11px; font-weight: 700; }"
        "QPushButton#discoverDownloadButton:hover:!disabled { background-color: #e5e7eb; color: " + accent + "; }"
        "QPushButton#discoverDownloadButton:disabled { background-color: #ffffff; color: #71717a; }");
    connect(m_launchButton, &QPushButton::clicked, this, &PackageDetailsDialog::onActionClicked);
    actionRow->addWidget(m_launchButton);
    rightColLayout->addLayout(actionRow);
    heroLayout->addWidget(rightCol, 1);
    mainLayout->addLayout(heroLayout);

    // Separator
    auto* line1 = new QFrame(this);
    line1->setFixedHeight(1);
    line1->setStyleSheet("background-color: #52525b;");
    mainLayout->addWidget(line1);

    // Placeholder for gallery (filled in onAppStreamDataReady)
    m_galleryContainer = new QWidget(this);
    m_galleryContainer->setStyleSheet("background: transparent;");
    auto* galleryPlaceholderLayout = new QVBoxLayout(m_galleryContainer);
    galleryPlaceholderLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_galleryContainer);

    // —— Detalii pachet în carduri ——
    auto* detailsTitle = new QLabel(tr("Package details"), this);
    detailsTitle->setStyleSheet("color: #f9fafb; font-size: 16px; font-weight: 600;");
    mainLayout->addWidget(detailsTitle);

    auto* cardsGrid = new QGridLayout();
    cardsGrid->setSpacing(12);
    int cardRow = 0, cardCol = 0;
    auto addCard = [this, &cardRow, &cardCol, cardsGrid](const QString& title, QWidget* valueWidget) {
        auto* card = new QFrame(this);
        card->setStyleSheet(s_detailCardStyle);
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(14, 14, 14, 14);
        cardLayout->setSpacing(4);
        auto* titleLabel = new QLabel(title, this);
        titleLabel->setStyleSheet("color: #71717a; font-size: 12px;");
        cardLayout->addWidget(titleLabel);
        valueWidget->setStyleSheet("color: #f9fafb; font-size: 14px;");
        cardLayout->addWidget(valueWidget);
        cardsGrid->addWidget(card, cardRow, cardCol);
        if (cardCol == 1) { cardCol = 0; cardRow++; } else cardCol++;
    };
    m_versionLabel->setText(m_info.version);
    addCard(tr("Version"), m_versionLabel);
    if (!m_info.maintainer.isEmpty()) {
        m_maintainerLabel->setText(m_info.maintainer);
        addCard(tr("Maintainer"), m_maintainerLabel);
    }
    if (!m_info.upstreamUrl.isEmpty()) {
        m_urlLabel->setText(QString("<a href='%1' style='color: #3b82f6; text-decoration: none;'>%1</a>").arg(m_info.upstreamUrl));
        m_urlLabel->setOpenExternalLinks(true);
        m_urlLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        m_urlLabel->setWordWrap(true);
        addCard(tr("Upstream URL"), m_urlLabel);
    }
    if (!m_info.lastUpdated.isNull()) {
        m_lastUpdatedLabel->setText(m_info.lastUpdated.toString("MMM d, yyyy"));
        addCard(tr("Last updated"), m_lastUpdatedLabel);
    }
    mainLayout->addLayout(cardsGrid);

    // Dependencies – listă expandabilă
    if (!m_info.dependList.isEmpty()) {
        m_depsExpandButton = new QPushButton(this);
        m_depsExpandButton->setText(QString(tr("Dependencies (%1) ▼")).arg(m_info.dependList.size()));
        m_depsExpandButton->setStyleSheet(
            "QPushButton { background: transparent; color: #a1a1aa; border: none; text-align: left; padding: 8px 0; font-size: 14px; }"
            "QPushButton:hover { color: #f9fafb; }");
        m_depsExpandButton->setCursor(Qt::ArrowCursor);
        connect(m_depsExpandButton, &QPushButton::clicked, this, &PackageDetailsDialog::onDependenciesToggled);
        mainLayout->addWidget(m_depsExpandButton);

        m_depsContent = new QFrame(this);
        m_depsContent->setStyleSheet("background-color: #27272a; border: 1px solid #52525b; border-radius: 14px; padding: 12px;");
        m_depsContent->hide();
        auto* depsLayout = new QVBoxLayout(m_depsContent);
        depsLayout->setContentsMargins(12, 12, 12, 12);
        m_dependenciesText->setPlainText(m_info.dependList.join("\n"));
        m_dependenciesText->setReadOnly(true);
        m_dependenciesText->setStyleSheet("background: transparent; border: none; color: #a1a1aa; font-size: 12px;");
        m_dependenciesText->setMaximumHeight(200);
        m_dependenciesText->setCursor(Qt::IBeamCursor);
        depsLayout->addWidget(m_dependenciesText);
        mainLayout->addWidget(m_depsContent);
    }

    // Command – card
    bool isAUR = (m_info.repository.toLower() == "aur");
    auto* commandCard = new QFrame(this);
    commandCard->setStyleSheet(s_detailCardStyle);
    auto* commandCardLayout = new QVBoxLayout(commandCard);
    commandCardLayout->setSpacing(10);
    auto* commandTitle = new QLabel(tr("Command"), this);
    commandTitle->setStyleSheet("color: #71717a; font-size: 12px;");
    commandCardLayout->addWidget(commandTitle);
    QLabel* commandText = nullptr;
    if (isAUR) {
        auto* buttonRow = new QHBoxLayout();
        auto* yayBtn = new QPushButton("yay", this);
        yayBtn->setCheckable(true);
        yayBtn->setChecked(true);
        yayBtn->setStyleSheet("QPushButton { background: #52525b; color: #fafafa; border: none; border-radius: 8px; padding: 6px 12px; } QPushButton:checked { background: " + accent + "; }");
        auto* paruBtn = new QPushButton("paru", this);
        paruBtn->setCheckable(true);
        paruBtn->setStyleSheet("QPushButton { background: #52525b; color: #fafafa; border: none; border-radius: 8px; padding: 6px 12px; } QPushButton:checked { background: " + accent + "; }");
        buttonRow->addWidget(yayBtn);
        buttonRow->addWidget(paruBtn);
        buttonRow->addStretch();
        commandCardLayout->addLayout(buttonRow);
        commandText = new QLabel(QString("yay -S %1").arg(m_info.name), this);
        connect(yayBtn, &QPushButton::clicked, [yayBtn, paruBtn, commandText, this]() {
            yayBtn->setChecked(true);
            paruBtn->setChecked(false);
            commandText->setText(QString("yay -S %1").arg(m_info.name));
        });
        connect(paruBtn, &QPushButton::clicked, [yayBtn, paruBtn, commandText, this]() {
            paruBtn->setChecked(true);
            yayBtn->setChecked(false);
            commandText->setText(QString("paru -S %1").arg(m_info.name));
        });
    } else {
        commandText = new QLabel(QString("sudo pacman -S %1").arg(m_info.name), this);
    }
    commandText->setStyleSheet("background-color: #1e2123; color: #fafafa; border-radius: 8px; padding: 10px 12px; font-family: monospace; font-size: 13px;");
    commandText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    commandCardLayout->addWidget(commandText);
    mainLayout->addWidget(commandCard);

    auto* noteLabel = new QLabel(tr("Ensure your system meets the requirements before installation."), m_detailsContentWidget);
    noteLabel->setStyleSheet("color: #71717a; font-size: 11px;");
    noteLabel->setWordWrap(true);
    mainLayout->addWidget(noteLabel);
    mainLayout->addStretch();

    m_detailsScrollArea->setWidget(m_detailsContentWidget);
    dialogLayout->addWidget(m_detailsScrollArea, 1);

    // Buttons at bottom
    auto* buttonWidget = new QWidget(this);
    buttonWidget->setStyleSheet("background-color: #1e2123;");
    auto* buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(24, 12, 24, 24);
    buttonLayout->addStretch();
    m_closeButton->setMinimumWidth(100);
    m_closeButton->setMinimumHeight(36);
    m_closeButton->setStyleSheet(
        "QPushButton { background-color: #27272a; color: #fafafa; border: none; border-radius: 14px; }"
        "QPushButton:hover { background-color: #3f3f46; }");
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(m_closeButton);
    dialogLayout->addWidget(buttonWidget, 0);
    
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
        "    background-color: " + accent + ";"
        "}"
    );
    progressLayout->addWidget(m_progressBar);
    
    // Toggle log button
    m_toggleLogButton->setStyleSheet(
        "QPushButton {"
        "    background: none;"
        "    border: none;"
        "    color: " + accent + ";"
        "    text-decoration: underline;"
        "    font-size: 11px;"
        "    padding: 4px;"
        "}"
        "QPushButton:hover {"
        "    color: #60a5fa;"
        "}"
    );
    connect(m_toggleLogButton, &QPushButton::clicked, this, &PackageDetailsDialog::toggleLogViewer);
    progressLayout->addWidget(m_toggleLogButton, 0, Qt::AlignCenter);
    
    m_progressWidget->setStyleSheet("QWidget { background-color: #1e2123; }");
    m_progressWidget->hide();
    dialogLayout->addWidget(m_progressWidget, 0);
    
    // Log viewer section (hidden by default)
    auto* logLayout = new QVBoxLayout(m_logWidget);
    logLayout->setContentsMargins(20, 0, 20, 20);
    logLayout->setSpacing(8);
    
    m_logViewer->setReadOnly(true);
    m_logViewer->setMaximumHeight(200);
    m_logViewer->setStyleSheet(
        "QTextEdit {"
        "    background-color: #1e2123;"
        "    border: 1px solid #27272a;"
        "    border-radius: 6px;"
        "    padding: 8px;"
        "    font-family: monospace;"
        "    font-size: 11px;"
        "    color: #a1a1aa;"
        "}"
    );
    m_logViewer->setCursor(Qt::IBeamCursor);
    logLayout->addWidget(m_logViewer);
    
    m_logWidget->setStyleSheet("QWidget { background-color: #1e2123; }");
    m_logWidget->hide();
    dialogLayout->addWidget(m_logWidget, 0);
    
    setLayout(dialogLayout);
}

void PackageDetailsDialog::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    if (windowFlags() & Qt::Widget) {
        updateDetailsContentMinHeight();
    }
}

void PackageDetailsDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QTimer::singleShot(0, this, [this]() { updateDetailsContentMinHeight(); });
}

void PackageDetailsDialog::updateDetailsContentMinHeight() {
    if (!(windowFlags() & Qt::Widget) || !m_detailsScrollArea || !m_detailsContentWidget) return;
    QWidget* vp = m_detailsScrollArea->viewport();
    if (!vp) return;
    int h = vp->height();
    if (h > 0 && m_detailsContentWidget->minimumHeight() != h) {
        m_detailsContentWidget->setMinimumHeight(h);
    }
}

void PackageDetailsDialog::onDependenciesToggled() {
    if (!m_depsContent || !m_depsExpandButton) return;
    const bool show = !m_depsContent->isVisible();
    m_depsContent->setVisible(show);
    m_depsExpandButton->setText(show
        ? QString(tr("Dependencies (%1) ▲")).arg(m_info.dependList.size())
        : QString(tr("Dependencies (%1) ▼")).arg(m_info.dependList.size()));
}

void PackageDetailsDialog::checkInstallStatus() {
    m_isInstalled = AlpmWrapper::instance().isPackageInstalled(m_info.name);
    // Update the installed badge in the header
    if (m_isInstalled) {
        m_statusBadge->show();
    } else {
        m_statusBadge->hide();
    }
}

void PackageDetailsDialog::updateButtonStates() {
    if (!m_launchButton)
        return;

    // Dacă instalarea este în curs, afișăm stare de „busy” în loc de Get/Open
    if (m_isInstalling) {
        m_launchButton->setText(tr("Installing..."));
        m_launchButton->setEnabled(false);
        return;
    }

    QString desktopFile = findDesktopFile();
    if (m_isInstalled) {
        m_launchButton->setText(tr("Open"));
        m_launchButton->setEnabled(!desktopFile.isEmpty());
    } else {
        m_launchButton->setText(tr("Get"));
        m_launchButton->setEnabled(true);
    }
}

void PackageDetailsDialog::onActionClicked() {
    if (m_isInstalled) {
        launchApplication();
    } else {
        onInstall();
    }
}

void PackageDetailsDialog::onInstall() {
    Logger::info(QString("Installing package: %1").arg(m_info.name));

    // Marchează dialogul ca fiind în curs de instalare și actualizează butonul (spinner/text)
    m_isInstalling = true;
    updateButtonStates();

    if (m_closeButton)
        m_closeButton->setEnabled(false);

    PackageManager::instance().installPackage(m_info.name, m_info.repository);
}

void PackageDetailsDialog::onUninstall() {
    auto reply = QMessageBox::question(this, "Uninstall Package",
        QString("Are you sure you want to uninstall %1?\n\n"
                "This will remove the package and skip dependency checks.")
        .arg(m_info.name),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        Logger::info(QString("Uninstalling package: %1").arg(m_info.name));
        m_launchButton->setEnabled(false);
        m_closeButton->setEnabled(false);
        PackageManager::instance().uninstallPackage(m_info.name, m_info.repository);
    }
}

void PackageDetailsDialog::showProgress(const QString& message) {
    m_progressLabel->setText(message);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressWidget->show();
    m_progressBar->show();
    m_progressLabel->show();
    m_logViewer->clear();
    m_currentOperation = message;
    m_totalPackages = 0;
    m_currentPackage = 0;
}

void PackageDetailsDialog::hideProgress() {
    // Hide the progress bar and label, but keep the widget and toggle button visible
    m_progressBar->hide();
    m_progressLabel->hide();
    // Don't hide m_progressWidget - keeps the toggle button visible
    // Don't hide the log widget or reset log visibility
    // This allows users to review logs after operation completes
}

void PackageDetailsDialog::toggleLogViewer() {
    m_logVisible = !m_logVisible;
    if (m_logVisible) {
        m_logWidget->show();
        m_toggleLogButton->setText("Hide Logs");
    } else {
        m_logWidget->hide();
        m_toggleLogButton->setText("Show Logs");
    }
}

void PackageDetailsDialog::parseProgressOutput(const QString& output) {
    // Parse pacman/yay/paru output for progress information
    
    // Pattern: "downloading..." or "installing..."
    if (output.contains("downloading", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Downloading packages...");
    } else if (output.contains("installing", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Installing packages...");
    } else if (output.contains("building", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Building packages...");
    } else if (output.contains("checking", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Checking dependencies...");
    } else if (output.contains("resolving", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Resolving dependencies...");
    }
    
    // Pattern: "(1/5)" or "( 1/5)" to track package progress
    QRegularExpression packagePattern(R"(\(\s*(\d+)/(\d+)\))");
    auto match = packagePattern.match(output);
    if (match.hasMatch()) {
        m_currentPackage = match.captured(1).toInt();
        m_totalPackages = match.captured(2).toInt();
        
        if (m_totalPackages > 0) {
            int percentage = (m_currentPackage * 100) / m_totalPackages;
            m_progressBar->setValue(percentage);
        }
    }
    
    // Pattern: "[##########] 100%" for download progress
    QRegularExpression percentPattern(R"(\s+(\d+)%\s*)");
    auto percentMatch = percentPattern.match(output);
    if (percentMatch.hasMatch()) {
        int percentage = percentMatch.captured(1).toInt();
        m_progressBar->setValue(percentage);
    }
}

void PackageDetailsDialog::onOperationStarted(const QString& message) {
    showProgress(message);
}

void PackageDetailsDialog::onOperationOutput(const QString& output) {
    if (output.trimmed().isEmpty()) {
        return;
    }
    
    // Add to log viewer
    m_logViewer->append(output.trimmed());
    
    // Auto-scroll to bottom
    QTextCursor cursor = m_logViewer->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logViewer->setTextCursor(cursor);
    
    // Parse output for progress information
    parseProgressOutput(output);
    
    // Force UI update
    m_progressLabel->repaint();
    m_progressBar->repaint();
    QCoreApplication::processEvents();
}

void PackageDetailsDialog::onOperationCompleted(bool success, const QString& message) {
    // Refresh ALPM state so subsequent queries reflect the change
    AlpmWrapper::instance().release();
    AlpmWrapper::instance().initialize();

    // Update status and UI
    checkInstallStatus();
    m_isInstalling = false;
    updateButtonStates();

    hideProgress();

    // Re-enable close button
    if (m_closeButton)
        m_closeButton->setEnabled(true);
}

void PackageDetailsDialog::onOperationError(const QString& error) {
    // Refresh ALPM state (best-effort)
    AlpmWrapper::instance().release();
    AlpmWrapper::instance().initialize();

    // Update status and UI
    checkInstallStatus();
    m_isInstalling = false;
    updateButtonStates();

    hideProgress();

    // Re-enable close button
    if (m_closeButton)
        m_closeButton->setEnabled(true);
}

QString PackageDetailsDialog::findDesktopFile() const {
    if (!m_isInstalled) {
        return QString();
    }
    
    // Common locations for .desktop files
    QStringList desktopDirs = {
        "/usr/share/applications",
        "/usr/local/share/applications",
        QDir::homePath() + "/.local/share/applications"
    };
    
    // Try to find desktop file matching the package name
    // Common patterns: package.desktop, package-*.desktop
    QStringList patterns = {
        m_info.name + ".desktop",
        m_info.name + "-*.desktop"
    };
    
    for (const QString& dir : desktopDirs) {
        QDir desktopDir(dir);
        if (!desktopDir.exists()) {
            continue;
        }
        
        // First try exact patterns
        for (const QString& pattern : patterns) {
            QStringList matches = desktopDir.entryList(QStringList() << pattern, QDir::Files);
            if (!matches.isEmpty()) {
                QString desktopFile = desktopDir.absoluteFilePath(matches.first());
                Logger::info(QString("Found desktop file for %1: %2").arg(m_info.name, desktopFile));
                return desktopFile;
            }
        }
        
        // If not found, try fuzzy matching with all .desktop files
        QStringList allDesktopFiles = desktopDir.entryList(QStringList() << "*.desktop", QDir::Files);
        
        // Create regex patterns for fuzzy matching
        // Handle reverse domain names: com.obsproject.Studio.desktop -> obs-studio
        // Handle simple names: code.desktop -> visual-studio-code-bin
        QString packageNameLower = m_info.name.toLower();
        QStringList nameVariants;
        
        // Add the full package name
        nameVariants << packageNameLower;
        
        // Extract keywords from package name (split by dash and underscore)
        QStringList parts = packageNameLower.split(QRegularExpression("[-_]"));
        QStringList significantParts;
        for (const QString& part : parts) {
            if (part.length() > 3) { // Skip very short parts to avoid false matches
                significantParts << part;
            }
        }
        
        // Special handling for common patterns
        QStringList specialVariants;
        if (packageNameLower.contains("visual-studio-code")) {
            specialVariants << "vscode" << "code";
        } else if (packageNameLower == "obs-studio") {
            // For obs-studio, look for obsproject specifically
            specialVariants << "obsproject";
        }
        
        // Try special variants first (highest priority)
        for (const QString& variant : specialVariants) {
            for (const QString& desktopFileName : allDesktopFiles) {
                QString fileNameLower = desktopFileName.toLower();
                
                if (fileNameLower.contains(variant)) {
                    QString desktopFile = desktopDir.absoluteFilePath(desktopFileName);
                    
                    if (verifyDesktopFile(desktopFile, nameVariants + specialVariants + significantParts)) {
                        Logger::info(QString("Found desktop file for %1 via special match: %2")
                                    .arg(m_info.name, desktopFile));
                        return desktopFile;
                    }
                }
            }
        }
        
        // Try full package name match
        for (const QString& desktopFileName : allDesktopFiles) {
            QString fileNameLower = desktopFileName.toLower();
            
            if (fileNameLower.contains(packageNameLower)) {
                QString desktopFile = desktopDir.absoluteFilePath(desktopFileName);
                
                if (verifyDesktopFile(desktopFile, nameVariants + specialVariants + significantParts)) {
                    Logger::info(QString("Found desktop file for %1 via full name match: %2")
                                .arg(m_info.name, desktopFile));
                    return desktopFile;
                }
            }
        }
        
        // Finally, try matching individual significant parts (but verify carefully)
        for (const QString& part : significantParts) {
            for (const QString& desktopFileName : allDesktopFiles) {
                QString fileNameLower = desktopFileName.toLower();
                
                // Use word boundary-like matching: ensure part is not in the middle of another word
                // Check if part appears as a separate component (after . or at start, before . or -)
                QRegularExpression wordBoundary(QString("(^|[._-])%1([._-]|$)").arg(QRegularExpression::escape(part)));
                
                if (wordBoundary.match(fileNameLower).hasMatch()) {
                    QString desktopFile = desktopDir.absoluteFilePath(desktopFileName);
                    
                    if (verifyDesktopFile(desktopFile, nameVariants + specialVariants + significantParts)) {
                        Logger::info(QString("Found desktop file for %1 via word match: %2")
                                    .arg(m_info.name, desktopFile));
                        return desktopFile;
                    }
                }
            }
        }
    }
    
    Logger::debug(QString("No desktop file found for package: %1").arg(m_info.name));
    return QString();
}

bool PackageDetailsDialog::verifyDesktopFile(const QString& desktopFilePath,
                                             const QStringList& nameVariants) const {
    QFile file(desktopFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // Check if Exec line contains any of our name variants
    QRegularExpression execPattern(R"(^Exec=(.*)$)", QRegularExpression::MultilineOption);
    auto match = execPattern.match(content);
    
    if (match.hasMatch()) {
        QString execLine = match.captured(1).toLower();
        
        // Check if any variant appears in the Exec line
        for (const QString& variant : nameVariants) {
            if (execLine.contains(variant)) {
                return true;
            }
        }
    }
    
    // Also check Name field as a fallback
    QRegularExpression namePattern(R"(^Name=(.*)$)", QRegularExpression::MultilineOption);
    match = namePattern.match(content);
    
    if (match.hasMatch()) {
        QString nameField = match.captured(1).toLower();
        
        for (const QString& variant : nameVariants) {
            if (nameField.contains(variant)) {
                return true;
            }
        }
    }
    
    return false;
}

void PackageDetailsDialog::onScreenshotLoaded(QNetworkReply* reply, QLabel* label, const QString& cachedPath) {
    if (!reply || !label) {
        if (reply) reply->deleteLater();
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }
    QByteArray data = reply->readAll();
    reply->deleteLater();
    if (!cachedPath.isEmpty()) {
        CacheHelper::ensureScreenshotsDir();
        QFile f(cachedPath);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(data);
        }
    }
    QPixmap px;
    if (px.loadFromData(data) && label->width() > 0 && label->height() > 0) {
        label->setPixmap(px.scaled(label->width(), label->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void PackageDetailsDialog::loadAppStreamAsync() {
    m_appstreamWatcher = new QFutureWatcher<AppStreamData>(this);
    connect(m_appstreamWatcher, &QFutureWatcher<AppStreamData>::finished, this, &PackageDetailsDialog::onAppStreamDataReady);
    QString name = m_info.name;
    m_appstreamWatcher->setFuture(QtConcurrent::run([name]() {
        return AppStreamHelper::getDataForPackage(name);
    }));
}

void PackageDetailsDialog::setPlaceholderIcon(int size) {
    if (!m_appstreamIconLabel) return;
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
    m_appstreamIconLabel->setPixmap(px);
}

QString PackageDetailsDialog::iconPathFromDesktopFile(const QString& desktopPath) const {
    QSettings ini(desktopPath, QSettings::IniFormat);
    QString icon = ini.value(QLatin1String("Desktop Entry/Icon")).toString().trimmed();
    if (icon.isEmpty()) return QString();
    if (icon.startsWith(QLatin1Char('/')) && QFile::exists(icon)) return icon;
    return QString();
}

void PackageDetailsDialog::onAppStreamDataReady() {
    if (!m_appstreamWatcher) return;
    m_appstream = m_appstreamWatcher->result();
    m_appstreamWatcher->deleteLater();
    m_appstreamWatcher = nullptr;

    const int headerIconSize = 80;
    bool iconSet = false;
    if (!m_appstream.iconPath.isEmpty() && m_appstreamIconLabel) {
        QPixmap px(m_appstream.iconPath);
        if (!px.isNull()) {
            m_appstreamIconLabel->setPixmap(px.scaled(headerIconSize, headerIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            iconSet = true;
        }
    }
    if (!iconSet && m_appstreamIconLabel) {
        QString path;
        QDir iconsDir(CacheHelper::iconsDir());
        const QStringList exts = {QStringLiteral(".png"), QStringLiteral(".svg")};
        for (const QString& ext : exts) {
            path = iconsDir.absoluteFilePath(m_info.name + ext);
            if (QFile::exists(path)) break;
            path = QString();
        }
        if (path.isEmpty() && m_isInstalled) {
            QString desktopFile = findDesktopFile();
            if (!desktopFile.isEmpty()) {
                QString iconName = iconPathFromDesktopFile(desktopFile);
                if (!iconName.isEmpty()) path = iconName;
            }
        }
        if (!path.isEmpty()) {
            QPixmap px(path);
            if (!px.isNull()) {
                m_appstreamIconLabel->setPixmap(px.scaled(headerIconSize, headerIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                iconSet = true;
            }
        }
        if (!iconSet && m_isInstalled) {
            QString desktopFile = findDesktopFile();
            if (!desktopFile.isEmpty()) {
                QSettings ini(desktopFile, QSettings::IniFormat);
                QString iconName = ini.value(QLatin1String("Desktop Entry/Icon")).toString().trimmed();
                if (!iconName.isEmpty()) {
                    QIcon themeIcon = QIcon::fromTheme(iconName);
                    if (!themeIcon.isNull()) {
                        QPixmap px = themeIcon.pixmap(headerIconSize, headerIconSize);
                        if (!px.isNull()) {
                            m_appstreamIconLabel->setPixmap(px.scaled(headerIconSize, headerIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                            iconSet = true;
                        }
                    }
                }
            }
        }
    }
    if (m_categoryLabel) {
        if (!m_appstream.categories.isEmpty()) {
            m_categoryLabel->setText(m_appstream.categories.join(QLatin1String(" • ")));
        } else if (!m_info.repository.isEmpty()) {
            m_categoryLabel->setText(m_info.repository);
        } else {
            m_categoryLabel->setText(QLatin1String("—"));
        }
    }
    if (m_descriptionLabel) {
        QString descText = m_info.description;
        if (!m_appstream.summary.isEmpty()) descText = m_appstream.summary;
        if (!m_appstream.description.isEmpty()) descText = m_appstream.description;
        m_descriptionLabel->setText(descText);
    }
    if (!m_appstream.screenshotUrls.isEmpty() && m_galleryContainer) {
        QLayout* galleryLayout = m_galleryContainer->layout();
        if (galleryLayout) {
            auto* title = new QLabel(tr("Screenshots"), this);
            title->setStyleSheet("color: #f9fafb; font-size: 16px; font-weight: 600;");
            galleryLayout->addWidget(title);
            auto* screenshotsScroll = new QScrollArea(this);
            screenshotsScroll->setWidgetResizable(true);
            screenshotsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            screenshotsScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            screenshotsScroll->setFrameShape(QFrame::NoFrame);
            screenshotsScroll->setFixedHeight(200);
            screenshotsScroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");
            m_screenshotsWidget = new QWidget(this);
            auto* screenshotsLayout = new QHBoxLayout(m_screenshotsWidget);
            screenshotsLayout->setSpacing(12);
            screenshotsLayout->setContentsMargins(0, 8, 0, 0);
            const int thumbHeight = 180;
            const int thumbWidth = 320;
            CacheHelper::ensureScreenshotsDir();
            for (const QString& urlStr : m_appstream.screenshotUrls) {
                QLabel* thumb = new QLabel(m_screenshotsWidget);
                thumb->setFixedSize(thumbWidth, thumbHeight);
                thumb->setAlignment(Qt::AlignCenter);
                thumb->setStyleSheet("background-color: #27272a; border-radius: 14px; border: 1px solid #52525b;");
                thumb->setScaledContents(false);
                QUrl url(urlStr);
                if (url.isLocalFile() || urlStr.startsWith(QLatin1Char('/'))) {
                    QString path = url.isLocalFile() ? url.toLocalFile() : urlStr;
                    QString cachedPath = CacheHelper::pathForUrl(QLatin1String("file://") + path);
                    if (!QFile::exists(cachedPath)) {
                        CacheHelper::ensureScreenshotsDir();
                        QFile::copy(path, cachedPath);
                    }
                    QPixmap px(path);
                    if (!px.isNull()) {
                        thumb->setPixmap(px.scaled(thumbWidth, thumbHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    }
                } else {
                    QString cachedPath = CacheHelper::pathForUrl(urlStr);
                    if (QFile::exists(cachedPath)) {
                        QPixmap px(cachedPath);
                        if (!px.isNull()) {
                            thumb->setPixmap(px.scaled(thumbWidth, thumbHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                        }
                    } else {
                        QNetworkRequest req{QUrl(urlStr)};
                        QNetworkReply* reply = m_network->get(req);
                        connect(reply, &QNetworkReply::finished, this, [this, reply, thumb, cachedPath]() {
                            onScreenshotLoaded(reply, thumb, cachedPath);
                        });
                    }
                }
                screenshotsLayout->addWidget(thumb);
            }
            screenshotsScroll->setWidget(m_screenshotsWidget);
            galleryLayout->addWidget(screenshotsScroll);
        }
    }
}

void PackageDetailsDialog::launchApplication() {
    QString desktopFile = findDesktopFile();
    
    if (desktopFile.isEmpty()) {
        QMessageBox::warning(this, "Launch Failed",
            QString("Could not find a desktop file for %1.\n"
                    "This application may not have a graphical interface or "
                    "may need to be launched from the terminal.").arg(m_info.name));
        return;
    }
    
    // Launch the application using gtk-launch or similar
    QProcess* process = new QProcess(this);
    
    // Try gtk-launch first (works on most desktop environments)
    QString baseName = QFileInfo(desktopFile).fileName();
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process, baseName](int exitCode, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        
        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            Logger::error(QString("Failed to launch application: %1").arg(baseName));
            QMessageBox::warning(this, "Launch Failed",
                QString("Failed to launch %1.\n"
                        "Exit code: %2").arg(m_info.name).arg(exitCode));
        } else {
            Logger::info(QString("Successfully launched: %1").arg(baseName));
        }
    });
    
    // Try gtk-launch first
    process->start("gtk-launch", QStringList() << baseName);
    
    // If gtk-launch doesn't start, try alternative methods
    if (!process->waitForStarted(1000)) {
        // Try dex (Desktop Entry Execution)
        process->start("dex", QStringList() << desktopFile);
        
        if (!process->waitForStarted(1000)) {
            // Try exo-open (XFCE)
            process->start("exo-open", QStringList() << desktopFile);
            
            if (!process->waitForStarted(1000)) {
                // Last resort: try to parse and execute the Exec line
                process->deleteLater();
                QMessageBox::warning(this, "Launch Failed",
                    "Could not find a suitable desktop file launcher.\n"
                    "Please install gtk-launch, dex, or exo-open.");
                Logger::error("No desktop file launcher available");
            }
        }
    }
}
