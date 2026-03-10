#include "home_widget.h"
#include "package_card.h"
#include "loading_overlay.h"
#include "scroll_bar_fade.h"
#include "../core/alpm_wrapper.h"
#include "../core/aur_helper.h"
#include "../utils/logger.h"
#include "../utils/app_cache.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QEventLoop>
#include <QResizeEvent>
#include <QSet>
#include <QTimer>
#include <QtConcurrent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>
#include <QPainterPath>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QProcess>

HomeWidget::HomeWidget(QWidget* parent)
    : QWidget(parent)
    , m_scrollArea(new QScrollArea(this))
    , m_contentWidget(new QWidget())
    , m_gridLayout(new QGridLayout())
    , m_updateTimer(new QTimer(this)) {
    
    setupUi();
    QTimer::singleShot(0, this, &HomeWidget::onInitialLoad);
    
    connect(m_updateTimer, &QTimer::timeout, this, &HomeWidget::onUpdateTimer);
    m_updateTimer->start(30000);
}

void HomeWidget::onInitialLoad() {
    QVector<PackageInfo> cached = AppCache::loadFeaturedCache();
    if (!cached.isEmpty()) {
        m_featuredPackages = cached;
        createPackageCards();
        if (m_contentStack) m_contentStack->setCurrentIndex(1);
        (void)QtConcurrent::run([this]() {
            QVector<PackageInfo> packages = AlpmWrapper::instance().getInstalledPackages();
            QSet<QString> names;
            for (const auto& p : packages) names.insert(p.name);
            QMetaObject::invokeMethod(this, [this, names]() {
                for (auto* card : m_packageCards) {
                    card->updateInstallStatus(names.contains(card->packageInfo().name));
                }
            }, Qt::QueuedConnection);
        });
    }

    const QVector<QPair<QString, QString>> featured = {
        {"firefox", "extra"},
        {"gimp", "extra"},
        {"vlc", "extra"},
        {"telegram-desktop", "extra"},
        {"obs-studio", "extra"},
        {"blender", "extra"},
        {"spotify", "AUR"},
        {"discord", "extra"},
        {"google-chrome", "AUR"},
        {"visual-studio-code-bin", "AUR"},
        {"libreoffice-still", "extra"},
        {"zoom", "AUR"}
    };
    (void)QtConcurrent::run([this, featured]() {
        buildFeaturedListAndRefreshUi(featured);
    });
    loadPromoImages();
}

static QPixmap roundedPixmap(const QPixmap& src, const QSize& size, int radius) {
    if (src.isNull() || size.isEmpty()) return QPixmap();
    QPixmap scaled = src.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap out(size);
    out.fill(Qt::transparent);
    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, size.width(), size.height()), radius, radius);
    p.setClipPath(path);
    p.drawPixmap(0, 0, size.width(), size.height(), scaled);
    p.end();
    return out;
}

void HomeWidget::loadPromoImages() {
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    const int radius = 14;
    auto setImage = [this, radius](QNetworkReply* reply, QFrame* frame, QLabel* label) {
        if (!reply || !frame || !label) return;
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        QByteArray data = reply->readAll();
        QPixmap pix;
        if (!pix.loadFromData(data)) return;
        QSize size = frame->size();
        if (size.width() <= 0 || size.height() <= 0)
            size = QSize(qMax(200, frame->width()), qMax(200, frame->height()));
        QPixmap rounded = roundedPixmap(pix, size, radius);
        if (rounded.isNull()) return;
        label->setGeometry(0, 0, size.width(), size.height());
        label->setPixmap(rounded);
        label->show();
    };
    auto* r1 = nam->get(QNetworkRequest(QUrl(QStringLiteral("https://apps.pearos.xyz/banner.png"))));
    auto* r2 = nam->get(QNetworkRequest(QUrl(QStringLiteral("https://apps.pearos.xyz/card2.png"))));
    auto* r3 = nam->get(QNetworkRequest(QUrl(QStringLiteral("https://apps.pearos.xyz/card3.png"))));
    connect(r1, &QNetworkReply::finished, this, [this, setImage, r1]() { setImage(r1, m_bannerCard, m_bannerImage); });
    connect(r2, &QNetworkReply::finished, this, [this, setImage, r2]() { setImage(r2, m_promoCard1, m_promoImage1); });
    connect(r3, &QNetworkReply::finished, this, [this, setImage, r3]() { setImage(r3, m_promoCard2, m_promoImage2); });
}

