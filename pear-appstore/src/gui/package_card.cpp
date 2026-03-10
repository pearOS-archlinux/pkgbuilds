#include "package_card.h"
#include "../core/alpm_wrapper.h"
#include "../core/appstream_helper.h"
#include "../utils/cache_helper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QStyle>
#include <QIcon>
#include <QPainter>
#include <QtConcurrent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QApplication>
#include <QPointer>

PackageCard::PackageCard(const PackageInfo& info, QWidget* parent, DisplayStyle style)
    : QWidget(parent)
    , m_info(info)
    , m_displayStyle(style)
    , m_downloadButton(new QPushButton(this))
    , m_iconLabel(new QLabel(this))
    , m_nameLabel(new QLabel(this))
    , m_descriptionLabel(new QLabel(this))
    , m_versionLabel(new QLabel(this))
    , m_repositoryLabel(new QLabel(this))
    , m_statusLabel(new QLabel(this)) {
    setupUi();
    checkInstallStatus();
}

void PackageCard::setupUi() {
    QString desc = m_info.description;
    setProperty("class", "package-card");
    setCursor(Qt::ArrowCursor);

    if (m_displayStyle == Discover || m_displayStyle == SearchResults) {
        setMinimumHeight(88);
        setMinimumWidth(280);
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 12, 16, 0);
        mainLayout->setSpacing(0);

        if (m_displayStyle == SearchResults) {
            m_screenshotLabel = new QLabel(this);
            m_screenshotLabel->setFixedHeight(120);
            m_screenshotLabel->setMinimumWidth(248);
            m_screenshotLabel->setAlignment(Qt::AlignCenter);
            m_screenshotLabel->setStyleSheet("background-color: #27272a; border-radius: 12px 12px 0 0;");
            m_screenshotLabel->setScaledContents(false);
            m_screenshotLabel->hide();
            mainLayout->addWidget(m_screenshotLabel, 0);
        }

        QFrame* separator = new QFrame(this);
        separator->setObjectName("discoverCardSeparator");
        separator->setFixedHeight(1);
        separator->setStyleSheet("QFrame#discoverCardSeparator { background-color: rgba(255,255,255,0.08); border: none; }");
        mainLayout->addWidget(separator, 0);

        const int iconSize = 56;
        m_iconLabel->setFixedSize(iconSize, iconSize);
        m_iconLabel->setAlignment(Qt::AlignCenter);
        m_iconLabel->setStyleSheet(
            "background-color: #27272a; border-radius: 12px; border: none;"
        );
        setPlaceholderIcon(m_iconLabel, iconSize);

        m_nameLabel->setText(m_info.name);
        m_nameLabel->setWordWrap(false);
        auto nameFont = m_nameLabel->font();
        nameFont.setPointSize(15);
        nameFont.setBold(true);
        m_nameLabel->setFont(nameFont);
        m_nameLabel->setStyleSheet("color: #f9fafb;");

        m_descriptionLabel->setText(desc);
        m_descriptionLabel->setWordWrap(true);
        m_descriptionLabel->setStyleSheet("color: #a1a1aa; font-size: 12px;");
        m_descriptionLabel->setMaximumHeight(36);

        auto* textColumn = new QVBoxLayout();
        textColumn->setContentsMargins(0, 0, 0, 0);
        textColumn->setSpacing(2);
        textColumn->addWidget(m_nameLabel, 0);
        textColumn->addWidget(m_descriptionLabel, 0);

        m_downloadButton->setText(m_isInstalled ? tr("Installed") : tr("Get"));
        m_downloadButton->setCursor(Qt::ArrowCursor);
        m_downloadButton->setObjectName("discoverDownloadButton");
        m_downloadButton->setFixedSize(58, 28);
        connect(m_downloadButton, &QPushButton::clicked, this, [this]() { emit clicked(m_info); });

        auto* rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(14);
        rowLayout->addWidget(m_iconLabel, 0);
        rowLayout->addLayout(textColumn, 1);
        rowLayout->addWidget(m_downloadButton, 0, Qt::AlignVCenter);
        mainLayout->addLayout(rowLayout, 0);
        mainLayout->setContentsMargins(16, 12, 16, 12);

        m_versionLabel->hide();
        m_repositoryLabel->hide();
        m_statusLabel->hide();
        setLayout(mainLayout);
        return;
    }

    // Compact style (Search, Installed)
    setMinimumHeight(160);
    setMinimumWidth(200);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    const int iconSize = 48;
    m_iconLabel->setFixedSize(iconSize, iconSize);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("background-color: transparent; border: none;");
    setPlaceholderIcon(m_iconLabel, iconSize);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->addWidget(m_iconLabel, 0);
    m_nameLabel->setText(m_info.name);
    m_nameLabel->setWordWrap(false);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto nameFont = m_nameLabel->font();
    nameFont.setPointSize(14);
    nameFont.setBold(true);
    m_nameLabel->setFont(nameFont);
    headerLayout->addWidget(m_nameLabel, 1);
    m_statusLabel->setProperty("class", "status-badge");
    m_statusLabel->hide();
    headerLayout->addWidget(m_statusLabel, 0);
    mainLayout->addLayout(headerLayout);

    m_descriptionLabel->setText(desc);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_descriptionLabel->setMinimumHeight(40);
    m_descriptionLabel->setMaximumHeight(70);
    mainLayout->addWidget(m_descriptionLabel, 1);

    m_versionLabel->setText(QString("Version: %1").arg(m_info.version));
    auto versionFont = m_versionLabel->font();
    versionFont.setPointSize(9);
    m_versionLabel->setFont(versionFont);
    m_versionLabel->setStyleSheet("color: #888;");
    mainLayout->addWidget(m_versionLabel, 0);

    m_repositoryLabel->setText(m_info.repository);
    m_repositoryLabel->setProperty("class", "repo-badge");
    m_repositoryLabel->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(m_repositoryLabel, 0);

    setLayout(mainLayout);
}

