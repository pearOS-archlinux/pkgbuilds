#include "mainwindow.h"
#include "home_widget.h"
#include "search_widget.h"
#include "installed_widget.h"
#include "updates_widget.h"
#include "settings_widget.h"
#include "user_page_widget.h"
#include "package_details_dialog.h"
#include "../utils/logger.h"
#include "../utils/app_cache.h"
#include "../core/alpm_wrapper.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QDir>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QMouseEvent>
#include <QWindow>
#include <QShowEvent>
#include <QResizeEvent>
#include <QPalette>
#include <QTimer>
#include <QtConcurrent>
#include <QGraphicsBlurEffect>
#include <QPixmap>
#include <QProcess>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QSettings>
#ifdef HAVE_KWINDOW_EFFECTS
#include <KWindowEffects>
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::ArrowCursor);

    setupUi();
    loadStyleSheet();
    qApp->installEventFilter(this);

    if (m_headerWidget) {
        m_headerWidget->installEventFilter(this);
    }
    if (m_centralWidget) {
        m_centralWidget->installEventFilter(this);
        m_centralWidget->setMouseTracking(true);
    }

    Logger::info("MainWindow created successfully");
}

MainWindow::~MainWindow() {
    AlpmWrapper::instance().release();
    Logger::info("MainWindow destroyed");
}

void MainWindow::setupUi() {
    setWindowTitle("AppStore");
    setMinimumSize(800, 600);
    resize(1200, 800);

    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName("appContainer");
    auto* appLayout = new QVBoxLayout(m_centralWidget);
    appLayout->setContentsMargins(0, 4, 2, 4);
    appLayout->setSpacing(0);

    auto* wrapper = new QWidget(this);
    wrapper->setObjectName("wrapper");
    auto* wrapperLayout = new QHBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(8, 4, 0, 4);
    wrapperLayout->setSpacing(7);

    setupSidebar();
    wrapperLayout->addWidget(m_leftSideWidget);

    m_rightColumn = new QWidget(this);
    m_rightColumn->setObjectName("rightColumn");
    auto* rightLayout = new QVBoxLayout(m_rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    setupHeader();
    rightLayout->addWidget(m_headerWidget);

    QWidget* contentWrapper = new QWidget(this);
    contentWrapper->setObjectName("contentWrapper");
    m_contentLayout = new QVBoxLayout(contentWrapper);
    m_contentLayout->setContentsMargins(24, 16, 0, 16);
    m_contentLayout->setSpacing(0);
    auto* loadingLabel = new QLabel(tr("Loading repositories..."), this);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet("color: #a1a1aa; font-size: 15px;");
    m_loadingPlaceholder = loadingLabel;
    m_contentLayout->addWidget(m_loadingPlaceholder, 1);
    rightLayout->addWidget(contentWrapper, 1);

    wrapperLayout->addWidget(m_rightColumn, 1);

    m_reflectionDebounceTimer = new QTimer(this);
    m_reflectionDebounceTimer->setSingleShot(true);
    connect(m_reflectionDebounceTimer, &QTimer::timeout, this, &MainWindow::updateSidebarReflection);
    m_reflectionUpdateTimer = new QTimer(this);
    connect(m_reflectionUpdateTimer, &QTimer::timeout, this, &MainWindow::updateSidebarReflection);
    m_reflectionUpdateTimer->start(0); // 0 = cât de des permite event loop-ul, blur în timp real
    QTimer::singleShot(500, this, &MainWindow::updateSidebarReflection);

    appLayout->addWidget(wrapper, 1);

    setCentralWidget(m_centralWidget);
    createMenuBar();
}

void MainWindow::setupHeader() {
    m_headerWidget = new QWidget(this);
    m_headerWidget->setObjectName("titleBarDrag");
    m_headerWidget->setFixedHeight(8);
    m_headerWidget->setCursor(Qt::SizeAllCursor);
    m_headerWidget->setStyleSheet("QWidget#titleBarDrag { background: transparent; }");
}

namespace {
const int PAGE_DISCOVER = 0;
const int PAGE_SEARCH = 1;
const int PAGE_ARCADE = 2;
const int PAGE_WORK = 3;
const int PAGE_PLAY = 4;
const int PAGE_DEVELOP = 5;
const int PAGE_CATEGORIES = 6;
const int PAGE_UPDATES = 7;
const int PAGE_USER = 8;

QPixmap tintPixmap(const QPixmap& src, const QColor& color) {
    if (src.isNull()) return src;
    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32);
    const int a = color.alpha();
    const int r = color.red();
    const int g = color.green();
    const int b = color.blue();
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QRgb pixel = img.pixel(x, y);
            int alpha = qAlpha(pixel);
            img.setPixel(x, y, qRgba(r, g, b, (alpha * a) / 255));
        }
    }
    return QPixmap::fromImage(img);
}

const int UPDATES_ROW = 6;
const int UPDATES_BADGE_ROLE = Qt::UserRole;