void HomeWidget::refreshWhenRepositoriesReady() {
    const QVector<QPair<QString, QString>> featured = {
        {"firefox", "extra"},
        {"gimp", "extra"},
        {"vlc", "extra"},
        {"telegram-desktop", "extra"},
        {"obs-studio", "extra"},
        {"blender", "extra"},
        {"spotify", "AUR"},
        {"discord", "extra"},
        {"google-chrome", "AUR"},
        {"visual-studio-code-bin", "AUR"},
        {"libreoffice-still", "extra"},
        {"zoom", "AUR"}
    };
    (void)QtConcurrent::run([this, featured]() {
        buildFeaturedListAndRefreshUi(featured);
    });
}

void HomeWidget::buildFeaturedListAndRefreshUi(const QVector<QPair<QString, QString>>& featured) {
    static const QStringList descriptions = {
        "Fast, Private & Safe Web Browser",
        "GNU Image Manipulation Program",
        "Multi-platform MPEG, VCD/DVD, and DivX player",
        "Official Telegram Desktop client",
        "Free, open source software for live streaming and recording",
        "A fully integrated 3D graphics creation suite",
        "A proprietary music streaming service",
        "All-in-one voice and text chat for gamers",
        "The popular web browser by Google",
        "Visual Studio Code (official binary version)",
        "Free and Open Source Office Suite",
        "Video Conferencing and Web Conferencing Service"
    };
    QVector<PackageInfo> list;
    list.reserve(featured.size());
    for (int i = 0; i < featured.size(); ++i) {
        PackageInfo pkg;
        pkg.name = featured[i].first;
        pkg.repository = featured[i].second;
        pkg.version = "Latest";
        pkg.description = (i < descriptions.size()) ? descriptions[i] : QString();
        PackageInfo repoInfo = AlpmWrapper::instance().getPackageInfo(pkg.name);
        if (!repoInfo.name.isEmpty() && !repoInfo.repository.isEmpty()) {
            pkg.repository = repoInfo.repository;
            pkg.version = repoInfo.version;
            pkg.description = repoInfo.description.isEmpty() ? pkg.description : repoInfo.description;
        }
        list.append(pkg);
    }
    Logger::info(QString("Loaded %1 featured packages").arg(list.size()));
    AppCache::saveFeaturedCache(list);
    QMetaObject::invokeMethod(this, [this, list]() {
        m_featuredPackages = list;
        createPackageCards();
        if (m_contentStack) m_contentStack->setCurrentIndex(1);
        (void)QtConcurrent::run([this]() {
            QVector<PackageInfo> packages = AlpmWrapper::instance().getInstalledPackages();
            QSet<QString> names;
            for (const auto& p : packages) names.insert(p.name);
            QMetaObject::invokeMethod(this, [this, names]() {
                for (auto* card : m_packageCards) {
                    card->updateInstallStatus(names.contains(card->packageInfo().name));
                }
            }, Qt::QueuedConnection);
        });
    }, Qt::QueuedConnection);
}

void HomeWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    auto* titleLabel = new QLabel("Discover", this);
    auto titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    m_contentStack = new QStackedWidget(this);
    m_contentStack->addWidget(new LoadingOverlay(this));
    
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    const QString cardStyle = "QFrame { background-color: #27272a; border: none; border-radius: 14px; }";
    const int cornerRadius = 14;

    m_bannerCard = new QFrame(m_contentWidget);
    m_bannerCard->setFixedHeight(300);
    m_bannerCard->setStyleSheet(cardStyle);
    m_bannerCard->setProperty("openUrl", QUrl(QStringLiteral("https://apps.pearos.xyz/1")));
    m_bannerImage = new QLabel(m_bannerCard);
    m_bannerImage->setAlignment(Qt::AlignCenter);
    m_bannerImage->setScaledContents(false);
    m_bannerImage->setStyleSheet("background: transparent; border: none; border-radius: " + QString::number(cornerRadius) + "px;");
    m_bannerImage->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_bannerImage->hide();
    contentLayout->addWidget(m_bannerCard);

    auto* twoCardRow = new QHBoxLayout();
    twoCardRow->setSpacing(24);
    twoCardRow->setContentsMargins(0, 28, 0, 0);
    m_promoCard1 = new QFrame(m_contentWidget);
    m_promoCard1->setFixedHeight(220);
    m_promoCard1->setStyleSheet(cardStyle);
    m_promoCard1->setProperty("openUrl", QUrl(QStringLiteral("https://apps.pearos.xyz/2")));
    m_promoImage1 = new QLabel(m_promoCard1);
    m_promoImage1->setAlignment(Qt::AlignCenter);
    m_promoImage1->setScaledContents(false);
    m_promoImage1->setStyleSheet("background: transparent; border: none; border-radius: " + QString::number(cornerRadius) + "px;");
    m_promoImage1->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_promoImage1->hide();
    m_promoCard2 = new QFrame(m_contentWidget);
    m_promoCard2->setFixedHeight(220);
    m_promoCard2->setStyleSheet(cardStyle);
    m_promoCard2->setProperty("openUrl", QUrl(QStringLiteral("https://apps.pearos.xyz/3")));
    m_promoImage2 = new QLabel(m_promoCard2);
    m_promoImage2->setAlignment(Qt::AlignCenter);
    m_promoImage2->setScaledContents(false);
    m_promoImage2->setStyleSheet("background: transparent; border: none; border-radius: " + QString::number(cornerRadius) + "px;");
    m_promoImage2->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_promoImage2->hide();
    twoCardRow->addWidget(m_promoCard1, 1);
    twoCardRow->addWidget(m_promoCard2, 1);
    contentLayout->addLayout(twoCardRow);

    struct PromoClickFilter : QObject {
        bool eventFilter(QObject* o, QEvent* e) override {
            if (e->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent*>(e)->button() == Qt::LeftButton) {
                QUrl url = o->property("openUrl").toUrl();
                if (!url.isValid()) return true;
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
                // Evită eroarea portal XDG "Could not register app ID" folosind xdg-open
                QProcess::startDetached(QStringLiteral("xdg-open"), {url.toString()});
#else
                QDesktopServices::openUrl(url);
#endif
                return true;
            }
            return QObject::eventFilter(o, e);
        }
    };
    static PromoClickFilter* promoFilter = new PromoClickFilter();
    m_bannerCard->installEventFilter(promoFilter);
    m_promoCard1->installEventFilter(promoFilter);
    m_promoCard2->installEventFilter(promoFilter);

    QFrame* sep = new QFrame(m_contentWidget);
    sep->setFixedHeight(1);
    sep->setStyleSheet("QFrame { background-color: rgba(255,255,255,0.08); border: none; }");
    contentLayout->addSpacing(50);
    contentLayout->addWidget(sep);
    contentLayout->addSpacing(20);

    m_gridLayout->setSpacing(35);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addLayout(m_gridLayout);
    
    m_contentStack->addWidget(m_scrollArea);
    applyScrollBarFade(m_scrollArea);
    mainLayout->addWidget(m_contentStack);
    setLayout(mainLayout);
}

void HomeWidget::loadFeaturedPackages() {
    // Sync version; prefer onInitialLoad() which runs this work in background to avoid blocking UI.
    m_featuredPackages = {
        {"firefox", "Latest", "Fast, Private & Safe Web Browser", "extra"},
        {"gimp", "Latest", "GNU Image Manipulation Program", "extra"},
        {"vlc", "Latest", "Multi-platform MPEG, VCD/DVD, and DivX player", "extra"},
        {"telegram-desktop", "Latest", "Official Telegram Desktop client", "extra"},
        {"obs-studio", "Latest", "Free, open source software for live streaming and recording", "extra"},
        {"blender", "Latest", "A fully integrated 3D graphics creation suite", "extra"},
        {"spotify", "Latest", "A proprietary music streaming service", "AUR"},
        {"discord", "Latest", "All-in-one voice and text chat for gamers", "extra"},
        {"google-chrome", "Latest", "The popular web browser by Google", "AUR"},
        {"visual-studio-code-bin", "Latest", "Visual Studio Code (official binary version)", "AUR"},
        {"libreoffice-still", "Latest", "Free and Open Source Office Suite", "extra"},
        {"zoom", "Latest", "Video Conferencing and Web Conferencing Service", "AUR"}
    };
    for (auto& pkg : m_featuredPackages) {
        PackageInfo repoInfo = AlpmWrapper::instance().getPackageInfo(pkg.name);
        if (!repoInfo.name.isEmpty() && !repoInfo.repository.isEmpty()) {
            pkg.repository = repoInfo.repository;
            pkg.version = repoInfo.version;
            pkg.description = repoInfo.description;
        } else if (pkg.repository.toLower() == "aur") {
            pkg.repository = "aur";
        }
    }
    Logger::info(QString("Loaded %1 featured packages").arg(m_featuredPackages.size()));
}

