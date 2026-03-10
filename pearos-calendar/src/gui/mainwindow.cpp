#include "mainwindow.h"
#include "../utils/logger.h"
#include <algorithm>
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
#include <QScrollBar>
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
#include <QRegion>
#include <QProcess>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QInputDialog>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTabBar>
#include <QCalendarWidget>
#include <QLocale>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QHeaderView>
#include <QWheelEvent>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QPropertyAnimation>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QGraphicsView>
#ifdef HAVE_KWINDOW_EFFECTS
#include <KWindowEffects>
#endif

namespace {

static QPixmap blurPixmap(const QPixmap& src, int radius) {
    if (src.isNull()) return src;
    QGraphicsBlurEffect* eff = new QGraphicsBlurEffect;
    eff->setBlurRadius(radius);
    QGraphicsScene scene;
    QGraphicsPixmapItem item(src);
    item.setGraphicsEffect(eff);
    scene.addItem(&item);
    QPixmap out(src.size());
    out.fill(Qt::transparent);
    QPainter p(&out);
    scene.render(&p, QRectF(), QRectF(0, 0, src.width(), src.height()));
    return out;
}

class CalendarHeaderOverlay : public QWidget {
public:
    CalendarHeaderOverlay(QScrollArea* scrollArea, QWidget* parent = nullptr)
        : QWidget(parent), m_scrollArea(scrollArea) {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents, false);
        m_bgLabel = new QLabel(this);
        m_bgLabel->setScaledContents(true);
        m_bgLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    void setContentWidgets(QLabel* title, QWidget* weekdayRow) {
        m_title = title;
        m_weekdayRow = weekdayRow;
        if (m_title) m_title->setParent(this);
        if (m_weekdayRow) m_weekdayRow->setParent(this);
        m_title->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        m_weekdayRow->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
    void updateBlur() {
        if (!m_scrollArea || !m_scrollArea->viewport() || !m_bgLabel) return;
        int h = height();
        if (h <= 0) return;
        QWidget* vp = m_scrollArea->viewport();
        QPixmap grabbed = vp->grab(QRect(0, 0, width(), qMin(h, vp->height())));
        if (grabbed.isNull()) return;
        m_bgLabel->setPixmap(blurPixmap(grabbed, 18));
    }
protected:
    void resizeEvent(QResizeEvent* e) override {
        QWidget::resizeEvent(e);
        m_bgLabel->setGeometry(0, 0, width(), height());
        m_bgLabel->lower();
        const int titleH = 36;
        if (m_title) {
            m_title->setGeometry(0, 0, width(), titleH);
            m_title->raise();
        }
        if (m_weekdayRow) {
            m_weekdayRow->setGeometry(0, titleH, width(), 28);
            m_weekdayRow->raise();
        }
        QTimer::singleShot(0, this, [this]() { updateBlur(); });
    }
    void showEvent(QShowEvent* e) override {
        QWidget::showEvent(e);
        QTimer::singleShot(50, this, [this]() { updateBlur(); });
    }
private:
    QScrollArea* m_scrollArea = nullptr;
    QLabel* m_bgLabel = nullptr;
    QLabel* m_title = nullptr;
    QWidget* m_weekdayRow = nullptr;
};

static CalendarHeaderOverlay* s_calendarHeaderOverlay = nullptr;

class CalendarCellDelegate : public QStyledItemDelegate {
public:
    CalendarCellDelegate(QCalendarWidget* calendar, MainWindow* window, bool useRedForToday)
        : m_calendar(calendar), m_window(window), m_useRedForToday(useRedForToday) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        const int row = index.row();
        const int col = index.column();
        const int year = m_calendar->yearShown();
        const int month = m_calendar->monthShown();
        QDate firstDay(year, month, 1);
        const int delta = row * 7 + col - (firstDay.dayOfWeek() - 1);
        const QDate cellDate = firstDay.addDays(delta);

        const QString dayText = index.data(Qt::DisplayRole).toString();
        const QRect r = option.rect;
        const int pad = 4;
        const QRect textRect(r.x(), r.y(), r.width() - pad, r.height() - pad);

        const QDate today = QDate::currentDate();
        const bool isToday = (cellDate == today);
        const bool hasEvent = m_window && m_window->hasEventOn(cellDate);
        int displayedYear = year, displayedMonth = month;
        if (!m_window || !m_window->getDisplayedMonthYear(displayedYear, displayedMonth)) {
            displayedYear = year;
            displayedMonth = month;
        }
        const bool inCurrentMonth = (cellDate.month() == displayedMonth && cellDate.year() == displayedYear);

        if (isToday && inCurrentMonth) {
            painter->setFont(option.font);
            const QFontMetrics fm(option.font);
            const int textW = fm.horizontalAdvance(dayText);
            const int textH = fm.height();
            const int side = qMax(textW, textH) + 8;
            const int x = r.right() - side - pad;
            const int y = r.y() + pad;
            const QRect circleRect(x, y, side, side);

            if (m_useRedForToday) {
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(220, 38, 38));
                painter->drawEllipse(circleRect);
                painter->setPen(Qt::white);
            } else {
                const QColor accent = qApp->palette().color(QPalette::Highlight);
                const QColor accentText = qApp->palette().color(QPalette::HighlightedText);
                painter->setPen(Qt::NoPen);
                painter->setBrush(accent);
                painter->drawEllipse(circleRect);
                painter->setPen(accentText);
            }

            painter->drawText(circleRect, Qt::AlignCenter, dayText);