class SidebarDelegate : public QStyledItemDelegate {
public:
    explicit SidebarDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const bool selected = (opt.state & QStyle::State_Selected) != 0;
        QColor fg = selected ? opt.palette.color(QPalette::Highlight) : QColor(250, 250, 250);
        const int row = index.row();
        static const QColor selectedBg(0x30, 0x31, 0x32); // #303132, același ca User și item:selected

        painter->save();
        if (selected) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(selectedBg);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setRenderHint(QPainter::SmoothPixmapTransform);
            painter->drawRoundedRect(opt.rect.adjusted(2, 1, -2, -1), 8, 8);
            painter->setBrush(Qt::NoBrush);
        }
        painter->setPen(fg);

        int iconSize = 22;
        if (const QWidget* w = opt.widget) {
            if (const QListWidget* list = qobject_cast<const QListWidget*>(w))
                iconSize = list->iconSize().width();
        }
        QRect iconRect = opt.rect;
        iconRect.setWidth(iconSize);
        iconRect.setHeight(iconSize);
        iconRect.translate(12, (opt.rect.height() - iconSize) / 2);

        QVariant iconVar = index.data(Qt::DecorationRole);
        if (iconVar.canConvert<QIcon>()) {
            QIcon icon = iconVar.value<QIcon>();
            QPixmap px = icon.pixmap(iconSize, iconSize);
            if (!px.isNull())
                painter->drawPixmap(iconRect, tintPixmap(px, fg));
        }

        int badgeW = 0;
        if (row == UPDATES_ROW) {
            int count = index.data(UPDATES_BADGE_ROLE).toInt();
            if (count > 0) badgeW = 26;
        }
        QRect textRect = opt.rect;
        textRect.setLeft(iconRect.left() + iconSize + 8);
        textRect.setTop(iconRect.top());
        textRect.setHeight(iconSize);
        textRect.setRight(opt.rect.right() - 4 - badgeW);
        painter->setFont(opt.font);
        if (selected) painter->setFont(QFont(opt.font.family(), opt.font.pointSize(), QFont::Medium));
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());

        if (row == UPDATES_ROW) {
            int count = index.data(UPDATES_BADGE_ROLE).toInt();
            if (count > 0) {
                const int badgeSize = 22;
                const int badgeRight = opt.rect.right() - 4;
                QRect badgeRect(badgeRight - badgeSize, opt.rect.y() + (opt.rect.height() - badgeSize) / 2, badgeSize, badgeSize);
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(82, 82, 91)); // grey badge for updates
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setRenderHint(QPainter::SmoothPixmapTransform);
                painter->drawRoundedRect(badgeRect, badgeSize / 2, badgeSize / 2);
                painter->setPen(Qt::white);
                painter->setBrush(Qt::NoBrush);
                QFont badgeFont = opt.font;
                badgeFont.setPointSize(10);
                badgeFont.setWeight(QFont::DemiBold);
                painter->setFont(badgeFont);
                QString badgeText = count > 99 ? QStringLiteral("99+") : QString::number(count);
                painter->drawText(badgeRect, Qt::AlignCenter, badgeText);
            }
        }
        painter->restore();
    }
};
} // namespace