void PackageCard::loadAppStreamAsync() {
    if (m_appstreamWatcher) return;
    m_appstreamWatcher = new QFutureWatcher<AppStreamData>(this);
    connect(m_appstreamWatcher, &QFutureWatcher<AppStreamData>::finished, this, &PackageCard::onAppStreamDataReady);
    QString name = m_info.name;
    m_appstreamWatcher->setFuture(QtConcurrent::run([name]() {
        return AppStreamHelper::getDataForPackage(name);
    }));
}

void PackageCard::onAppStreamDataReady() {
    if (!m_appstreamWatcher) return;
    AppStreamData data = m_appstreamWatcher->result();
    m_appstreamWatcher->deleteLater();
    m_appstreamWatcher = nullptr;
    applyAppStreamData(data);
}

void PackageCard::applyAppStreamData(const AppStreamData& data) {
    const int iconSize = 56;
    QPixmap iconPx;

    if (!data.iconPath.isEmpty()) {
        iconPx.load(data.iconPath);
    }
    if (iconPx.isNull() && !data.iconStockName.isEmpty()) {
        QIcon themeIcon = QIcon::fromTheme(data.iconStockName);
        if (!themeIcon.isNull()) {
            iconPx = themeIcon.pixmap(iconSize, iconSize);
        }
    }
    if (iconPx.isNull()) {
        QStringList themeCandidates;
        themeCandidates << m_info.name << m_info.name.toLower()
                       << m_info.name.replace(QLatin1Char('-'), QLatin1Char('_'))
                       << m_info.name.toLower().replace(QLatin1Char('-'), QLatin1Char('_'));
        for (const QString& key : themeCandidates) {
            if (key.isEmpty()) continue;
            QIcon themeIcon = QIcon::fromTheme(key);
            if (!themeIcon.isNull()) {
                iconPx = themeIcon.pixmap(iconSize, iconSize);
                if (!iconPx.isNull()) break;
            }
        }
    }
    if (!iconPx.isNull()) {
        m_iconLabel->setPixmap(iconPx.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (!data.summary.isEmpty() || !data.description.isEmpty()) {
        QString desc = data.summary.isEmpty() ? data.description : data.summary;
        if (!desc.isEmpty()) {
            m_descriptionLabel->setText(desc);
        }
    }

    if (m_displayStyle == SearchResults && m_screenshotLabel && !data.screenshotUrls.isEmpty()) {
        const QString urlStr = data.screenshotUrls.first();
        const int thumbW = 248;
        const int thumbH = 120;
        QUrl url(urlStr);
        if (url.isLocalFile() || urlStr.startsWith(QLatin1Char('/'))) {
            QString path = url.isLocalFile() ? url.toLocalFile() : urlStr;
            QPixmap px(path);
            if (!px.isNull()) {
                m_screenshotLabel->setPixmap(px.scaled(thumbW, thumbH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_screenshotLabel->show();
            }
        } else {
            static QNetworkAccessManager* s_network = nullptr;
            if (!s_network) s_network = new QNetworkAccessManager(qApp);
            QString cachedPath = CacheHelper::pathForUrl(urlStr);
            if (QFile::exists(cachedPath)) {
                QPixmap px(cachedPath);
                if (!px.isNull()) {
                    m_screenshotLabel->setPixmap(px.scaled(thumbW, thumbH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    m_screenshotLabel->show();
                }
            } else {
                QNetworkRequest req{QUrl(urlStr)};
                QNetworkReply* reply = s_network->get(req);
                QPointer<QLabel> label(m_screenshotLabel);
                connect(reply, &QNetworkReply::finished, this, [reply, label, thumbW, thumbH, cachedPath]() {
                    if (reply->error() != QNetworkReply::NoError) {
                        reply->deleteLater();
                        return;
                    }
                    QPixmap px;
                    px.loadFromData(reply->readAll());
                    reply->deleteLater();
                    if (!px.isNull()) {
                        if (!cachedPath.isEmpty()) {
                            CacheHelper::ensureScreenshotsDir();
                            px.save(cachedPath);
                        }
                        if (label) {
                            label->setPixmap(px.scaled(thumbW, thumbH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                            label->show();
                        }
                    }
                });
            }
        }
    }
}

void PackageCard::checkInstallStatus() {
    m_isInstalled = AlpmWrapper::instance().isPackageInstalled(m_info.name);
    updateInstallStatus(m_isInstalled);
}

void PackageCard::updateInstallStatus(bool installed) {
    m_isInstalled = installed;

    if (m_downloadButton) {
        m_downloadButton->setText(installed ? tr("Installed") : tr("Get"));
        m_downloadButton->setEnabled(!installed);
    }
    if (m_statusLabel && m_displayStyle == Compact) {
        if (m_isInstalled) {
            m_statusLabel->setText("Installed");
            m_statusLabel->show();
        } else {
            m_statusLabel->hide();
        }
    }
}

void PackageCard::setPlaceholderIcon(QLabel* label, int size) {
    QPixmap px(size, size);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    QColor bg("#52525b");
    p.setBrush(bg);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(0, 0, size, size, size / 4, size / 4);
    QString letter = m_info.name.isEmpty() ? QLatin1String("?") : m_info.name.left(1).toUpper();
    p.setPen(QColor("#a1a1aa"));
    QFont f = p.font();
    f.setPointSize(size / 2);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, letter);
    p.end();
    label->setPixmap(px);
}

void PackageCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_info);
    }
    QWidget::mousePressEvent(event);
}

void PackageCard::enterEvent(QEnterEvent* event) {
    setProperty("hovered", true);
    style()->unpolish(this);
    style()->polish(this);
    QWidget::enterEvent(event);
}

void PackageCard::leaveEvent(QEvent* event) {
    setProperty("hovered", false);
    style()->unpolish(this);
    style()->polish(this);
    QWidget::leaveEvent(event);
}