            if (hasEvent && m_window) {
                const QStringList titles = m_window->eventTitlesOn(cellDate);
                if (!titles.isEmpty()) {
                    const QString labelText = titles.size() > 1 ? (titles.first() + QStringLiteral("…")) : titles.first();
                    const QColor labelBg(0x29, 0x56, 0x40);
                    const QColor labelFg(0x43, 0xd6, 0x93);
                    QFont smallFont = option.font;
                    smallFont.setPointSize(qMax(7, smallFont.pointSize() - 2));
                    painter->setFont(smallFont);
                    QFontMetrics labelFm(smallFont);
                    const int labelH = labelFm.height() + 4;
                    const int labelY = r.bottom() - labelH - 2;
                    const int labelLeft = r.x() + 2;
                    const int labelRight = r.right() - 2;
                    const int maxW = labelRight - labelLeft;
                    const QString elided = labelFm.horizontalAdvance(labelText) > maxW
                        ? labelFm.elidedText(labelText, Qt::ElideRight, maxW)
                        : labelText;
                    const int labelW = qMin(labelFm.horizontalAdvance(elided) + 8, maxW);
                    const QRect labelR(labelLeft, labelY, labelW, labelH);
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(labelBg);
                    painter->drawRoundedRect(labelR, 4, 4);
                    painter->setPen(labelFg);
                    painter->drawText(labelR, Qt::AlignCenter, elided);
                }
            }
            return;
        }

        if (hasEvent && inCurrentMonth && m_window) {
            const QStringList titles = m_window->eventTitlesOn(cellDate);
            if (!titles.isEmpty()) {
                const QString labelText = titles.size() > 1 ? (titles.first() + QStringLiteral("…")) : titles.first();
                const QColor labelBg(0x29, 0x56, 0x40);
                const QColor labelFg(0x43, 0xd6, 0x93);
                QFont smallFont = option.font;
                smallFont.setPointSize(qMax(7, smallFont.pointSize() - 2));
                painter->setFont(smallFont);
                QFontMetrics labelFm(smallFont);
                const int labelH = labelFm.height() + 4;
                const int labelY = r.bottom() - labelH - 2;
                const int labelLeft = r.x() + 2;
                const int labelRight = r.right() - 2;
                const int maxW = labelRight - labelLeft;
                const QString elided = labelFm.horizontalAdvance(labelText) > maxW
                    ? labelFm.elidedText(labelText, Qt::ElideRight, maxW)
                    : labelText;
                const int labelW = qMin(labelFm.horizontalAdvance(elided) + 8, maxW);
                const QRect labelR(labelLeft, labelY, labelW, labelH);
                painter->setPen(Qt::NoPen);
                painter->setBrush(labelBg);
                painter->drawRoundedRect(labelR, 4, 4);
                painter->setPen(labelFg);
                painter->drawText(labelR, Qt::AlignCenter, elided);
            }
        }