void MainWindow::setupSidebar() {
    m_leftSideWidget = new QFrame(this);
    m_leftSideWidget->setObjectName("leftSide");
    m_leftSideWidget->setFixedWidth(240);

    m_sidebarReflection = new QLabel(m_leftSideWidget);
    m_sidebarReflection->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_sidebarReflection->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_sidebarReflection->setScaledContents(true);
    m_sidebarReflection->setStyleSheet("background: transparent;");
    auto* blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(65);
    blurEffect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
    m_sidebarReflection->setGraphicsEffect(blurEffect);

    auto* sideLayout = new QVBoxLayout(m_leftSideWidget);
    sideLayout->setContentsMargins(12, 12, 26, 20);
    sideLayout->setSpacing(8);

    QWidget* menuCircles = new QWidget(this);
    menuCircles->setObjectName("menuCircles");
    menuCircles->setCursor(Qt::ArrowCursor);
    menuCircles->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto* circlesLayout = new QHBoxLayout(menuCircles);
    circlesLayout->setContentsMargins(0, 0, 0, 0);
    circlesLayout->setSpacing(6);
    circlesLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    auto makeTrafficBtn = [this](const QString& bgColor, const QString& symbol) -> QPushButton* {
        QPushButton* btn = new QPushButton(this);
        btn->setFixedSize(15, 15);
        btn->setCursor(Qt::ArrowCursor);
        btn->setFlat(true);
        btn->setStyleSheet("background: " + bgColor + "; border: none; border-radius: 7px;");
        QLabel* lbl = new QLabel(symbol, btn);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setFixedSize(15, 15);
        lbl->setAttribute(Qt::WA_TransparentForMouseEvents);
        lbl->setStyleSheet("color: " + bgColor + "; font-size: 13px; font-weight: bold; background: transparent;");
        lbl->raise();
        btn->setProperty("trafficLabel", QVariant::fromValue<QLabel*>(lbl));
        return btn;
    };
    QPushButton* btnClose = makeTrafficBtn(QStringLiteral("#f96057"), QStringLiteral("\u00D7"));
    connect(btnClose, &QPushButton::clicked, this, &QMainWindow::close);
    circlesLayout->addWidget(btnClose);
    QPushButton* btnMin = makeTrafficBtn(QStringLiteral("#f8ce52"), QStringLiteral("\u2212"));
    connect(btnMin, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    circlesLayout->addWidget(btnMin);
    QPushButton* btnMax = makeTrafficBtn(QStringLiteral("#5fcf65"), QStringLiteral("\u25A1"));
    connect(btnMax, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) showNormal(); else showMaximized();
    });
    circlesLayout->addWidget(btnMax);
    auto getTrafficLabel = [](QPushButton* b) -> QLabel* {
        return b ? b->property("trafficLabel").value<QLabel*>() : nullptr;
    };
    struct TrafficHoverFilter : QObject {
        QWidget* circles = nullptr;
        QPushButton* bClose = nullptr;
        QPushButton* bMin = nullptr;
        QPushButton* bMax = nullptr;
        QLabel* lClose = nullptr;
        QLabel* lMin = nullptr;
        QLabel* lMax = nullptr;
        void setHovered(bool hovered) {
            const QString onColor = QStringLiteral("#1a1a1a");
            auto setLabel = [hovered, onColor](QLabel* l, const QString& hideColor) {
                if (!l) return;
                l->setStyleSheet("color: " + (hovered ? onColor : hideColor) + "; font-size: 13px; font-weight: bold; background: transparent;");
            };
            if (lClose) setLabel(lClose, QStringLiteral("#f96057"));
            if (lMin)   setLabel(lMin,   QStringLiteral("#f8ce52"));
            if (lMax)   setLabel(lMax,   QStringLiteral("#5fcf65"));
        }
        bool eventFilter(QObject* o, QEvent* e) override {
            if (!circles) return QObject::eventFilter(o, e);
            if (e->type() == QEvent::Enter) {
                setHovered(true);
            } else if (e->type() == QEvent::Leave) {
                QWidget* w = qApp->widgetAt(QCursor::pos());
                bool stillInside = w && (w == circles || circles->isAncestorOf(w));
                if (!stillInside) setHovered(false);
            }
            return QObject::eventFilter(o, e);
        }
    };
    auto* trafficFilter = new TrafficHoverFilter();
    trafficFilter->circles = menuCircles;
    trafficFilter->bClose = btnClose;
    trafficFilter->bMin = btnMin;
    trafficFilter->bMax = btnMax;
    trafficFilter->lClose = getTrafficLabel(btnClose);
    trafficFilter->lMin   = getTrafficLabel(btnMin);
    trafficFilter->lMax   = getTrafficLabel(btnMax);
    trafficFilter->setParent(menuCircles);
    trafficFilter->setHovered(false);
    menuCircles->installEventFilter(trafficFilter);
    btnClose->installEventFilter(trafficFilter);
    btnMin->installEventFilter(trafficFilter);
    btnMax->installEventFilter(trafficFilter);
    sideLayout->addWidget(menuCircles);
    sideLayout->addSpacing(20);

    m_sidebarSearch = new QLineEdit(this);
    m_sidebarSearch->setObjectName("sidebarSearch");
    m_sidebarSearch->setCursor(Qt::IBeamCursor);
    m_sidebarSearch->setPlaceholderText("Search");
    m_sidebarSearch->setMinimumHeight(32);
    m_sidebarSearch->setClearButtonEnabled(true);
    connect(m_sidebarSearch, &QLineEdit::returnPressed, this, [this]() {
        QString q = m_sidebarSearch->text().trimmed();
        if (!q.isEmpty() && m_searchWidget) {
            setPage(1);
            m_searchWidget->setSearchQuery(q);
        }
    });
    sideLayout->addWidget(m_sidebarSearch);
    sideLayout->addSpacing(20);

    m_sideList = new QListWidget(this);
    m_sideList->setObjectName("sideList");
    m_sideList->setCursor(Qt::ArrowCursor);
    const int iconSize = 22;
    m_sideList->setIconSize(QSize(iconSize, iconSize));
    m_sideList->setItemDelegate(new SidebarDelegate(this));
    auto addSideItem = [this](const QString& text, const QString& iconPath) {
        m_sideList->addItem(new QListWidgetItem(QIcon(iconPath), text));
    };
    addSideItem(tr("Discover"), QStringLiteral(":/icons/sidebar_icons/discover.png"));
    addSideItem(tr("Arcade"), QStringLiteral(":/icons/sidebar_icons/arcade.png"));
    addSideItem(tr("Work"), QStringLiteral(":/icons/sidebar_icons/work.png"));
    addSideItem(tr("Play"), QStringLiteral(":/icons/sidebar_icons/play.png"));
    addSideItem(tr("Develop"), QStringLiteral(":/icons/sidebar_icons/develop.png"));
    addSideItem(tr("Categories"), QStringLiteral(":/icons/sidebar_icons/categories.png"));
    auto* updatesItem = new QListWidgetItem(QIcon(QStringLiteral(":/icons/sidebar_icons/updates.png")), tr("Updates"));
    updatesItem->setData(UPDATES_BADGE_ROLE, 0);
    m_sideList->addItem(updatesItem);
    m_sideList->setCurrentRow(0);
    const int pageByRow[] = {PAGE_DISCOVER, PAGE_ARCADE, PAGE_WORK, PAGE_PLAY, PAGE_DEVELOP, PAGE_CATEGORIES, PAGE_UPDATES};
    connect(m_sideList, &QListWidget::currentRowChanged, this, [this, pageByRow](int row) {
        if (row >= 0 && row < 7) setPage(pageByRow[row]);
    });
    sideLayout->addWidget(m_sideList, 1);

    m_sidebarProfileWidget = new QWidget(this);
    m_sidebarProfileWidget->setObjectName("sidebarProfile");
    auto* profileLayout = new QHBoxLayout(m_sidebarProfileWidget);
    profileLayout->setContentsMargins(12, 10, 12, 10);
    profileLayout->setSpacing(12);

    const int avatarSize = 40;
    QLabel* avatar = new QLabel(this);
    avatar->setObjectName("sidebarProfileImg");
    avatar->setFixedSize(avatarSize, avatarSize);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setScaledContents(false);
    QString facePath = QDir::homePath() + QStringLiteral("/.face.icon");
    if (QFile::exists(facePath)) {
        QPixmap face(facePath);
        if (!face.isNull()) {
            QPixmap rounded(avatarSize, avatarSize);
            rounded.fill(Qt::transparent);
            QPainter p(&rounded);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            QPainterPath path;
            path.addEllipse(0, 0, avatarSize, avatarSize);
            p.setClipPath(path);
            p.drawPixmap(0, 0, avatarSize, avatarSize, face.scaled(avatarSize, avatarSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            p.end();
            avatar->setPixmap(rounded);
        }
    }
    profileLayout->addWidget(avatar);
    m_sidebarProfileName = new QLabel(tr("User"), this);
    m_sidebarProfileName->setObjectName("sidebarProfileName");
    m_sidebarProfileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    profileLayout->addWidget(m_sidebarProfileName);
    sideLayout->addWidget(m_sidebarProfileWidget);
    loadPearIdDisplayName();
    struct ProfileClickFilter : QObject {
        MainWindow* mw = nullptr;
        bool eventFilter(QObject* o, QEvent* e) override {
            if (e->type() == QEvent::MouseButtonRelease && mw) {
                mw->setPage(PAGE_USER);
                return true;
            }
            return QObject::eventFilter(o, e);
        }
    };
    auto* filter = new ProfileClickFilter();
    filter->mw = this;
    filter->setParent(m_sidebarProfileWidget);
    m_sidebarProfileWidget->installEventFilter(filter);
}

void MainWindow::loadPearIdDisplayName() {
    if (!m_sidebarProfileName) return;
    QString scriptPath;
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList candidates = {
        appDir.absoluteFilePath(QStringLiteral("../pearID/get_user_info.sh")),
        appDir.absoluteFilePath(QStringLiteral("pearID/get_user_info.sh")),
        QStringLiteral("/usr/share/pearos-appstore/pearID/get_user_info.sh")
    };
    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            scriptPath = path;
            break;
        }
    }
    if (scriptPath.isEmpty()) return;
    QProcess* process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        if (exitCode != 0 || exitStatus != QProcess::NormalExit || !m_sidebarProfileName) return;
        const QByteArray out = process->readAllStandardOutput().trimmed();
        const QList<QByteArray> lines = out.split('\n');
        QString first = QString::fromUtf8(lines.value(0).trimmed());
        QString last = QString::fromUtf8(lines.value(1).trimmed());
        if (!first.isEmpty() || !last.isEmpty()) {
            m_sidebarProfileName->setText((first + QChar(' ') + last).trimmed());
        }
    });
    process->start(scriptPath, {QStringLiteral("--first-name"), QStringLiteral("--last-name")}, QProcess::ReadOnly);
}