void HomeWidget::createPackageCards() {
    for (auto* card : m_packageCards) {
        if (card && card->parent() == m_contentWidget) {
            m_gridLayout->removeWidget(card);
            card->deleteLater();
        }
    }
    m_packageCards.clear();
    for (const auto& pkg : m_featuredPackages) {
        auto* card = new PackageCard(pkg, m_contentWidget, PackageCard::Discover);
        connect(card, &PackageCard::clicked, this, &HomeWidget::onPackageClicked);
        m_packageCards.append(card);
        card->loadAppStreamAsync();
    }
    m_columnCount = 2;
    refreshGridLayout();
}

int HomeWidget::columnCountForWidth(int width) {
    (void)width;
    return 2;
}

void HomeWidget::refreshGridLayout() {
    while (QLayoutItem* item = m_gridLayout->takeAt(0)) {
        delete item;
    }
    int row = 0, col = 0;
    for (auto* card : m_packageCards) {
        m_gridLayout->addWidget(card, row, col);
        col++;
        if (col >= m_columnCount) {
            col = 0;
            row++;
        }
    }
    m_gridLayout->setRowStretch(row + 1, 1);
}

void HomeWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    m_columnCount = 2;
    refreshGridLayout();
}

void HomeWidget::checkInstalledPackages() {
    for (auto* card : m_packageCards) {
        card->checkInstallStatus();
    }
}

void HomeWidget::onUpdateTimer() {
    if (m_packageCards.isEmpty()) return;
    (void)QtConcurrent::run([this]() {
        QVector<PackageInfo> packages = AlpmWrapper::instance().getInstalledPackages();
        QSet<QString> names;
        for (const auto& p : packages) {
            names.insert(p.name);
        }
        QMetaObject::invokeMethod(this, [this, names]() {
            for (auto* card : m_packageCards) {
                card->updateInstallStatus(names.contains(card->packageInfo().name));
            }
        }, Qt::QueuedConnection);
    });
}

void HomeWidget::onPackageClicked(const PackageInfo& info) {
    Logger::info(QString("Package clicked: %1").arg(info.name));
    
    // Fetch full package details including dependencies
    PackageInfo fullInfo;
    
    if (info.repository.toLower() == "aur") {
        // For AUR packages, query AUR API for full details
        AurHelper aurHelper;
        QEventLoop loop;
        
        connect(&aurHelper, &AurHelper::packageInfoReceived, [&fullInfo, &loop](const PackageInfo& aurInfo) {
            fullInfo = aurInfo;
            loop.quit();
        });
        
        connect(&aurHelper, &AurHelper::error, [&fullInfo, &info, &loop](const QString& error) {
            Logger::warning(QString("Failed to fetch AUR package info: %1").arg(error));
            fullInfo = info; // Fallback to basic info
            loop.quit();
        });
        
        aurHelper.getPackageInfo(info.name);
        loop.exec(); // Wait for response
        
        // If we didn't get full info, use the basic info
        if (fullInfo.name.isEmpty()) {
            fullInfo = info;
        }
    } else {
        // For official repos, fetch full details from ALPM
        fullInfo = AlpmWrapper::instance().getPackageInfo(info.name);
        // If not found, use the basic info
        if (fullInfo.name.isEmpty()) {
            fullInfo = info;
        }
    }
    
    emit openPackageRequested(fullInfo);
    checkInstalledPackages();
}