        painter->setFont(option.font);
        if (inCurrentMonth)
            painter->setPen(option.palette.color(QPalette::WindowText));
        else
            painter->setPen(option.palette.color(QPalette::PlaceholderText));
        painter->drawText(textRect, Qt::AlignRight | Qt::AlignTop, dayText);
    }

private:
    QCalendarWidget* m_calendar = nullptr;
    MainWindow* m_window = nullptr;
    bool m_useRedForToday = false;
};

} // namespace

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

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("pearOS Calendar");
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
    setupContent();
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
    m_headerWidget->setFixedHeight(32);

    auto* layout = new QHBoxLayout(m_headerWidget);
    layout->setContentsMargins(12, 4, 12, 4);
    layout->setSpacing(8);

    // macOS-style traffic lights
    QWidget* trafficContainer = new QWidget(m_headerWidget);
    auto* trafficLayout = new QHBoxLayout(trafficContainer);
    trafficLayout->setContentsMargins(0, 0, 0, 0);
    trafficLayout->setSpacing(6);

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
    trafficLayout->addWidget(btnClose);

    QPushButton* btnMin = makeTrafficBtn(QStringLiteral("#f8ce52"), QStringLiteral("\u2212"));
    connect(btnMin, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    trafficLayout->addWidget(btnMin);

    QPushButton* btnMax = makeTrafficBtn(QStringLiteral("#5fcf65"), QStringLiteral("\u25A1"));
    connect(btnMax, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) showNormal(); else showMaximized();
    });
    trafficLayout->addWidget(btnMax);

    auto getTrafficLabel = [](QPushButton* b) -> QLabel* {
        return b ? b->property("trafficLabel").value<QLabel*>() : nullptr;
    };

    struct TrafficHoverFilter : QObject {
        QWidget* container = nullptr;
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
            if (!container) return QObject::eventFilter(o, e);
            if (e->type() == QEvent::Enter) {
                setHovered(true);
            } else if (e->type() == QEvent::Leave) {
                QWidget* w = qApp->widgetAt(QCursor::pos());
                bool stillInside = w && (w == container || container->isAncestorOf(w));
                if (!stillInside) setHovered(false);
            }
            return QObject::eventFilter(o, e);
        }
    };

    auto* hoverFilter = new TrafficHoverFilter();
    hoverFilter->container = trafficContainer;
    hoverFilter->bClose = btnClose;
    hoverFilter->bMin = btnMin;
    hoverFilter->bMax = btnMax;
    hoverFilter->lClose = getTrafficLabel(btnClose);
    hoverFilter->lMin = getTrafficLabel(btnMin);
    hoverFilter->lMax = getTrafficLabel(btnMax);
    hoverFilter->setParent(trafficContainer);
    hoverFilter->setHovered(false);
    trafficContainer->installEventFilter(hoverFilter);
    btnClose->installEventFilter(hoverFilter);
    btnMin->installEventFilter(hoverFilter);
    btnMax->installEventFilter(hoverFilter);

    layout->addWidget(trafficContainer, 0);
    m_headerTrafficWidget = trafficContainer;

    // Sidebar toggle button
    m_sidebarToggleButton = new QPushButton(QStringLiteral("Events"), m_headerWidget);
    m_sidebarToggleButton->setCursor(Qt::PointingHandCursor);
    m_sidebarToggleButton->setFlat(true);
    m_sidebarToggleButton->setMinimumWidth(70);
    layout->addWidget(m_sidebarToggleButton, 0);

    // Center tab bar (Day / Week / Month / Year) – hidden
    // QWidget* centerContainer = new QWidget(m_headerWidget);
    // centerContainer->setObjectName("headerCenter");
    // auto* centerLayout = new QHBoxLayout(centerContainer);
    // centerLayout->setContentsMargins(0, 0, 0, 0);
    // centerLayout->setSpacing(0);
    // centerLayout->addStretch(1);
    // m_tabBar = new QTabBar(centerContainer);
    // m_tabBar->setExpanding(false);
    // m_tabBar->setDrawBase(false);
    // m_tabBar->addTab(tr("Day"));
    // m_tabBar->addTab(tr("Week"));
    // m_tabBar->addTab(tr("Month"));
    // m_tabBar->addTab(tr("Year"));
    // m_tabBar->setCurrentIndex(2);
    // centerLayout->addWidget(m_tabBar);
    // centerLayout->addStretch(1);
    // centerContainer->setCursor(Qt::SizeAllCursor);
    // layout->addWidget(centerContainer, 1);
    layout->addStretch(1);

    m_headerWidget->setStyleSheet("QWidget#titleBarDrag { background: transparent; }");
}

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
    sideLayout->setContentsMargins(12, 12, 12, 20);
    sideLayout->setSpacing(8);

    // Traffic-light buttons live primarily in the sidebar
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

    QPushButton* sbClose = makeTrafficBtn(QStringLiteral("#f96057"), QStringLiteral("\u00D7"));
    connect(sbClose, &QPushButton::clicked, this, &QMainWindow::close);
    circlesLayout->addWidget(sbClose);
    QPushButton* sbMin = makeTrafficBtn(QStringLiteral("#f8ce52"), QStringLiteral("\u2212"));
    connect(sbMin, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    circlesLayout->addWidget(sbMin);
    QPushButton* sbMax = makeTrafficBtn(QStringLiteral("#5fcf65"), QStringLiteral("\u25A1"));
    connect(sbMax, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) showNormal(); else showMaximized();
    });
    circlesLayout->addWidget(sbMax);

    // Top row: traffic lights + Events button (Events visible when sidebar open, like traffic)
    QWidget* topSidebarRow = new QWidget(m_leftSideWidget);
    auto* topSidebarLayout = new QHBoxLayout(topSidebarRow);
    topSidebarLayout->setContentsMargins(0, 0, 0, 0);
    topSidebarLayout->setSpacing(8);
    topSidebarLayout->addWidget(menuCircles);
    m_sidebarEventsButton = new QPushButton(QStringLiteral("Events"), this);
    m_sidebarEventsButton->setCursor(Qt::PointingHandCursor);
    m_sidebarEventsButton->setFlat(true);
    m_sidebarEventsButton->setMinimumWidth(70);
    topSidebarLayout->addWidget(m_sidebarEventsButton, 0);
    topSidebarLayout->addStretch(1);
    sideLayout->addWidget(topSidebarRow);
    m_sidebarTrafficWidget = menuCircles;

    QLabel* header = new QLabel(tr("Events"), m_leftSideWidget);
    QFont headerFont = header->font();
    headerFont.setPointSize(11);
    headerFont.setWeight(QFont::DemiBold);
    header->setFont(headerFont);
    header->setStyleSheet("color: #a1a1aa;");
    sideLayout->addWidget(header);

    m_sidebarSearch = new QLineEdit(this);
    m_sidebarSearch->setObjectName("sidebarSearch");
    m_sidebarSearch->setCursor(Qt::IBeamCursor);
    m_sidebarSearch->setPlaceholderText(tr("Search events"));
    m_sidebarSearch->setMinimumHeight(32);
    m_sidebarSearch->setClearButtonEnabled(true);
    sideLayout->addWidget(m_sidebarSearch);

    m_sideList = new QListWidget(this);
    m_sideList->setObjectName("sideList");
    m_sideList->setCursor(Qt::ArrowCursor);
    m_sideList->setSpacing(4);
    m_sideList->setContextMenuPolicy(Qt::CustomContextMenu);
    sideLayout->addWidget(m_sideList, 1);

    connect(m_sideList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (!item || !m_monthScrollArea || m_monthCalendars.isEmpty())
            return;
        const QString id = item->data(Qt::UserRole).toString();
        auto it = std::find_if(m_events.cbegin(), m_events.cend(), [&id](const CalendarEvent& e) { return e.id == id; });
        if (it == m_events.cend())
            return;
        const QDate date = it->date;
        if (m_tabBar)
            m_tabBar->setCurrentIndex(2);
        if (m_viewStack)
            m_viewStack->setCurrentIndex(2);
        for (QCalendarWidget* cal : m_monthCalendars) {
            if (cal->yearShown() == date.year() && cal->monthShown() == date.month()) {
                m_monthScrollArea->ensureWidgetVisible(cal, 0, 80);
                m_monthCalendar = cal;
                if (m_monthTitleLabel)
                    m_monthTitleLabel->setText(date.toString(QStringLiteral("MMMM yyyy")));
                for (QCalendarWidget* c : m_monthCalendars) {
                    if (QTableView* tv = c->findChild<QTableView*>())
                        tv->viewport()->update();
                }
                if (s_calendarHeaderOverlay)
                    s_calendarHeaderOverlay->updateBlur();
                break;
            }
        }
    });

    connect(m_sideList, &QListWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QListWidgetItem* item = m_sideList->itemAt(pos);
        if (!item)
            return;
        const QString id = item->data(Qt::UserRole).toString();
        QMenu menu(this);
        QAction* delAct = menu.addAction(tr("Delete"));
        connect(delAct, &QAction::triggered, this, [this, id]() {
            m_events.erase(std::remove_if(m_events.begin(), m_events.end(),
                [&id](const CalendarEvent& e) { return e.id == id; }), m_events.end());
            saveEvents();
            refreshSidebarEvents();
            updateCalendarHighlights();
        });
        menu.exec(m_sideList->mapToGlobal(pos));
    });

    connect(m_sidebarSearch, &QLineEdit::textChanged, this, [this](const QString& text) {
        const QString query = text.trimmed().toLower();
        for (int i = 0; i < m_sideList->count(); ++i) {
            QListWidgetItem* item = m_sideList->item(i);
            const bool match = query.isEmpty() ||
                item->text().toLower().contains(query);
            item->setHidden(!match);
        }
    });

    // Mini calendar for current month at bottom
    m_miniCalendar = new QCalendarWidget(this);
    m_miniCalendar->setObjectName(QStringLiteral("miniCalendar"));
    m_miniCalendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_miniCalendar->setNavigationBarVisible(false);
    m_miniCalendar->setMaximumHeight(180);
    m_miniCalendar->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    m_miniCalendar->setGridVisible(false);
    m_miniCalendar->setSelectionMode(QCalendarWidget::NoSelection);
    m_miniCalendar->setFocusPolicy(Qt::NoFocus);
    m_miniCalendar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_miniCalendar->setDateRange(QDate(1900, 1, 1), QDate(2100, 12, 31));
    m_miniCalendar->setCurrentPage(QDate::currentDate().year(), QDate::currentDate().month());
    m_miniCalendar->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // Monday first
    m_miniCalendar->setFirstDayOfWeek(Qt::Monday);
    QFont miniFont = m_miniCalendar->font();
    miniFont.setPointSize(qMax(7, miniFont.pointSize() - 3));
    m_miniCalendar->setFont(miniFont);
    if (QTableView* miniView = m_miniCalendar->findChild<QTableView*>()) {
        miniView->clearSelection();
        miniView->setItemDelegate(new CalendarCellDelegate(m_miniCalendar, this, true));
        if (QHeaderView* hh = miniView->horizontalHeader())
            hh->setMaximumHeight(22);
    }
    sideLayout->addWidget(m_miniCalendar);

    // User profile at bottom (round photo + name only)
    QWidget* profileWidget = new QWidget(m_leftSideWidget);
    profileWidget->setObjectName("sidebarProfile");
    auto* profileLayout = new QHBoxLayout(profileWidget);
    profileLayout->setContentsMargins(0, 8, 0, 0);
    profileLayout->setSpacing(10);

    const int faceSize = 40;
    QLabel* profileImg = new QLabel(profileWidget);
    profileImg->setObjectName("sidebarProfileImg");
    profileImg->setFixedSize(faceSize, faceSize);
    profileImg->setScaledContents(true);
    QString facePath = QDir::homePath() + QStringLiteral("/.face.icon");
    if (!QFile::exists(facePath))
        facePath = QDir::homePath() + QStringLiteral("/.face");
    if (QFile::exists(facePath)) {
        QPixmap pm(facePath);
        if (!pm.isNull()) {
            pm = pm.scaled(faceSize, faceSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            profileImg->setPixmap(pm);
        }
    }
    profileImg->setMask(QRegion(0, 0, faceSize, faceSize, QRegion::Ellipse));
    profileLayout->addWidget(profileImg, 0);

    m_sidebarProfileName = new QLabel(tr("Unknown"), profileWidget);
    m_sidebarProfileName->setObjectName("sidebarProfileName");
    m_sidebarProfileName->setStyleSheet("color: #e5e5e5;");
    m_sidebarProfileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    profileLayout->addWidget(m_sidebarProfileName, 1);

    sideLayout->addWidget(profileWidget);

    loadPearIdDisplayName();
}