void MainWindow::ensureSettingsWidget() {
    if (m_settingsWidget) return;
    m_settingsWidget = new SettingsWidget(this);
    connect(m_settingsWidget, &SettingsWidget::multilibStatusChanged,
            this, [this]() {
        if (m_searchWidget && m_settingsWidget)
            m_searchWidget->updateRepositoryList(
                m_settingsWidget->isMultilibEnabled(),
                m_settingsWidget->isChaoticAurEnabled());
    });
    connect(m_settingsWidget, &SettingsWidget::chaoticAurStatusChanged,
            this, [this]() {
        if (m_searchWidget && m_settingsWidget)
            m_searchWidget->updateRepositoryList(
                m_settingsWidget->isMultilibEnabled(),
                m_settingsWidget->isChaoticAurEnabled());
    });
    if (m_searchWidget && m_settingsWidget)
        m_searchWidget->updateRepositoryList(
            m_settingsWidget->isMultilibEnabled(),
            m_settingsWidget->isChaoticAurEnabled());
}

void MainWindow::ensureSettingsWidgetForUserPage() {
    ensureSettingsWidget();
    if (m_userPageWidget && m_settingsWidget)
        m_userPageWidget->setSettingsWidget(m_settingsWidget);
}

static QWidget* makePlaceholder(const QString& title, QWidget* parent) {
    auto* w = new QWidget(parent);
    auto* lay = new QVBoxLayout(w);
    lay->addStretch();
    auto* lbl = new QLabel(title, w);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("color: #a1a1aa; font-size: 15px;");
    lay->addWidget(lbl);
    lay->addStretch();
    return w;
}