void MainWindow::loadPearIdDisplayName() {
    if (!m_sidebarProfileName)
        return;

    QString scriptPath;
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList candidates = {
        appDir.absoluteFilePath(QStringLiteral("../pearID/get_user_info.sh")),
        appDir.absoluteFilePath(QStringLiteral("pearID/get_user_info.sh")),
        QStringLiteral("/usr/share/pearos-appstore/pearID/get_user_info.sh"),
        QStringLiteral("/usr/share/pearos-calendar/pearID/get_user_info.sh")
    };
    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            scriptPath = path;
            break;
        }
    }
    if (scriptPath.isEmpty())
        return;

    QProcess* process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        if (exitCode != 0 || exitStatus != QProcess::NormalExit || !m_sidebarProfileName)
            return;
        const QByteArray out = process->readAllStandardOutput().trimmed();
        const QList<QByteArray> lines = out.split('\n');
        QString first = QString::fromUtf8(lines.value(0).trimmed());
        QString last = QString::fromUtf8(lines.value(1).trimmed());
        if (!first.isEmpty() || !last.isEmpty()) {
            m_sidebarProfileName->setText((first + QChar(' ') + last).trimmed());
        }
    });
    process->start(scriptPath,
                   {QStringLiteral("--first-name"), QStringLiteral("--last-name")},
                   QProcess::ReadOnly);
}

void MainWindow::setupContent() {
    if (!m_contentLayout) return;

    // Day view placeholder
    QWidget* dayView = new QWidget(this);
    auto* dayLayout = new QVBoxLayout(dayView);
    dayLayout->setContentsMargins(0, 0, 0, 0);
    dayLayout->addWidget(new QLabel(tr("Day view coming soon"), dayView), 0, Qt::AlignCenter);

    // Week view placeholder
    QWidget* weekView = new QWidget(this);
    auto* weekLayout = new QVBoxLayout(weekView);
    weekLayout->setContentsMargins(0, 0, 0, 0);
    weekLayout->addWidget(new QLabel(tr("Week view coming soon"), weekView), 0, Qt::AlignCenter);

    // Month view: scroll area full size, header overlay on top with blur
    QWidget* monthView = new QWidget(this);
    auto* monthLayout = new QVBoxLayout(monthView);
    monthLayout->setContentsMargins(0, 0, 0, 0);

    QLocale loc(QLocale::English, QLocale::UnitedKingdom);
    m_monthTitleLabel = new QLabel(tr("Month YYYY"), monthView);
    QFont monthFont = m_monthTitleLabel->font();
    monthFont.setBold(true);
    monthFont.setPointSize(monthFont.pointSize() + 2);
    m_monthTitleLabel->setFont(monthFont);
    m_monthTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_monthTitleLabel->setStyleSheet("color: #f5f5f5; padding: 0 4px 6px 4px; background: transparent;");

    QWidget* weekdayHeaderRow = new QWidget(monthView);
    weekdayHeaderRow->setFixedHeight(28);
    weekdayHeaderRow->setStyleSheet("background: transparent;");
    auto* headerLayout = new QHBoxLayout(weekdayHeaderRow);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);
    for (int d = 1; d <= 7; ++d) {
        QLabel* dayLbl = new QLabel(loc.standaloneDayName(d, QLocale::ShortFormat), weekdayHeaderRow);
        dayLbl->setAlignment(Qt::AlignCenter);
        dayLbl->setStyleSheet("color: #a1a1aa; font-weight: 600; background: transparent;");
        headerLayout->addWidget(dayLbl, 1);
    }

    m_monthScrollArea = new QScrollArea(monthView);
    m_monthScrollArea->setWidgetResizable(true);
    m_monthScrollArea->setFrameShape(QFrame::NoFrame);
    m_monthScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_monthScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_monthScrollArea->setFocusPolicy(Qt::NoFocus);

    QWidget* scrollContent = new QWidget(m_monthScrollArea);
    auto* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(16);

    const int startYear = 1980;
    const int endYear = 2040;
    m_monthCalendars.clear();
    m_monthCalendars.reserve((endYear - startYear + 1) * 12);
    int currentMonthIndex = -1;
    const QDate today = QDate::currentDate();

    for (int y = startYear; y <= endYear; ++y) {
        for (int month = 1; month <= 12; ++month) {
        QDate monthDate(y, month, 1);
            if (monthDate.year() == today.year() && monthDate.month() == today.month())
                currentMonthIndex = m_monthCalendars.size();

            QCalendarWidget* cal = new QCalendarWidget(scrollContent);
            cal->setLocale(loc);
            cal->setFirstDayOfWeek(Qt::Monday);
            cal->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
            cal->setHorizontalHeaderFormat(QCalendarWidget::NoHorizontalHeader);
            cal->setGridVisible(true);
            cal->setNavigationBarVisible(false);
            cal->setSelectionMode(QCalendarWidget::NoSelection);
            cal->setCurrentPage(y, month);
            cal->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            cal->setMinimumHeight(380);
            if (QTableView* tv = cal->findChild<QTableView*>()) {
                tv->setSelectionMode(QAbstractItemView::NoSelection);
                tv->setItemDelegate(new CalendarCellDelegate(cal, this, true));
                tv->clearSelection();
                if (QHeaderView* vh = tv->verticalHeader())
                    vh->setDefaultSectionSize(48);
                if (QHeaderView* hh = tv->horizontalHeader())
                    hh->setDefaultSectionSize(72);
            }
            scrollLayout->addWidget(cal);
            m_monthCalendars.append(cal);
            cal->installEventFilter(this);
            if (QTableView* tv = cal->findChild<QTableView*>()) {
                tv->viewport()->installEventFilter(this);
                connect(tv, &QAbstractItemView::doubleClicked, this, [this, cal](const QModelIndex& index) {
                    const int row = index.row();
                    const int col = index.column();
                    const int year = cal->yearShown();
                    const int month = cal->monthShown();
                    QDate firstDay(year, month, 1);
                    const int delta = row * 7 + col - (firstDay.dayOfWeek() - 1);
                    const QDate cellDate = firstDay.addDays(delta);
                    if (cellDate.isValid())
                        openEventDialogForDate(cellDate);
                });
            }
        }
    }

    m_monthCalendar = (currentMonthIndex >= 0 && currentMonthIndex < m_monthCalendars.size())
        ? m_monthCalendars[currentMonthIndex]
        : m_monthCalendars.value(0);
    m_monthScrollArea->setWidget(scrollContent);

    if (QScrollBar* vsb = m_monthScrollArea->verticalScrollBar()) {
        vsb->setSingleStep(24);
        vsb->setPageStep(m_monthScrollArea->viewport()->height() / 2);
        connect(vsb, &QScrollBar::valueChanged, this, [this](int value) {
            if (!m_monthTitleLabel || m_monthCalendars.isEmpty()) return;
            QWidget* content = m_monthScrollArea->widget();
            if (!content) return;
            const int threshold = m_monthScrollArea->viewport() ? m_monthScrollArea->viewport()->height() / 2 : 200;
            const int v = value + threshold;
            QCalendarWidget* first = m_monthCalendars.constFirst();
            int firstTop = first->mapTo(content, QPoint(0, 0)).y();
            if (v < firstTop) {
                m_monthTitleLabel->setText(QDate(first->yearShown(), first->monthShown(), 1).toString(QStringLiteral("MMMM yyyy")));
                if (m_monthCalendar != first) {
                    m_monthCalendar = first;
                    for (QCalendarWidget* cal : m_monthCalendars) {
                        if (QTableView* tv = cal->findChild<QTableView*>())
                            tv->viewport()->update();
                    }
                }
            } else {
                for (QCalendarWidget* cal : m_monthCalendars) {
                    int top = cal->mapTo(content, QPoint(0, 0)).y();
                    if (v >= top && v < top + cal->height()) {
                        QDate d(cal->yearShown(), cal->monthShown(), 1);
                        m_monthTitleLabel->setText(d.toString(QStringLiteral("MMMM yyyy")));
                        if (m_monthCalendar != cal) {
                            m_monthCalendar = cal;
                            for (QCalendarWidget* c : m_monthCalendars) {
                                if (QTableView* tv = c->findChild<QTableView*>())
                                    tv->viewport()->update();
                            }
                        }
                        break;
                    }
                }
            }
            if (s_calendarHeaderOverlay)
                s_calendarHeaderOverlay->updateBlur();
        });
    }
    monthLayout->addWidget(m_monthScrollArea, 1);

    const int headerOverlayHeight = 64;
    CalendarHeaderOverlay* overlay = new CalendarHeaderOverlay(m_monthScrollArea, monthView);
    s_calendarHeaderOverlay = overlay;
    overlay->setObjectName(QStringLiteral("calendarHeaderOverlay"));
    overlay->setContentWidgets(m_monthTitleLabel, weekdayHeaderRow);
    overlay->setFixedHeight(headerOverlayHeight);
    overlay->setGeometry(0, 0, monthView->width(), headerOverlayHeight);
    overlay->raise();
    monthView->installEventFilter(this);

    QTimer::singleShot(80, this, [this, currentMonthIndex]() {
        if (m_monthScrollArea && currentMonthIndex >= 0 && currentMonthIndex < m_monthCalendars.size() && m_monthCalendars[currentMonthIndex]) {
            int y = m_monthCalendars[currentMonthIndex]->y();
            if (QScrollBar* vsb = m_monthScrollArea->verticalScrollBar())
                vsb->setValue(qMax(0, y - 8));
        }
    });

    // Year view placeholder
    QWidget* yearView = new QWidget(this);
    auto* yearLayout = new QVBoxLayout(yearView);
    yearLayout->setContentsMargins(0, 0, 0, 0);
    yearLayout->addWidget(new QLabel(tr("Year view coming soon"), yearView), 0, Qt::AlignCenter);

    m_viewStack = new QStackedWidget(this);
    m_viewStack->addWidget(dayView);   // index 0
    m_viewStack->addWidget(weekView);  // index 1
    m_viewStack->addWidget(monthView); // index 2
    m_viewStack->addWidget(yearView);  // index 3
    m_viewStack->setCurrentIndex(2);   // Month by default

    m_contentLayout->addWidget(m_viewStack, 1);

    // Events
    loadEvents();
    refreshSidebarEvents();
    updateCalendarHighlights();

    if (m_monthTitleLabel && m_monthCalendar) {
        QDate d(m_monthCalendar->yearShown(), m_monthCalendar->monthShown(), 1);
        m_monthTitleLabel->setText(d.toString(QStringLiteral("MMMM yyyy")));
    }

    // Header tab bar <-> stacked views (tabs hidden)
    // if (m_tabBar && m_viewStack) {
    //     connect(m_tabBar, &QTabBar::currentChanged, this, [this](int idx) {
    //         if (m_viewStack && idx >= 0 && idx < m_viewStack->count())
    //             m_viewStack->setCurrentIndex(idx);
    //     });
    //     m_tabBar->setCurrentIndex(2);
    //     m_viewStack->setCurrentIndex(2);
    // }
    if (m_viewStack)
        m_viewStack->setCurrentIndex(2);

    auto doSidebarToggle = [this]() {
        if (!m_leftSideWidget)
            return;
        if (!m_sidebarAnimation) {
            m_sidebarAnimation = new QPropertyAnimation(m_leftSideWidget, "maximumWidth", this);
            m_sidebarAnimation->setDuration(220);
            m_sidebarAnimation->setEasingCurve(QEasingCurve::InOutCubic);
        }
        m_sidebarVisible = !m_sidebarVisible;
        int start = m_leftSideWidget->maximumWidth();
        int end = m_sidebarVisible ? 240 : 0;
        if (!m_sidebarVisible)
            m_leftSideWidget->setMinimumWidth(0);
        m_sidebarAnimation->stop();
        m_sidebarAnimation->setStartValue(start);
        m_sidebarAnimation->setEndValue(end);
        m_sidebarAnimation->start();

        if (m_headerTrafficWidget)
            m_headerTrafficWidget->setVisible(!m_sidebarVisible);
        if (m_sidebarTrafficWidget)
            m_sidebarTrafficWidget->setVisible(m_sidebarVisible);
        if (m_sidebarToggleButton)
            m_sidebarToggleButton->setVisible(!m_sidebarVisible);
        if (m_sidebarEventsButton)
            m_sidebarEventsButton->setVisible(m_sidebarVisible);
    };

    if (m_sidebarToggleButton)
        connect(m_sidebarToggleButton, &QPushButton::clicked, this, doSidebarToggle);
    if (m_sidebarEventsButton)
        connect(m_sidebarEventsButton, &QPushButton::clicked, this, doSidebarToggle);

    // Start with sidebar hidden
    if (m_leftSideWidget) {
        m_leftSideWidget->setMaximumWidth(0);
        m_leftSideWidget->setMinimumWidth(0);
        m_sidebarVisible = false;
    }
    if (m_headerTrafficWidget)
        m_headerTrafficWidget->setVisible(true);
    if (m_sidebarTrafficWidget)
        m_sidebarTrafficWidget->setVisible(false);
    if (m_sidebarToggleButton)
        m_sidebarToggleButton->setVisible(true);
    if (m_sidebarEventsButton)
        m_sidebarEventsButton->setVisible(false);
}