static QWidget* makeArcadePlaceholder(QWidget* parent) {
    auto* w = new QWidget(parent);
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addStretch();

    auto* card = new QFrame(w);
    card->setObjectName(QStringLiteral("arcadePlaceholderCard"));
    card->setMinimumWidth(360);
    card->setFixedHeight(160);
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 24, 24, 24);

    auto* title = new QLabel(QObject::tr("Arcade is under construction"), card);
    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    title->setStyleSheet("font-size: 18px; font-weight: 600; color: #fafafa;");

    auto* subtitle = new QLabel(QObject::tr("It will be available in a future update."), card);
    subtitle->setWordWrap(true);
    subtitle->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    subtitle->setStyleSheet("font-size: 13px; color: #a1a1aa; margin-top: 6px;");

    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    lay->addWidget(card, 0, Qt::AlignHCenter);
    lay->addStretch();

    // Match card background with the app's card style via stylesheet rule
    card->setStyleSheet("QFrame#arcadePlaceholderCard { background-color: #393b3e; border-radius: 16px; }");

    return w;
}

void MainWindow::setupContent() {
    m_homeWidget = new HomeWidget(this);
    m_searchWidget = new SearchWidget(this);
    m_installedWidget = new InstalledWidget(this);
    m_updatesWidget = new UpdatesWidget(this);
    m_userPageWidget = new UserPageWidget(m_installedWidget, this, this);

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName("mainContainer");
    m_stackedWidget->addWidget(m_homeWidget);
    m_stackedWidget->addWidget(m_searchWidget);
    m_stackedWidget->addWidget(makeArcadePlaceholder(this));
    m_stackedWidget->addWidget(makePlaceholder(tr("Work"), this));
    m_stackedWidget->addWidget(makePlaceholder(tr("Play"), this));
    m_stackedWidget->addWidget(makePlaceholder(tr("Develop"), this));
    m_stackedWidget->addWidget(makePlaceholder(tr("Categories"), this));
    m_stackedWidget->addWidget(m_updatesWidget);
    m_stackedWidget->addWidget(m_userPageWidget);

    m_contentStack = new QStackedWidget(this);
    m_contentStack->addWidget(m_stackedWidget);

    m_detailsPage = new QWidget(this);
    m_detailsPage->setObjectName("detailsPage");
    m_detailsPage->setStyleSheet("QWidget#detailsPage { background-color: #1e2123; }");
    m_detailsLayout = new QVBoxLayout(m_detailsPage);
    m_detailsLayout->setContentsMargins(0, 0, 0, 0);
    m_detailsLayout->setSpacing(0);
    QHBoxLayout* backRow = new QHBoxLayout();
    backRow->setContentsMargins(12, 8, 12, 6);
    backRow->setSpacing(0);
    QPushButton* backBtn = new QPushButton(m_detailsPage);
    backBtn->setObjectName("detailsBackButton");
    backBtn->setText(tr("←"));
    backBtn->setFixedSize(40, 40);
    backBtn->setStyleSheet(
        "QPushButton#detailsBackButton {"
        "  background-color: #393b3e; border: none; border-radius: 20px;"
        "  color: #fafafa; font-size: 18px; font-weight: 500; padding: 0;"
        "}"
        "QPushButton#detailsBackButton:hover { background-color: #52525b; color: #fafafa; }"
    );
    backBtn->setCursor(Qt::ArrowCursor);
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::onBackFromDetails);
    backRow->addWidget(backBtn);
    backRow->addStretch();
    m_detailsLayout->addLayout(backRow);
    m_contentStack->addWidget(m_detailsPage);

    m_searchWidget->updateRepositoryList(false, false);

    connect(m_homeWidget, &HomeWidget::openPackageRequested, this, &MainWindow::onOpenPackageRequested);
    connect(m_searchWidget, &SearchWidget::openPackageRequested, this, &MainWindow::onOpenPackageRequested);
    connect(m_installedWidget, &InstalledWidget::openPackageRequested, this, &MainWindow::onOpenPackageRequested);
    connect(m_updatesWidget, &UpdatesWidget::updatesCountChanged, this, [this](int count) {
        if (!m_sideList || m_sideList->count() <= UPDATES_ROW) return;
        m_sideList->item(UPDATES_ROW)->setData(UPDATES_BADGE_ROLE, count);
        m_sideList->viewport()->update();
    });
}

void MainWindow::finishContentSetup() {
    if (!m_contentLayout || !m_loadingPlaceholder) return;
    m_contentLayout->removeWidget(m_loadingPlaceholder);
    m_loadingPlaceholder->deleteLater();
    m_loadingPlaceholder = nullptr;
    setupContent();
    m_contentLayout->addWidget(m_contentStack, 1);
    setPage(0);
    // Badge Updates: încarcă count din cache la pornire
    if (m_sideList && m_sideList->count() > UPDATES_ROW) {
        QVector<UpdateInfo> cached = AppCache::loadUpdatesCache();
        m_sideList->item(UPDATES_ROW)->setData(UPDATES_BADGE_ROLE, cached.size());
        m_sideList->viewport()->update();
    }
}

void MainWindow::runInitialSyncIfNeeded() {
    QSettings settings;
    const QString key = QStringLiteral("initialSyncDone");
    const bool alreadyDone = settings.value(key, false).toBool();
    if (alreadyDone)
        return;

    // Rulează o singură dată la prima lansare: pkexec pacman -Sy
    QProcess::startDetached(QStringLiteral("pkexec"),
                            {QStringLiteral("pacman"), QStringLiteral("-Sy")});
    settings.setValue(key, true);
}

void MainWindow::setPage(int index) {
    if (!m_stackedWidget) return;
    // Dacă suntem pe pagina de detalii pachet și utilizatorul alege alt item din sidebar, închidem detaliile
    if (m_contentStack && m_contentStack->currentIndex() == 1) {
        if (m_currentDetailsDialog) {
            m_detailsLayout->removeWidget(m_currentDetailsDialog);
            m_currentDetailsDialog->deleteLater();
            m_currentDetailsDialog = nullptr;
        }
        if (m_contentLayout) {
            m_contentLayout->setContentsMargins(24, 16, 0, 16);
        }
        m_contentStack->setCurrentIndex(0);
    }
    m_stackedWidget->setCurrentIndex(index);
    if (m_sideList) {
        const int rowByPage[] = {0, -1, 1, 2, 3, 4, 5, 6, -1}; // 0=Discover..6=Updates, 8=User (fără row, profil jos)
        int row = (index >= 0 && index <= 8) ? rowByPage[index] : -1;
        m_sideList->blockSignals(true);
        m_sideList->setCurrentRow(row >= 0 ? row : -1);
        m_sideList->blockSignals(false);
    }
    const bool onUserPage = (index == PAGE_USER);
    if (m_sidebarProfileWidget) {
        m_sidebarProfileWidget->setProperty("selected", onUserPage);
        m_sidebarProfileWidget->style()->unpolish(m_sidebarProfileWidget);
        m_sidebarProfileWidget->style()->polish(m_sidebarProfileWidget);
    }
    updateSidebarReflection();
}

void MainWindow::onOpenPackageRequested(const PackageInfo& info) {
    if (m_currentDetailsDialog) {
        m_detailsLayout->removeWidget(m_currentDetailsDialog);
        m_currentDetailsDialog->deleteLater();
        m_currentDetailsDialog = nullptr;
    }
    m_currentDetailsDialog = new PackageDetailsDialog(info, this, true);
    m_detailsLayout->addWidget(m_currentDetailsDialog, 1);
    m_contentStack->setCurrentIndex(1);
}

void MainWindow::onBackFromDetails() {
    if (m_currentDetailsDialog) {
        m_detailsLayout->removeWidget(m_currentDetailsDialog);
        m_currentDetailsDialog->deleteLater();
        m_currentDetailsDialog = nullptr;
    }
    if (m_contentLayout) {
        m_contentLayout->setContentsMargins(24, 16, 0, 16);
    }
    m_contentStack->setCurrentIndex(0);
    updateSidebarReflection();
}