void MainWindow::loadEvents() {
    m_events.clear();
    const QString dirPath = QStringLiteral("/usr/share/extras/pearos-calendar");
    QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }
    QFile file(dir.filePath(QStringLiteral("events.json")));
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return;

    const QJsonArray arr = doc.object().value(QStringLiteral("events")).toArray();
    for (const QJsonValue& v : arr) {
        const QJsonObject o = v.toObject();
        CalendarEvent ev;
        ev.id = o.value(QStringLiteral("id")).toString();
        ev.title = o.value(QStringLiteral("title")).toString();
        ev.details = o.value(QStringLiteral("details")).toString();
        const QString dateStr = o.value(QStringLiteral("date")).toString();
        ev.date = QDate::fromString(dateStr, Qt::ISODate);
        if (ev.date.isValid() && !ev.title.isEmpty())
            m_events.append(ev);
    }
}

void MainWindow::saveEvents() const {
    const QString dirPath = QStringLiteral("/usr/share/extras/pearos-calendar");
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!QDir().mkpath(dirPath)) {
            Logger::error(QStringLiteral("Failed to create events storage directory: %1").arg(dirPath));
            return;
        }
    }

    QJsonArray arr;
    for (const CalendarEvent& ev : m_events) {
        if (!ev.date.isValid() || ev.title.trimmed().isEmpty())
            continue;
        QJsonObject o;
        o.insert(QStringLiteral("id"), ev.id);
        o.insert(QStringLiteral("title"), ev.title);
        o.insert(QStringLiteral("details"), ev.details);
        o.insert(QStringLiteral("date"), ev.date.toString(Qt::ISODate));
        arr.append(o);
    }

    QJsonObject root;
    root.insert(QStringLiteral("events"), arr);
    const QJsonDocument doc(root);

    const QString filePath = dir.filePath(QStringLiteral("events.json"));
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        Logger::error(QStringLiteral("Failed to open events file for writing: %1").arg(filePath));
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void MainWindow::refreshSidebarEvents() {
    if (!m_sideList)
        return;

    QSignalBlocker blocker(m_sideList);
    m_sideList->clear();

    if (m_events.isEmpty())
        return;

    const QDate today = QDate::currentDate();

    // Simple sort by date
    QVector<CalendarEvent> events = m_events;
    std::sort(events.begin(), events.end(), [](const CalendarEvent& a, const CalendarEvent& b) {
        if (a.date == b.date)
            return a.title.toLower() < b.title.toLower();
        return a.date < b.date;
    });

    for (const CalendarEvent& ev : events) {
        QString category;
        if (ev.date < today)
            category = QStringLiteral("Past");
        else if (ev.date == today)
            category = QStringLiteral("Today");
        else
            category = QStringLiteral("Upcoming");

        const QString text = QStringLiteral("%1 • %2 • %3")
                                 .arg(ev.date.toString(Qt::ISODate), category, ev.title);
        auto* item = new QListWidgetItem(text, m_sideList);
        item->setData(Qt::UserRole, ev.id);
        m_sideList->addItem(item);
    }

    updateMiniCalendar();
}

bool MainWindow::hasEventOn(const QDate& date) const {
    if (!date.isValid())
        return false;
    for (const CalendarEvent& ev : m_events) {
        if (ev.date == date)
            return true;
    }
    return false;
}

QStringList MainWindow::eventTitlesOn(const QDate& date) const {
    QStringList out;
    if (!date.isValid()) return out;
    for (const CalendarEvent& ev : m_events) {
        if (ev.date == date && !ev.title.trimmed().isEmpty())
            out.append(ev.title.trimmed());
    }
    return out;
}

bool MainWindow::getDisplayedMonthYear(int& year, int& month) const {
    if (!m_monthCalendar) return false;
    year = m_monthCalendar->yearShown();
    month = m_monthCalendar->monthShown();
    return true;
}

void MainWindow::updateCalendarHighlights() {
    for (QCalendarWidget* cal : m_monthCalendars) {
        if (!cal) continue;
        cal->setDateTextFormat(QDate(), QTextCharFormat());
        if (QTableView* view = cal->findChild<QTableView*>())
            view->viewport()->update();
    }
}

void MainWindow::updateMiniCalendar() {
    if (!m_miniCalendar)
        return;

    const QDate today = QDate::currentDate();
    m_miniCalendar->setCurrentPage(today.year(), today.month());
    m_miniCalendar->setDateTextFormat(QDate(), QTextCharFormat());
    if (QTableView* view = m_miniCalendar->findChild<QTableView*>())
        view->viewport()->update();
}