void MainWindow::updateSidebarReflection() {
    if (!m_rightColumn || !m_sidebarReflection || !m_leftSideWidget) return;
    if (!m_rightColumn->isVisible() || m_rightColumn->width() < 20 || m_rightColumn->height() < 20) return;

    const int displayWidth = 100;
    const int grabWidth = 72;
    const int h = m_rightColumn->height();
    QPixmap strip = m_rightColumn->grab(QRect(0, 0, grabWidth, h));
    if (strip.isNull()) return;

    const int w = m_leftSideWidget->width();
    m_sidebarReflection->setGeometry(w - displayWidth, 0, displayWidth, m_leftSideWidget->height());
    m_sidebarReflection->setPixmap(strip.scaled(displayWidth, h, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    m_sidebarReflection->lower();
}

void MainWindow::createMenuBar() {
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* refreshAction = new QAction("&Refresh", this);
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, [this]() {
        if (!m_stackedWidget) return;
        int idx = m_stackedWidget->currentIndex();
        if (idx == PAGE_UPDATES) m_updatesWidget->checkForUpdates();
        else if (idx == PAGE_USER) m_installedWidget->refreshPackages();
    });
    fileMenu->addAction(refreshAction);
    fileMenu->addSeparator();
    auto* quitAction = new QAction("&Quit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(quitAction);

    auto* helpMenu = menuBar()->addMenu("&Help");
    auto* aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About AppStore",
            "AppStore\n\n"
            "A modern package manager for Arch Linux\n"
            "Version: 0.2.28\n"
            "Built with Qt6 and C++17\n\n"
            "© 2025 Arka Linux GUI");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::loadStyleSheet() {
    QFile styleFile(":/stylesheet.qss");
    if (!styleFile.exists()) styleFile.setFileName("stylesheet.qss");
    if (!styleFile.exists()) styleFile.setFileName("/usr/share/pearos-appstore/stylesheet.qss");

    QString base;
    if (styleFile.open(QFile::ReadOnly)) {
        base = QString::fromUtf8(styleFile.readAll());
        styleFile.close();
        Logger::info(QString("Stylesheet loaded from: %1").arg(styleFile.fileName()));
    } else {
        Logger::warning("Could not load stylesheet");
    }
    base += "\n/* Theme from system palette */\n" + themeOverlayFromPalette();
    qApp->setStyleSheet(base);
}

QString MainWindow::themeOverlayFromPalette() const {
    const QPalette p = qApp->palette();
    const QString accent   = p.color(QPalette::Highlight).name();
    const QString accentText = p.color(QPalette::HighlightedText).name();

    // Culori: bg #1e2123, search #393a3b, selected sidebar #303132, carduri #393b3e
    const QString bg       = QStringLiteral("#1e2123");
    const QString surface  = QStringLiteral("#27272a");
    const QString cardBg   = QStringLiteral("#393b3e");
    const QString border   = QStringLiteral("#52525b");
    const QString text     = QStringLiteral("#fafafa");
    const QString textMuted= QStringLiteral("#a1a1aa");
    const QString placeholder = QStringLiteral("#71717a");
    const QString btnBg    = QStringLiteral("#27272a");
    const QString btnHover = QStringLiteral("#3f3f46");
    const QString searchBoxBg = QStringLiteral("#393a3b");
    const QString sidebarSelectedBg = QStringLiteral("#303132");

    return QStringLiteral(
        "QWidget#appContainer { background-color: %1; }\n"
        "QWidget#rightColumn { background-color: %1; }\n"
        "QFrame#leftSide { background-color: %1; }\n"
        "QWidget#titleBarDrag { background-color: %1; border: none; }\n"
        "QLineEdit#sidebarSearch { background-color: %12; color: %5; selection-background-color: %8; border: none; border-radius: 16px; padding: 0 14px; min-height: 32px; }\n"
        "QLineEdit#sidebarSearch::placeholder { color: %7; }\n"
        "QListWidget#sideList::item { color: %5; }\n"
        "QListWidget#sideList::item:hover { background-color: transparent; }\n"
        "QListWidget#sideList::item:selected { background-color: %13; color: %8; font-weight: 500; }\n"
        "QLabel#sidebarProfileImg { background-color: %3; border: 1px solid %4; border-radius: 20px; }\n"
        "QLabel#sidebarProfileName { color: %5; }\n"
        "QWidget#sidebarProfile[selected=\"true\"] { background-color: %13; border-radius: 10px; padding: 10px 12px; min-height: 24px; }\n"
        "QWidget#sidebarProfile[selected=\"true\"] QLabel#sidebarProfileName { color: %8; font-weight: 500; }\n"
        "QWidget#contentWrapper, QStackedWidget#mainContainer { background-color: %1; }\n"
        "QWidget { color: %5; }\n"
        "QLabel { color: %5; }\n"
        "QPushButton { background-color: %9; color: %5; border: none; }\n"
        "QPushButton:hover { background-color: %10; color: %5; }\n"
        "QPushButton:disabled { background-color: %4; color: %7; }\n"
        "QPushButton#discoverDownloadButton { background-color: #ffffff; color: %8; border: none; border-radius: 14px; min-width: 58px; max-width: 58px; min-height: 28px; max-height: 28px; font-size: 11px; font-weight: 700; padding: 0 6px; }\n"
        "QPushButton#discoverDownloadButton:hover:!disabled { background-color: #e5e7eb; color: %8; }\n"
        "QPushButton#discoverDownloadButton:disabled { background-color: #ffffff; color: %7; }\n"
        "QLineEdit { background-color: %2; color: %5; border: 1px solid %4; selection-background-color: %8; }\n"
        "QLineEdit::placeholder { color: %7; }\n"
        "QComboBox { background-color: %2; color: %5; border: 1px solid %4; }\n"
        "QComboBox QAbstractItemView { background-color: %2; color: %5; selection-background-color: %3; }\n"
        "QWidget[class=\"package-card\"] { background-color: %3; border-color: %1; }\n"
        "QWidget[class=\"package-card\"]:hover, QWidget[class=\"package-card\"][hovered=\"true\"] { background-color: %3; border-color: %4; }\n"
        "QLabel[class=\"status-badge\"] { background-color: %8; color: %11; }\n"
        "QLabel[class=\"repo-badge\"] { background-color: %4; color: %6; }\n"
        "QGroupBox { background-color: %3; border-color: %1; }\n"
        "QGroupBox::title { color: %7; }\n"
        "QMenuBar { background-color: %1; color: %5; }\n"
        "QMenuBar::item:selected { background-color: %3; }\n"
        "QMenu { background-color: %2; color: %5; border: 1px solid %4; }\n"
        "QMenu::item:selected { background-color: %3; }\n"
        "QScrollBar::handle:vertical, QScrollBar::handle:horizontal { background-color: %4; }\n"
        "QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover { background-color: #3a3d4a; }\n"
        "QMessageBox QPushButton { background-color: %9; color: %5; }\n"
        "QMessageBox QPushButton:hover { background-color: %10; }\n"
        "QProgressBar::chunk { background-color: %8; }\n"
        "QCheckBox::indicator:checked { background-color: %8; border-color: %8; }\n"
    ).arg(bg, surface, cardBg, border, text, textMuted, placeholder, accent, btnBg, btnHover, accentText, searchBoxBg, sidebarSelectedBg);
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == qApp && event->type() == QEvent::ApplicationPaletteChange) {
        loadStyleSheet();
        return false;
    }
    if (watched == m_headerWidget && event->type() == QEvent::MouseButtonPress) {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            QWindow* win = windowHandle();
            if (win && win->startSystemMove()) {
                return true;
            }
        }
    }
    if (watched == m_centralWidget) {
        if (event->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(event);
            Qt::Edges edges = resizeEdgesAt(me->pos());
            if (edges != Qt::Edges()) {
                if ((edges & Qt::LeftEdge) && (edges & Qt::TopEdge)) {
                    m_centralWidget->setCursor(Qt::SizeFDiagCursor);
                } else if ((edges & Qt::RightEdge) && (edges & Qt::TopEdge)) {
                    m_centralWidget->setCursor(Qt::SizeBDiagCursor);
                } else if ((edges & Qt::LeftEdge) && (edges & Qt::BottomEdge)) {
                    m_centralWidget->setCursor(Qt::SizeBDiagCursor);
                } else if ((edges & Qt::RightEdge) && (edges & Qt::BottomEdge)) {
                    m_centralWidget->setCursor(Qt::SizeFDiagCursor);
                } else if ((edges & Qt::LeftEdge) || (edges & Qt::RightEdge)) {
                    m_centralWidget->setCursor(Qt::SizeHorCursor);
                } else {
                    m_centralWidget->setCursor(Qt::SizeVerCursor);
                }
                return false;
            }
            m_centralWidget->unsetCursor();
        } else if (event->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                Qt::Edges edges = resizeEdgesAt(me->pos());
                if (edges != Qt::Edges()) {
                    QWindow* win = windowHandle();
                    if (win && win->startSystemResize(edges)) {
                        return true;
                    }
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

Qt::Edges MainWindow::resizeEdgesAt(const QPoint& pos) const {
    if (!m_centralWidget) return Qt::Edges();
    const QRect r = m_centralWidget->rect();
    const int m = RESIZE_MARGIN;
    Qt::Edges edges;
    if (pos.x() <= r.x() + m) edges |= Qt::LeftEdge;
    if (pos.x() >= r.right() - m) edges |= Qt::RightEdge;
    if (pos.y() <= r.y() + m) edges |= Qt::TopEdge;
    if (pos.y() >= r.bottom() - m) edges |= Qt::BottomEdge;
    return edges;
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    if (!m_contentReady) {
        m_contentReady = true;
        runInitialSyncIfNeeded();
        // Afișează UI imediat (Discover din cache); inițializarea repos în background
        finishContentSetup();
        (void)QtConcurrent::run([this]() {
            bool ok = AlpmWrapper::instance().initialize();
            QMetaObject::invokeMethod(this, [this, ok]() {
                if (ok) {
                    if (m_homeWidget) m_homeWidget->refreshWhenRepositoriesReady();
                    if (m_installedWidget) m_installedWidget->refreshPackages();
                } else if (!ok) {
                    QMessageBox::critical(this, tr("Error"),
                        tr("Failed to initialize package manager. Please check your system configuration."));
                }
            }, Qt::QueuedConnection);
        });
    }
    static bool blurDone = false;
    if (!blurDone) {
        blurDone = true;
        enableBlurBehind();
    }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    if (m_reflectionDebounceTimer) {
        m_reflectionDebounceTimer->start(150);
    }
}

void MainWindow::enableBlurBehind() {
    QWindow* win = windowHandle();
    if (!win) return;
#ifdef HAVE_KWINDOW_EFFECTS
    if (KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind)) {
        KWindowEffects::enableBlurBehind(win, true);
        Logger::info("Blur behind enabled (KWindowEffects)");
    }
#else
    Q_UNUSED(win);
#endif
}