void MainWindow::openEventDialogForDate(const QDate& date) {
    if (!date.isValid())
        return;

    int editIndex = -1;
    for (int i = 0; i < m_events.size(); ++i) {
        if (m_events[i].date == date) {
            editIndex = i;
            break;
        }
    }
    const bool isEdit = (editIndex >= 0);

    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? tr("Edit event") : tr("Add event"));
    auto* layout = new QVBoxLayout(&dlg);

    auto* titleEdit = new QLineEdit(&dlg);
    titleEdit->setPlaceholderText(tr("Title"));
    auto* detailsEdit = new QTextEdit(&dlg);
    detailsEdit->setPlaceholderText(tr("Details"));

    if (isEdit) {
        titleEdit->setText(m_events[editIndex].title);
        detailsEdit->setPlainText(m_events[editIndex].details);
    }

    layout->addWidget(new QLabel(date.toString(Qt::ISODate), &dlg));
    layout->addWidget(titleEdit);
    layout->addWidget(detailsEdit, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString title = titleEdit->text().trimmed();
    const QString details = detailsEdit->toPlainText();
    if (title.isEmpty())
        return;

    if (isEdit) {
        m_events[editIndex].title = title;
        m_events[editIndex].details = details;
    } else {
        CalendarEvent ev;
        ev.date = date;
        ev.title = title;
        ev.details = details;
        ev.id = QStringLiteral("%1-%2")
                    .arg(date.toString(Qt::ISODate),
                         QString::number(QRandomGenerator::global()->generate(), 16));
        m_events.append(ev);
    }
    saveEvents();
    refreshSidebarEvents();
    updateCalendarHighlights();
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
    auto* quitAction = new QAction("&Quit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(quitAction);

    auto* helpMenu = menuBar()->addMenu("&Help");
    auto* aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, tr("About pearOS Calendar"),
            tr("pearOS Calendar\n\n"
               "A simple calendar application for pearOS\n"
               "Built with Qt6 and C++17."));
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::loadStyleSheet() {
    QFile styleFile(":/stylesheet.qss");
    if (!styleFile.exists()) styleFile.setFileName("stylesheet.qss");
    if (!styleFile.exists()) styleFile.setFileName("/usr/share/pearos-calendar/stylesheet.qss");

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
        "QWidget#titleBarDrag { background-color: rgba(30, 33, 35, 0.82); border: none; }\n"
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
        "QCalendarWidget { background-color: %1; border: none; }\n"
        "QCalendarWidget QWidget { background-color: transparent; }\n"
        "QCalendarWidget QAbstractItemView { background-color: %1; border: none; gridline-color: %4; }\n"
        "QCalendarWidget QAbstractItemView::item:selected { background: transparent; color: %5; }\n"
        "QCalendarWidget#miniCalendar QAbstractItemView::item { padding: 0px 1px; margin: 0px; }\n"
    ).arg(bg, surface, cardBg, border, text, textMuted, placeholder, accent, btnBg, btnHover, accentText, searchBoxBg, sidebarSelectedBg);
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == qApp && event->type() == QEvent::ApplicationPaletteChange) {
        loadStyleSheet();
        return false;
    }
    if (m_viewStack && watched == m_viewStack->widget(2) && event->type() == QEvent::Resize) {
        QWidget* monthView = static_cast<QWidget*>(watched);
        QWidget* ov = monthView->findChild<QWidget*>(QStringLiteral("calendarHeaderOverlay"));
        if (ov) ov->setGeometry(0, 0, monthView->width(), ov->height());
        if (s_calendarHeaderOverlay) s_calendarHeaderOverlay->updateBlur();
        return false;
    }
    if (event->type() == QEvent::Wheel && m_monthScrollArea && !m_monthCalendars.isEmpty()) {
        QWidget* w = qobject_cast<QWidget*>(watched);
        for (QCalendarWidget* cal : m_monthCalendars) {
            if (watched == cal || (w && cal->isAncestorOf(w))) {
                auto* we = static_cast<QWheelEvent*>(event);
                if (QScrollBar* vsb = m_monthScrollArea->verticalScrollBar()) {
                    vsb->setValue(vsb->value() - we->angleDelta().y());
                    return true;
                }
                break;
            }
        }
    }
    if (event->type() == QEvent::MouseButtonDblClick && m_monthCalendars.isEmpty() == false) {
        QWidget* viewport = qobject_cast<QWidget*>(watched);
        if (viewport) {
            for (QCalendarWidget* cal : m_monthCalendars) {
                QTableView* tv = cal->findChild<QTableView*>();
                if (!tv || tv->viewport() != viewport)
                    continue;
                auto* me = static_cast<QMouseEvent*>(event);
                const QModelIndex index = tv->indexAt(me->pos());
                if (!index.isValid())
                    break;
                const int row = index.row();
                const int col = index.column();
                const int year = cal->yearShown();
                const int month = cal->monthShown();
                QDate firstDay(year, month, 1);
                const int delta = row * 7 + col - (firstDay.dayOfWeek() - 1);
                const QDate cellDate = firstDay.addDays(delta);
                if (cellDate.isValid()) {
                    openEventDialogForDate(cellDate);
                    return true;
                }
                break;
            }
        }
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
