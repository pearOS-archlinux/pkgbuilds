/*

    Copyright (C) 2013  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "mainwindow.h"
#include "filer_titlebar.h"
#include "filer_sidebar.h"
#include "titlebar.h"
#include "welcome.h"

#include <QApplication>
#include <QToolBar>
#include <QSplitter>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QMouseEvent>
#include <QWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QScrollBar>
#include <QCoreApplication>
#include <QFile>
#include <QVariantAnimation>
#include <QTimer>
#include <QRegion>
#include <QGraphicsColorizeEffect>
#include <QStackedWidget>
#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QDialog>
#include <QFont>
#include <QIcon>
#include <QPixmap>
#include <QProcess>
#include <QTextBrowser>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QFrame>
#include <QSize>
#include <QRegularExpression>
#include <functional>
#include <KWindowEffects>
#include <QTextDocument>
#include <QEventLoop>
#include <QImage>
#include <QHash>
#include <QSet>
#include <memory>

namespace {

// Same blendTint() lambda as MainWindow::paintEvent() (src/mainwindow.cpp:3588-3593).
QColor blendTint(const QColor& base, const QColor& tint, qreal amount) {
    return QColor::fromRgbF(
        base.redF()   + (tint.redF()   - base.redF())   * amount,
        base.greenF() + (tint.greenF() - base.greenF()) * amount,
        base.blueF()  + (tint.blueF()  - base.blueF())  * amount);
}

// Ported from ../mainwindow.h's AboutUsDialog -- kept local to this file
// (not exposed via mainwindow.h) since nothing outside setupWelcomeContent()
// needs it.
class AboutUsDialog : public QDialog {
public:
    explicit AboutUsDialog(MainWindow* parent) : QDialog(parent) {
        // Static pill, no drag/bounce -- both relied on QWidget::move(),
        // which is a no-op on real Wayland (xdg-shell gives the compositor
        // sole ownership of a top-level's position; client repositioning
        // requests are simply ignored). Worked in dev testing only because
        // that ran forced under XWayland (QT_QPA_PLATFORM=xcb). Moving it
        // for real would need a KWin script + D-Bus relay like
        // WindowGeometryRelay already does for the main window (KWin's own
        // scripting API *can* reposition arbitrary windows) -- not done
        // here, so it just stays centered.
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setWindowTitle(QStringLiteral("Test Liquid Gel Live"));
        static const int kWidth = 260;
        static const int kHeight = 60;
        setFixedSize(kWidth, kHeight);
        setModal(true);
        setAttribute(Qt::WA_TranslucentBackground);

        QLabel* imageLabel = new QLabel(this);
        QImage logo(get_base_path() + QStringLiteral("/assets/move_me.png"));
        if (!logo.isNull()) {
            // Source art is black line-work on a transparent background --
            // always inverted to white line-work now (this pill has no
            // fill of its own, so it's sitting directly over the desktop).
            logo = logo.convertToFormat(QImage::Format_ARGB32);
            logo.invertPixels(QImage::InvertRgb);
            imageLabel->setPixmap(QPixmap::fromImage(logo).scaledToHeight(32, Qt::SmoothTransformation));
        }
        imageLabel->setAlignment(Qt::AlignCenter);

        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addStretch();
        layout->addWidget(imageLabel);
        layout->addStretch();

        // Positioned as a free-floating child (not in the centering layout)
        // so the image above stays truly centered instead of being skewed
        // by the close button's own width on one side.
        QPushButton* closeBtn = new QPushButton(QStringLiteral("×"), this); // ×
        closeBtn->setFixedSize(24, 24);
        closeBtn->move(kWidth - 24 - 8, (kHeight - 24) / 2);
        closeBtn->setFocusPolicy(Qt::NoFocus);
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setStyleSheet(QStringLiteral(
            "QPushButton { border: none; border-radius: 12px; background-color: rgba(255, 255, 255, 20);"
            " color: rgba(255, 255, 255, 0.85); font-size: 15px; font-weight: bold; }"
            "QPushButton:hover { background-color: #e74c3c; color: white; }"));
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
        closeBtn->raise();
    }

protected:
    // Painted by hand, not QSS -- see MainWindow's own paintEvent()/loadSettings()
    // comment above: stylesheet background/border/border-radius rules on a
    // frameless top-level widget don't reliably paint in this Qt5/style
    // setup, same reason MainWindow doesn't rely on QSS for its own rounded
    // corners either. No fill -- just the outline, so the pill itself stays
    // fully see-through and only the image/close-button/outline show.
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        p.setPen(QPen(QColor(0, 0, 0, 140), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r, r.height() / 2, r.height() / 2);
    }
};

// QListWidget::setViewportMargins() is protected -- this exists only to
// expose it, so the sidebar's row 0 sits kToolBarHeight px down at rest
// (the floating toolbar's own height, matching the Filer-shell Sidebar
// class's identical trick in sidebar.cpp) as a real scroll-through viewport
// inset instead of static layout space.
class NavSidebar : public QListWidget {
public:
    explicit NavSidebar(int toolBarHeight) {
        setViewportMargins(0, toolBarHeight, 0, 0);
    }
};

}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
    , desktopEnv_(get_desktop_environment())
    , isLiveIso_(check_if_live_iso()) {
    setWindowTitle(QStringLiteral("pearOS welcome"));

    // Same directory-resolution trick as the top-level Welcome app's
    // get_base_path(): prefer the directory the executable actually runs
    // from (build dir, where CMake copies assets/ alongside it), fall back
    // to the source tree's ../assets/ so `./mainwindow` also works run
    // in-place from qt5/build without a copy step.
    QString exeDir = QCoreApplication::applicationDirPath();
    assetsBasePath_ = exeDir;
    if (!QFile::exists(assetsBasePath_ + "/assets/titlebuttons")) {
        assetsBasePath_ = exeDir + "/..";
    }
    if (!QFile::exists(assetsBasePath_ + "/assets/titlebuttons")) {
        assetsBasePath_ = QStringLiteral("/usr/share/pearos-welcome");
    }

    loadSettings();

    // pearOS Welcome's real stylesheet (button/checkbox/label/sidebar
    // colors) -- ported from ../mainwindow.h's loadStylesheet(). Loaded
    // before setWindowFlags()/setupWelcomeContent() below so every widget
    // those build already picks it up on construction, same order as the
    // real app.
    //
    // The real Welcome app has no custom paintEvent() at all -- its whole
    // frosted look comes from styles.qss's own QMainWindow/#centralwidget/
    // #topbar/#sidebar/#contentArea background-color rules plus KWin's
    // blur-behind on a translucent window. This shell already has a much
    // more capable version of that same effect (real wallpaper-color
    // sampling, tint that reacts to window position, a real KWin
    // blur-behind region -- see paintEvent()/updateSidebarBlurRegion()), so
    // loading those specific rules verbatim doesn't add anything -- it just
    // paints a near-opaque rgba(30,30,31,200) wash on top of (or under,
    // depending on Qt's stylesheet paint order) what paintEvent() already
    // draws, washing the frost/tint out into the flat, muted look this was
    // reported as ("Welcome trebuia sa fie ca in Filer" -- Filer has no such
    // QSS rules to fight its own paintEvent() at all). Everything else in
    // the file (buttons, checkboxes, labels, breadcrumb, scrollbars) still
    // applies normally -- only the handful of rules that duplicate this
    // shell's own window/toolbar/sidebar/content-area background painting
    // are stripped, and only for this process; the shared file itself is
    // untouched, so the real Welcome app's own look is unaffected.
    {
        QString qssPath = assetsBasePath_ + QStringLiteral("/styles.qss");
        QFile f(qssPath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString qss = QString::fromUtf8(f.readAll());
            // QSS url(...) paths resolve against the process's working
            // directory, not styles.qss's own location -- rewrite the
            // switch-icon references (see "CheckBox as Switch" in
            // styles.qss) to an absolute path so they still load regardless
            // of where this binary is launched from.
            qss.replace(QStringLiteral("url(assets/"),
                        QStringLiteral("url(") + assetsBasePath_ + QStringLiteral("/assets/"));
            static const QStringList kConflictingSelectors = {
                QStringLiteral("QMainWindow"),
                QStringLiteral("QWidget#centralwidget"),
                QStringLiteral("QToolBar#topbar"),
                QStringLiteral("QListWidget#sidebar"),
                QStringLiteral("QStackedWidget#contentArea"),
            };
            for (const QString& selector : kConflictingSelectors) {
                QRegularExpression re(QRegularExpression::escape(selector) + QStringLiteral("\\s*\\{[^}]*\\}"));
                qss.remove(re);
            }
            setStyleSheet(qss);
        }
    }

    // Frameless, self-drawn chrome -- see mainwindow.h for exactly which
    // lines of Filer's real MainWindow::MainWindow() (src/mainwindow.cpp:
    // 538-736) this reproduces.
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // Reserves the kShadowMargin_ band the self-drawn drop shadow paints
    // into (see paintEvent()) -- widened from Filer's own 5,0,5,5 resize-grip
    // margin (src/mainwindow.cpp:569), which this shell never wired up
    // resize-grip handling for anyway. Collapses to 0,0,0,0 while maximized
    // -- see changeEvent().
    setContentsMargins(kShadowMargin, 0, kShadowMargin, kShadowMargin);

    QWidget* central = new QWidget(this);
    central->setAttribute(Qt::WA_TranslucentBackground);
    setCentralWidget(central);
    QVBoxLayout* rootLayout = new QVBoxLayout(central);
    rootLayout->setSpacing(0);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    // pearOS Welcome's nav sidebar (Install & Setup / Donate / Community /
    // More Options / What's New? / About Us) -- not Filer's places list, so
    // built directly here instead of via the Sidebar class (sidebar.h/.cpp),
    // reusing only its stylesheet formula (Fm::buildSidePaneStyleSheet(),
    // same as ../mainwindow.h's setupUi()). No fixed width: the splitter
    // controls it below (setSizes()), same reasoning as the Filer-shell
    // sidebar's own resize fix.
    sidebar_ = new NavSidebar(kToolBarHeight);
    sidebar_->setObjectName(QStringLiteral("sidebar"));
    sidebar_->setMinimumWidth(1);
    sidebar_->setFrameShape(QFrame::NoFrame);
    sidebar_->setIconSize(QSize(16, 16));
    sidebar_->setFocusPolicy(Qt::NoFocus);
    sidebar_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sidebar_->setAttribute(Qt::WA_TranslucentBackground);
    {
        QString qss = Fm::buildSidePaneStyleSheet(1, 10, 500, QColor(66, 66, 66), 255);
        qss += QStringLiteral("QListWidget { background: transparent; border: none; }");
        sidebar_->setStyleSheet(qss);
    }

    // Body: sidebar + Welcome content pages, in a QSplitter with a
    // zero-width handle -- ui.splitter->setHandleWidth(0)
    // (src/mainwindow.cpp:672-673). Spans the *entire* central widget, from
    // y=0 -- unlike a QMainWindow toolbar-area layout, nothing here reserves
    // toolbar space at the top; buildTitleBar()'s floating overlay (below)
    // is what actually leaves room for itself, by inset-scrolling instead
    // of a static margin (see updateToolbarOverlayGeometry()'s doc comment
    // for why a static margin was deliberately rejected in the real app).
    splitter_ = new QSplitter(Qt::Horizontal, central);
    splitter_->setHandleWidth(0);
    splitter_->setChildrenCollapsible(false);
    splitter_->addWidget(sidebar_);

    // Content area: pearOS Welcome's page stack (Install & Setup / Donate /
    // Community / More Options), switched by the sidebar above -- built in
    // setupWelcomeContent() below. Translucent so MainWindow::paintEvent()'s
    // own mainAreaTintColor() fill (painted behind this, on the window
    // itself) still shows through around the pages' own widgets, exactly
    // like Filer's real content area (src/mainwindow.cpp:3618).
    content_ = new QWidget(splitter_);
    content_->setAttribute(Qt::WA_TranslucentBackground);
    splitter_->addWidget(content_);
    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);
    splitter_->setSizes({170, 1});

    rootLayout->addWidget(splitter_);

    // Unified/vibrancy toolbar -- built as a floating overlay from the
    // start (never handed to addToolBar()/QMainWindow's own managed
    // toolbar area at all), so the sidebar/content above never lose any
    // space to it in the first place. Ported from the ctor's
    // removeToolBar()/setParent()/raise() block (src/mainwindow.cpp:
    // 1181-1192) -- skipping the "add it, then immediately pull it back
    // out" round trip, since this project has no other docked toolbar
    // state to preserve either way.
    toolBar_ = new Titlebar(
        assetsBasePath_ + QStringLiteral("/assets"), kToolBarHeight,
        [this]() { close(); },
        [this]() { showMinimized(); },
        [this]() { isMaximized() ? showNormal() : showMaximized(); },
        this);
    toolBar_->installEventFilter(this);
    trafficLights_ = static_cast<Titlebar*>(toolBar_)->trafficLights();

    // Logo + app name (left, right after the traffic lights) and OS version
    // (right-aligned, past the toolbar's own expanding spacer) -- ported
    // from ../mainwindow.h's setupUi() topbar block.
    {
        Titlebar* tb = static_cast<Titlebar*>(toolBar_);
        QString logoPath = assetsBasePath_ + QStringLiteral("/assets/welcome.png");
        if (QFile::exists(logoPath)) {
            QLabel* logoLabel = new QLabel();
            QPixmap pix(logoPath);
            logoLabel->setPixmap(pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            tb->insertBeforeSpacer(logoLabel);
        }
        QLabel* topbarTitle = new QLabel(QStringLiteral("pearOS NiceC0re"));
        topbarTitle->setObjectName(QStringLiteral("topbarTitle"));
        tb->insertBeforeSpacer(topbarTitle);
        QLabel* topbarVersion = new QLabel(get_os_version());
        topbarVersion->setObjectName(QStringLiteral("topbarVersion"));
        tb->addWidget(topbarVersion);
    }

    toolBar_->show();
    toolBar_->raise();
    updateToolbarOverlayGeometry();

    // Window-outline stroke overlay -- a raised sibling kept above every
    // other child so the 1px outline always wins the paint order, instead
    // of getting covered by whichever other raised overlay happened to
    // come last. Ported from the ctor (src/mainwindow.cpp:1236-1241).
    windowStrokeOverlay_ = new QWidget(this);
    windowStrokeOverlay_->setAttribute(Qt::WA_TranslucentBackground);
    windowStrokeOverlay_->setAttribute(Qt::WA_TransparentForMouseEvents);
    windowStrokeOverlay_->installEventFilter(this);
    windowStrokeOverlay_->show();
    updateWindowStrokeOverlayGeometry();

    // Toolbar hairline: invisible at rest, fades in on a 1.5s titlebar
    // hover, fades back out once the pointer leaves -- ported from the ctor
    // (src/mainwindow.cpp:1248-1266). Filer also fires this off scrolling
    // the current folder view (applyScrollInsets()' scroll-value polling);
    // there's no folder view here to scroll, so hover is the only trigger.
    toolbarHairlineAnim_ = new QVariantAnimation(this);
    toolbarHairlineAnim_->setDuration(200);
    connect(toolbarHairlineAnim_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        toolbarHairlineOpacity_ = value.toReal();
        update();
    });
    toolbarHairlineHoverTimer_ = new QTimer(this);
    toolbarHairlineHoverTimer_->setSingleShot(true);
    toolbarHairlineHoverTimer_->setInterval(1500);
    connect(toolbarHairlineHoverTimer_, &QTimer::timeout, this, [this]() { animateToolbarHairlineTo(1.0); });
    toolbarHairlineIdleTimer_ = new QTimer(this);
    toolbarHairlineIdleTimer_->setSingleShot(true);
    toolbarHairlineIdleTimer_->setInterval(1000);
    connect(toolbarHairlineIdleTimer_, &QTimer::timeout, this, [this]() { animateToolbarHairlineTo(0.0); });

    // See the member's own doc comment in mainwindow.h: moveEvent() alone
    // can't fix the black-after-drag bug on Wayland, since no moveEvent
    // fires there at all during a compositor-driven startSystemMove().
    repaintPumpTimer_ = new QTimer(this);
    connect(repaintPumpTimer_, &QTimer::timeout, this, [this]() { update(); });
    repaintPumpTimer_->start(200);

    setupWelcomeContent();

    // ../mainwindow.h's own WINDOW_WIDTH/WINDOW_HEIGHT (real Welcome is a
    // small fixed-size utility window, not a resizable file-manager one) --
    // plus room for the shadow margin, same reasoning as the Filer-shell's
    // own resize() call. Widened past the real app's 600 -- this shell's
    // window is resizable, not fixed-size, and 600 left the What's New?
    // page cramped.
    resize(800 + 2 * kShadowMargin, 560 + kShadowMargin);
}

// Ported from ../mainwindow.h's MainWindow::setupUi() -- the sidebar nav
// items + the four native content pages (Install & Setup / Donate /
// Community / More Options) + the breadcrumb label in the status bar.
// "What's New?" opens the real site in the default browser (open_url())
// instead of the real app's QWebEngineView window -- see welcome.h's header
// comment for why. sidebar_/content_/splitter_ themselves are already built
// by the ctor above; this only populates their contents.
void MainWindow::setupWelcomeContent() {
    const QString basePath = assetsBasePath_;

    auto addButton = [this, basePath](const QString& label, const QString& iconName, bool fromFile,
                                       std::function<void()> slot) {
        QPushButton* btn = new QPushButton(label);
        btn->setFocusPolicy(Qt::NoFocus);
        if (fromFile) {
            QString path = basePath + QStringLiteral("/") + iconName;
            if (QFile::exists(path)) {
                btn->setIcon(QIcon(path));
                btn->setIconSize(QSize(20, 20));
            }
        } else {
            btn->setIcon(QIcon::fromTheme(iconName));
        }
        btn->setLayoutDirection(Qt::RightToLeft);
        connect(btn, &QPushButton::clicked, this, [slot]() { slot(); });
        return btn;
    };

    auto addNavItem = [this](const QString& text, const QString& iconName) {
        QListWidgetItem* item = new QListWidgetItem(QIcon::fromTheme(iconName), text);
        sidebar_->addItem(item);
        return item;
    };
    auto addNavSeparator = [this](const QString& text) {
        QListWidgetItem* item = new QListWidgetItem(text.toUpper());
        item->setFlags(Qt::NoItemFlags);
        QFont f = item->font();
        f.setPixelSize(10);
        f.setBold(true);
        f.setLetterSpacing(QFont::PercentageSpacing, 105);
        item->setFont(f);
        item->setForeground(QColor(255, 255, 255, 90));
        sidebar_->addItem(item);
    };

    addNavItem(tr("Install & Setup"), QStringLiteral("system-software-install"));
    addNavItem(tr("Donate"), QStringLiteral("emblem-favorite"));
    addNavItem(tr("Community"), QStringLiteral("system-users"));
    addNavItem(tr("More Options"), QStringLiteral("preferences-other"));
    addNavSeparator(tr("MORE"));
    addNavItem(tr("What's New?"), QStringLiteral("help-about"));
    addNavItem(tr("Test Liquid Gel Live"), QStringLiteral("help-about"));

    // Top margin reserves the floating toolbar's own height -- unlike the
    // sidebar (which insets via a real scroll-through viewport margin, see
    // NavSidebar), the content pages aren't a scroll area, so a static
    // layout margin is the equivalent fix here: without it, row 0 of
    // whichever page is showing starts at y=0 and sits partly hidden under
    // toolBar_'s floating overlay (see updateToolbarOverlayGeometry()).
    QVBoxLayout* contentLayout = new QVBoxLayout(content_);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, kToolBarHeight, 0, 0);
    pageStack_ = new QStackedWidget();
    pageStack_->setObjectName(QStringLiteral("contentArea"));
    contentLayout->addWidget(pageStack_, 1);

    // Page: Install & Setup
    {
        QWidget* page = new QWidget();
        QVBoxLayout* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(20, 16, 20, 16);
        QGridLayout* grid1 = new QGridLayout();
        grid1->setSpacing(10);

        int row = 0, col = 0;
        auto place = [&](QPushButton* btn) {
            grid1->addWidget(btn, row, col);
            if (++col == 2) { col = 0; ++row; }
        };
        if (isLiveIso_) {
            QPushButton* installBtn = addButton(tr("Install pearOS "), QStringLiteral("system-software-install"), false,
                                                 [this]() { run_bin_install(); });
            // The installer takes a moment to actually open a window --
            // disable the button for a bit after click so impatient clicks
            // don't spawn it more than once.
            connect(installBtn, &QPushButton::clicked, this, [installBtn]() {
                installBtn->setEnabled(false);
                QTimer::singleShot(20000, installBtn, [installBtn]() { installBtn->setEnabled(true); });
            });
            place(installBtn);
        }
        place(addButton(tr("Screen Resolution "), QStringLiteral("video-display"), false,
                         [this]() { screen_resolution(desktopEnv_); }));
        place(addButton(tr("Update System "), QStringLiteral("system-software-update"), false,
                         [this]() { update_system(desktopEnv_); }));
        place(addButton(tr("Update Mirrorlist "), QStringLiteral("view-refresh"), false, []() {
            QProcess::startDetached(QStringLiteral("pkexec"),
                QStringList() << QStringLiteral("pacman-mirrors") << QStringLiteral("-f") << QStringLiteral("5"));
        }));
        place(addButton(tr("Fix Liquid Gel "), QStringLiteral("view-refresh"), false,
                         [this]() { fix_liquid_gel_after_upgrade(desktopEnv_); }));
        place(addButton(tr("Fix Pacman Keys "), QStringLiteral("dialog-password"), false,
                         [this]() { fix_pacman_keys(desktopEnv_); }));
        place(addButton(tr("Fix Layout "), QStringLiteral("preferences-desktop"), false, [this]() {
            QStringList failed;
            bool ok = fix_layout(&failed);
            if (ok)
                QMessageBox::information(this, tr("Fix Layout"), tr("Layout restored successfully."));
            else
                QMessageBox::warning(this, tr("Fix Layout"),
                    tr("Some files could not be copied:\n") + failed.join(QLatin1Char('\n')));
        }));
        if (!isLiveIso_)
            place(addButton(tr("YouTube "), QStringLiteral("assets/youtube.svg"), true,
                             []() { open_url(QStringLiteral("https://youtube.com/pearOS")); }));

        pageLayout->addLayout(grid1);
        pageLayout->addStretch();
        pageStack_->addWidget(page);
    }

    // Page: Donate
    {
        QWidget* page = new QWidget();
        QVBoxLayout* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(20, 16, 20, 16);
        QGridLayout* gridDonate = new QGridLayout();
        gridDonate->setSpacing(10);
        gridDonate->addWidget(addButton(tr("PayPal"), QStringLiteral("assets/paypal.svg"), true,
            []() { open_url(QStringLiteral("https://paypal.me/alxb421")); }), 0, 0);
        gridDonate->addWidget(addButton(tr("GitHub Sponsors"), QStringLiteral("assets/github.svg"), true,
            []() { open_url(QStringLiteral("https://github.com/sponsors/pearOS-archlinux?o=esb")); }), 0, 1);
        gridDonate->addWidget(addButton(tr("10 eur/mo"), QStringLiteral("assets/patreon.png"), true,
            []() { open_url(QStringLiteral("https://www.patreon.com/checkout/PearSoftwareandServices?rid=27197016")); }), 1, 0);
        pageLayout->addLayout(gridDonate);
        pageLayout->addStretch();
        pageStack_->addWidget(page);
    }

    // Page: Community / Social Media
    {
        QWidget* page = new QWidget();
        QVBoxLayout* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(20, 16, 20, 16);
        QGridLayout* gridSocial = new QGridLayout();
        gridSocial->setSpacing(10);
        gridSocial->addWidget(addButton(tr("GitHub  "), QStringLiteral("assets/github.svg"), true,
            []() { open_url(QStringLiteral("https://github.com/pearOS-archlinux")); }), 0, 0);
        gridSocial->addWidget(addButton(tr("Discord "), QStringLiteral("assets/discord.svg"), true,
            []() { open_url(QStringLiteral("https://discord.gg/pearos-697456171631509515")); }), 0, 1);
        pageLayout->addLayout(gridSocial);
        pageLayout->addStretch();
        pageStack_->addWidget(page);
    }

    // Page: More Options
    {
        QWidget* page = new QWidget();
        QVBoxLayout* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(20, 16, 20, 16);

        QWidget* autostartWidget = new QWidget();
        QHBoxLayout* ah = new QHBoxLayout(autostartWidget);
        ah->setContentsMargins(0, 0, 0, 0);
        ah->setSpacing(5);
        ah->addWidget(new QLabel(tr("AutoStart:")));
        autostartSwitch_ = new QCheckBox();
        autostartSwitch_->setFocusPolicy(Qt::NoFocus);
        autostartSwitch_->setChecked(autostart_file_exists());
        connect(autostartSwitch_, &QCheckBox::toggled, this, [](bool checked) { toggle_autostart(checked); });
        ah->addStretch();
        ah->addWidget(autostartSwitch_);
        pageLayout->addWidget(autostartWidget);

        pageLayout->addStretch();
        pageStack_->addWidget(page);
    }

    // Page: What's New? -- plain QTextBrowser instead of a full
    // QWebEngineView (see mainwindow.h's setupWelcomeContent() comment):
    // renders the fetched page's HTML with no JS/CSS/remote-image loading,
    // but stays in-window instead of shelling out to the default browser.
    {
        QWidget* page = new QWidget();
        QVBoxLayout* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(0, 0, 0, 0);
        whatsNewBrowser_ = new QTextBrowser();
        whatsNewBrowser_->setOpenExternalLinks(true);
        whatsNewBrowser_->setText(tr("Loading..."));
        pageLayout->addWidget(whatsNewBrowser_);
        pageStack_->addWidget(page);
    }

    networkManager_ = new QNetworkAccessManager(this);

    // Breadcrumb bar: lives in the real QMainWindow status bar, at the very
    // bottom of the window -- same place Filer parents its own
    // breadcrumbBar_ (`breadcrumbBar_ = new QWidget(ui.statusbar);` in
    // mainwindow.cpp), not a strip under the topbar.
    breadcrumbLabel_ = new QLabel(QStringLiteral("Welcome  ›  Install & Setup"));
    breadcrumbLabel_->setObjectName(QStringLiteral("breadcrumbLabel"));
    statusBar()->setObjectName(QStringLiteral("breadcrumbBar"));
    statusBar()->setFixedHeight(32);
    statusBar()->setSizeGripEnabled(false);
    statusBar()->addWidget(breadcrumbLabel_);

    static const char* kBreadcrumbNames[] = { "Install & Setup", "Donate", "Community", "More Options" };
    connect(sidebar_, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row < 0) return;
        if (row < 4) {
            pageStack_->setCurrentIndex(row);
            breadcrumbLabel_->setText(QStringLiteral("Welcome  ›  %1").arg(QLatin1String(kBreadcrumbNames[row])));
        } else if (row == 5) {
            pageStack_->setCurrentIndex(4);
            breadcrumbLabel_->setText(QStringLiteral("Welcome  ›  What's New?"));
            loadWhatsNewPage();
        } else if (row == 6) {
            AboutUsDialog dlg(this);
            dlg.exec();
        }
    });
    sidebar_->setCurrentRow(0);
}

void MainWindow::loadWhatsNewPage() {
    if (whatsNewLoaded_) return;
    whatsNewLoaded_ = true;

    const QUrl pageUrl(QStringLiteral("https://new.pearos.xyz/"));
    QNetworkReply* reply = networkManager_->get(QNetworkRequest(pageUrl));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            whatsNewLoaded_ = false; // allow a retry on next visit
            whatsNewBrowser_->setText(tr("Couldn't load page: %1").arg(reply->errorString()));
            return;
        }

        const QUrl baseUrl = reply->url();
        const QString html = QString::fromUtf8(reply->readAll());

        // Fetch every <img src="..."> up front and cache it on the
        // document *before* calling setHtml(). QTextDocument's own
        // loadResource() only ever resolves qrc:/file: images and returns
        // nothing for http(s) ones -- an earlier attempt overrode it to
        // block with a nested QEventLoop per missing image, but that's
        // called synchronously from inside text layout and re-entered
        // through Qt's own event processing (mouse-move -> layout again ->
        // another nested loop...), blowing the stack. Prefetching avoids
        // ever calling loadResource() for a network URL at all.
        QSet<QUrl> imageUrls;
        QRegularExpression imgRe(QStringLiteral("<img[^>]+src=[\"']([^\"']+)[\"']"),
                                  QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator it = imgRe.globalMatch(html);
        while (it.hasNext()) {
            QUrl url = baseUrl.resolved(QUrl(it.next().captured(1)));
            if (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https"))
                imageUrls.insert(url);
        }

        if (imageUrls.isEmpty()) {
            whatsNewBrowser_->document()->setBaseUrl(baseUrl);
            whatsNewBrowser_->setHtml(html);
            return;
        }

        auto pending = std::make_shared<int>(imageUrls.size());
        for (const QUrl& url : qAsConst(imageUrls)) {
            QNetworkReply* imgReply = networkManager_->get(QNetworkRequest(url));
            connect(imgReply, &QNetworkReply::finished, this, [this, imgReply, url, pending, html, baseUrl]() {
                imgReply->deleteLater();
                if (imgReply->error() == QNetworkReply::NoError) {
                    QImage image;
                    if (image.loadFromData(imgReply->readAll()))
                        whatsNewBrowser_->document()->addResource(QTextDocument::ImageResource, url, image);
                }
                if (--(*pending) == 0) {
                    whatsNewBrowser_->document()->setBaseUrl(baseUrl);
                    whatsNewBrowser_->setHtml(html);
                }
            });
        }
    });
}

void MainWindow::loadSettings() {
    // Real Filer config location -- Settings::profileDir()
    // (src/settings.cpp:175-201) resolves to
    // $XDG_CONFIG_HOME/filer/<profile>/settings.conf, profile "default" for
    // the normal desktop session. No Settings class is linked here (it pulls
    // in glib/libfm-qt transitively through Fm::FolderConfig et al.) -- just
    // a plain QSettings pointed at the same file.
    QString configHome = qEnvironmentVariable("XDG_CONFIG_HOME");
    if (configHome.isEmpty())
        configHome = QDir::homePath() + QStringLiteral("/.config");
    const QString configPath = configHome + QStringLiteral("/filer/default/settings.conf");

    QSettings settings(configPath, QSettings::IniFormat);

    // Defaults below match Settings' own ctor defaults (src/settings.cpp:
    // 79-120), so this looks the same as a fresh Filer profile even if the
    // config file is missing entirely.
    settings.beginGroup(QStringLiteral("Window"));
    tintWindow_ = settings.value(QStringLiteral("TintWindow"), tintWindow_ ? 1 : 0).toInt() != 0;
    sidebarTint_ = qBound(0, settings.value(QStringLiteral("SidebarTint"), sidebarTint_).toInt(), 100);
    mainWindowTint_ = qBound(0, settings.value(QStringLiteral("MainWindowTint"), mainWindowTint_).toInt(), 100);
    transparency_ = settings.value(QStringLiteral("Transparency"), transparency_ ? 1 : 0).toInt() != 0;
    transparencyPower_ = qBound(0, settings.value(QStringLiteral("TransparencyPower"), transparencyPower_).toInt(), 100);
    settings.endGroup();

    // [Desktop] group -- feeds loadWallpaperFromConfig()/
    // updateSidebarWallpaperTint(), the source data DesktopWindow itself
    // would otherwise be reading (see desktopwindow.cpp:479/410).
    settings.beginGroup(QStringLiteral("Desktop"));
    wallpaperFile_ = settings.value(QStringLiteral("Wallpaper")).toString();
    const QString modeStr = settings.value(QStringLiteral("WallpaperMode"), QStringLiteral("stretch")).toString();
    if (modeStr == QStringLiteral("none"))
        wallpaperMode_ = 0;
    else if (modeStr == QStringLiteral("transparent"))
        wallpaperMode_ = 1;
    else if (modeStr == QStringLiteral("fit"))
        wallpaperMode_ = 3;
    else if (modeStr == QStringLiteral("center"))
        wallpaperMode_ = 4;
    else if (modeStr == QStringLiteral("tile"))
        wallpaperMode_ = 5;
    else
        wallpaperMode_ = 2; // stretch
    bgColor_ = QColor(settings.value(QStringLiteral("BgColor"), bgColor_.name()).toString());
    settings.endGroup();
}

QString MainWindow::findPlasmaWallpaperPath(const QScreen* screen) const {
    QFile f(QDir::homePath() + QStringLiteral("/.config/plasma-org.kde.plasma.desktop-appletsrc"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    const int targetScreenIndex = screen ? QGuiApplication::screens().indexOf(const_cast<QScreen*>(screen)) : -1;

    // KConfig's nested groups are flat section headers like
    // "[Containments][141][Wallpaper][org.kde.image][General]", not real
    // QSettings '/'-nested groups -- track the current containment ID and
    // its lastScreen=/Image= by hand instead of fighting QSettings' group
    // model against a format it doesn't understand.
    static const QRegularExpression containmentRe(QStringLiteral("^\\[Containments\\]\\[(\\d+)\\]$"));
    static const QRegularExpression wallpaperGroupRe(QStringLiteral("^\\[Containments\\]\\[(\\d+)\\]\\[Wallpaper\\]"));

    QHash<int, int> containmentScreen;   // containment id -> lastScreen
    QHash<int, QString> containmentImage; // containment id -> Image= path
    int currentContainment = -1;
    bool inWallpaperGroup = false;

    while (!f.atEnd()) {
        QString line = QString::fromUtf8(f.readLine()).trimmed();
        if (line.startsWith(QLatin1Char('['))) {
            QRegularExpressionMatch m = containmentRe.match(line);
            if (m.hasMatch()) {
                currentContainment = m.captured(1).toInt();
                inWallpaperGroup = false;
                continue;
            }
            m = wallpaperGroupRe.match(line);
            inWallpaperGroup = m.hasMatch() && m.captured(1).toInt() == currentContainment;
            continue;
        }
        if (currentContainment < 0)
            continue;
        if (line.startsWith(QStringLiteral("lastScreen=")))
            containmentScreen[currentContainment] = line.mid(11).toInt();
        else if (inWallpaperGroup && line.startsWith(QStringLiteral("Image=")))
            containmentImage[currentContainment] = line.mid(6);
    }

    QString path;
    for (auto it = containmentImage.constBegin(); it != containmentImage.constEnd(); ++it) {
        if (containmentScreen.value(it.key(), -1) == targetScreenIndex) {
            path = it.value();
            break;
        }
    }
    if (path.isEmpty() && !containmentImage.isEmpty())
        path = containmentImage.constBegin().value(); // best-effort fallback

    if (path.startsWith(QStringLiteral("file://")))
        path.remove(0, 7);
    return path;
}

void MainWindow::loadWallpaperFromConfig(const QScreen* screen) {
    // Mirrors DesktopWindow::updateWallpaper() (src/desktopwindow.cpp:
    // 563-593), minus the on-disk scaled-image cache (that's an optimization
    // for repeated desktop-session startups, not needed for a single
    // in-memory sample here) and the Center/Tile QPixmap(wallpaperFile_)
    // load-at-native-size behavior, kept as-is since it's already just a
    // plain file load.
    wallpaperPixmap_ = QPixmap();
    wallpaperScreen_ = screen;
    if (!screen || wallpaperMode_ == 0 /* None */ || wallpaperMode_ == 1 /* Transparent */)
        return;

    // Prefer the real Plasma desktop wallpaper over Filer's own [Desktop]
    // Wallpaper config key -- see findPlasmaWallpaperPath()'s doc comment
    // in mainwindow.h for why that key is normally empty here.
    QString file = findPlasmaWallpaperPath(screen);
    if (file.isEmpty())
        file = wallpaperFile_;
    if (file.isEmpty())
        return;

    const QSize screenSize = screen->geometry().size();
    switch (wallpaperMode_) {
    case 2: { // Stretch -- ignores aspect ratio, fills the whole screen
        QImage image(file);
        if (!image.isNull())
            wallpaperPixmap_ = QPixmap::fromImage(
                image.scaled(screenSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        break;
    }
    case 3: { // Fit -- keeps aspect ratio
        QImage image(file);
        if (!image.isNull())
            wallpaperPixmap_ = QPixmap::fromImage(
                image.scaled(screenSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        break;
    }
    case 4: // Center
    case 5: // Tile
        wallpaperPixmap_ = QPixmap(file);
        break;
    default:
        break;
    }
}

void MainWindow::updateSidebarWallpaperTint(const QRect& windowGlobalRect) {
    // Ported from MainWindow::updateSidebarWallpaperTint()
    // (src/mainwindow.cpp:2902-2929) -- same sidebar-anchor-not-window-center
    // reasoning, same tiny-fluctuation damping, sampling loadWallpaperFromConfig()'s
    // pixmap (built straight from the Filer config) instead of a live
    // DesktopWindow.
    //
    // windowGlobalRect is a real screen-coordinate frame geometry -- this
    // shell is Wayland-only, and a Wayland client can't query its own
    // global position itself, so it's always relayed here by
    // WindowGeometryRelay, fed by the KWin script
    // (kwin-scripts/filer-shell-qt5-window-tracker.js) the same way real
    // Filer's WindowGeometryTracker works on Wayland
    // (src/windowgeometrytracker.h) -- see loadWindowTrackerKwinScript()
    // in main.cpp.
    if (!tintWindow_)
        return;
    const QPoint sidebarAnchor(windowGlobalRect.left(), windowGlobalRect.center().y());
    QScreen* screen = QGuiApplication::screenAt(sidebarAnchor);
    if (screen != wallpaperScreen_ || wallpaperPixmap_.isNull())
        loadWallpaperFromConfig(screen);

    QColor tint(28, 28, 28);
    if (screen && !wallpaperPixmap_.isNull()) {
        // sidebarColumnRight() returns 0 with the sidebar_ commented out
        // (see the ctor), which used to make this an empty/negative-width
        // sample rect -- permanently falling back to bgColor_ (a fixed,
        // non-moving color) regardless of window position. Sample the
        // whole window width instead whenever there's no sidebar column to
        // restrict it to.
        const int colRight = sidebarColumnRight();
        const int sampleWidth = colRight > 0 ? colRight : windowGlobalRect.width();
        const QPoint originOffset = windowGlobalRect.topLeft() - screen->geometry().topLeft();
        const QRect sampleRect(originOffset, QSize(sampleWidth, windowGlobalRect.height()));
        const QRect clipped = sampleRect.intersected(wallpaperPixmap_.rect());
        if (!clipped.isEmpty()) {
            // Downsample the sampled region to a single pixel -- a cheap
            // average-color extraction (bilinear filtering during the scale
            // does the averaging), same trick as
            // DesktopWindow::averageColorForRect().
            QImage onePixel = wallpaperPixmap_.copy(clipped).toImage()
                .scaled(1, 1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            tint = onePixel.pixelColor(0, 0);
        } else {
            tint = bgColor_;
        }
    } else if (screen) {
        tint = bgColor_;
    }

    auto channelDelta = [](int a, int b) { return qAbs(a - b); };
    if (channelDelta(tint.red(), fixedTint_.red()) > 4 ||
        channelDelta(tint.green(), fixedTint_.green()) > 4 ||
        channelDelta(tint.blue(), fixedTint_.blue()) > 4) {
        fixedTint_ = tint;
        update();
    }
}

QColor MainWindow::sampleWallpaperColor(const QRect& globalRect) {
    QScreen* screen = QGuiApplication::screenAt(globalRect.center());
    if (!screen)
        return bgColor_;
    if (screen != wallpaperScreen_ || wallpaperPixmap_.isNull())
        loadWallpaperFromConfig(screen);
    if (wallpaperPixmap_.isNull())
        return bgColor_;

    const QPoint originOffset = globalRect.topLeft() - screen->geometry().topLeft();
    const QRect sampleRect(originOffset, globalRect.size());
    const QRect clipped = sampleRect.intersected(wallpaperPixmap_.rect());
    if (clipped.isEmpty())
        return bgColor_;

    // Same downsample-to-1px average-color trick as updateSidebarWallpaperTint().
    QImage onePixel = wallpaperPixmap_.copy(clipped).toImage()
        .scaled(1, 1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return onePixel.pixelColor(0, 0);
}

int MainWindow::sidebarColumnRight() const {
    // Ported from the file-local sidebarColumnRight() helper
    // (src/mainwindow.cpp:2871-2875).
    if (!sidebar_ || !sidebar_->isVisible())
        return 0;
    return sidebar_->mapTo(const_cast<MainWindow*>(this), QPoint(sidebar_->width(), 0)).x();
}

QColor MainWindow::mainAreaTintColor() const {
    // Ported from MainWindow::mainAreaTintColor() (src/mainwindow.cpp:3549-3557).
    const qreal mainWindowTintAmount = tintWindow_ ? mainWindowTint_ / 100.0 : 0.0;
    const QColor base = palette().color(QPalette::Window);
    return blendTint(base, fixedTint_, mainWindowTintAmount);
}

void MainWindow::updateToolbarOverlayGeometry() {
    // Ported from MainWindow::updateToolbarOverlayGeometry()
    // (src/mainwindow.cpp:2953-2991), minus the ui.frame/ui.tabBar margin
    // reservation -- no tab bar in this project, so nothing else needs a
    // static top margin at all (the sidebar gets its own scroll-through
    // inset instead, see Sidebar's ctor in sidebar.cpp).
    if (!toolBar_)
        return;
    int h = toolBar_->sizeHint().height();
    if (h <= 0)
        h = kToolBarHeight;
    const QRect cr = contentsRect();
    toolBar_->setGeometry(cr.left(), cr.top(), cr.width(), h);
    toolBar_->raise();
}

void MainWindow::updateWindowStrokeOverlayGeometry() {
    if (!windowStrokeOverlay_)
        return;
    windowStrokeOverlay_->setGeometry(contentsRect());
    windowStrokeOverlay_->raise();
}

void MainWindow::updateSidebarBlurRegion() {
    // Ported from MainWindow::updateSidebarBlurRegion()
    // (src/mainwindow.cpp:2877-2894) -- real KWin blur-behind on the
    // sidebar column, clipped to the rounded window outline, instead of
    // just a painted tint pretending to be frosted glass.
    if (!windowHandle())
        return;
    const int colRight = sidebarColumnRight();
    if (colRight <= 0) {
        KWindowEffects::enableBlurBehind(windowHandle(), false);
    } else {
        const QRect cr = contentsRect();
        QPainterPath path;
        path.addRoundedRect(cr, FilerChrome::kWindowCornerRadius, FilerChrome::kWindowCornerRadius);
        QRegion region = QRegion(path.toFillPolygon().toPolygon()) & QRegion(cr.left(), cr.top(), colRight - cr.left(), cr.height());
        KWindowEffects::enableBlurBehind(windowHandle(), true, region);
    }
    update(); // repaint the custom chrome for the new geometry
}

void MainWindow::animateToolbarHairlineTo(qreal target) {
    // Ported from MainWindow::animateToolbarHairlineTo() (src/mainwindow.cpp:3033-3049).
    if (qFuzzyCompare(toolbarHairlineOpacity_, target) &&
        toolbarHairlineAnim_->state() != QAbstractAnimation::Running)
        return;
    toolbarHairlineAnim_->stop();
    toolbarHairlineAnim_->setStartValue(toolbarHairlineOpacity_);
    toolbarHairlineAnim_->setEndValue(target);
    toolbarHairlineAnim_->start();
}

void MainWindow::scheduleToolbarHairlineFadeOut() {
    toolbarHairlineIdleTimer_->start();
}

void MainWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    // Ported from MainWindow::paintEvent() (src/mainwindow.cpp:3564-3618,
    // 3630-3643) -- the frosted sidebar column + tinted content-area fill
    // (real TintWindow/SidebarTint/MainWindowTint/Transparency/
    // TransparencyPower formula, not an approximation), plus the toolbar
    // hairline fade.
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const bool frameless = windowFlags() & Qt::FramelessWindowHint;
    const qreal radius = (isMaximized() || !frameless) ? 0 : FilerChrome::kWindowCornerRadius;
    const QRect cr = contentsRect();

    // Soft black drop shadow, painted into the reserved kShadowMargin_ band
    // around cr -- QPainter has no blur primitive, so this fakes one with
    // several concentric rounded-rect *outlines* (not filled rects: filled,
    // overlapping rounded rects compound their alpha at every pixel they
    // all cover, which is what made this look like a harsh near-opaque
    // band right at the window edge before). Each ring only touches its
    // own ~1px band, so alpha fades cleanly from faint at the edge to
    // nothing at kShadowMargin_ out. Slight downward bias for a "dropped"
    // look, not a strong one. Skipped entirely while maximized (cr ==
    // rect(), no margin left to paint into).
    if (!isMaximized()) {
        QPen shadowPen;
        shadowPen.setWidthF(1.0);
        p.setBrush(Qt::NoBrush);
        for (int i = 1; i <= kShadowMargin; ++i) {
            const qreal t = qreal(i) / kShadowMargin; // 0 at the edge, 1 at the outer margin
            QColor layer(0, 0, 0, int(26 * (1.0 - t)));
            shadowPen.setColor(layer);
            p.setPen(shadowPen);
            QRectF ringRect(cr);
            ringRect.adjust(-i * 0.5, i * 0.15, i * 0.5, i * 0.9);
            QPainterPath ringPath;
            ringPath.addRoundedRect(ringRect, radius + i * 0.4, radius + i * 0.4);
            p.drawPath(ringPath);
        }
    }

    QPainterPath winPath;
    winPath.addRoundedRect(cr, radius, radius);

    const int colRight = sidebarColumnRight();
    QPainterPath colPath;
    colPath.addRect(cr.left(), cr.top(), colRight - cr.left(), cr.height());

    // Ported as-is from src/mainwindow.cpp:3594-3618 -- tint/blur stay live
    // regardless of focus there; only the traffic lights go greyscale on
    // deactivate (see changeEvent()).
    const qreal sidebarTintAmount = tintWindow_ ? sidebarTint_ / 100.0 : 0.0;
    const QColor base = palette().color(QPalette::Window);
    QColor sidebarFill = blendTint(base, fixedTint_, sidebarTintAmount);
    if (transparency_) {
        sidebarFill.setAlphaF(1.0 - (transparencyPower_ / 100.0));
    }
    p.fillPath(winPath.intersected(colPath), sidebarFill);
    p.fillPath(winPath.subtracted(colPath), mainAreaTintColor());

    QColor hairline(127, 127, 127, 60);
    const int lineLeft = qMax(colRight, cr.left());
    if (toolBar_ && toolBar_->isVisible() && toolbarHairlineOpacity_ > 0.0) {
        const int y = toolBar_->geometry().bottom();
        QColor fadedHairline = hairline;
        fadedHairline.setAlphaF(hairline.alphaF() * toolbarHairlineOpacity_);
        p.fillRect(QRect(lineLeft, y, cr.right() - lineLeft + 1, 1), fadedHairline);
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == toolBar_) {
        if (event->type() == QEvent::Paint) {
            // Ported from the ui.toolBar Paint branch (src/mainwindow.cpp:
            // 1799-1810): consume the toolbar's own native background paint
            // entirely, so whatever paintEvent() already painted underneath
            // (this window's one shared backing store) shows through
            // unobstructed -- the "unified toolbar" look. Child widgets
            // (traffic lights, spacer) still receive and handle their own
            // separate Paint events regardless, so they still draw normally
            // on top.
            return true;
        }
        // Ported from the ui.toolBar branch of MainWindow::eventFilter()
        // (src/mainwindow.cpp:1798-1835): empty-area presses mean "drag the
        // window" (children like the traffic-light buttons swallow their
        // own clicks first), a double-click toggles maximize like a real
        // title bar, and Enter/Leave drive the hairline fade countdown.
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && windowHandle()) {
                windowHandle()->startSystemMove();
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            isMaximized() ? showNormal() : showMaximized();
            return true;
        } else if (event->type() == QEvent::Enter) {
            toolbarHairlineHoverTimer_->start();
        } else if (event->type() == QEvent::Leave) {
            toolbarHairlineHoverTimer_->stop();
            scheduleToolbarHairlineFadeOut();
        }
    } else if (watched == windowStrokeOverlay_ && event->type() == QEvent::Paint) {
        // Ported from the windowStrokeOverlay_ Paint branch
        // (src/mainwindow.cpp:1721-1741).
        QPainter p(windowStrokeOverlay_);
        p.setRenderHint(QPainter::Antialiasing);
        const bool frameless = windowFlags() & Qt::FramelessWindowHint;
        const qreal radius = (isMaximized() || !frameless) ? 0 : FilerChrome::kWindowCornerRadius;
        QPainterPath strokePath;
        strokePath.addRoundedRect(QRectF(windowStrokeOverlay_->rect()).adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);
        QPen strokePen(QColor(255, 255, 255, 51)); // 51/255 ~= 20% opacity
        strokePen.setWidthF(1.0);
        p.setPen(strokePen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(strokePath);
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    // Ported from MainWindow::resizeEvent() (src/mainwindow.cpp:2312-2323),
    // minus the spatial-mode/rememberWindowSize persistence (no Settings
    // class linked here, see loadSettings()) and the X11 frameGeometry()
    // wallpaper-tint call -- this shell is Wayland-only, so
    // updateSidebarWallpaperTint() is always fed by WindowGeometryRelay
    // instead (see main.cpp).
    QMainWindow::resizeEvent(event);
    updateToolbarOverlayGeometry();
    updateWindowStrokeOverlayGeometry();
    updateSidebarBlurRegion(); // also calls update()
    update();
}

void MainWindow::moveEvent(QMoveEvent* event) {
    // Ported from MainWindow::moveEvent() (src/mainwindow.cpp:2349-2363),
    // minus the X11 frameGeometry() wallpaper-tint call -- see
    // resizeEvent()'s doc comment. The explicit update() here covers a
    // Wayland/KWin quirk where a frameless+translucent top-level's backing
    // store isn't always repainted by the compositor after an interactive
    // startSystemMove(), leaving stale (black) content on screen until
    // something forces Qt to repaint every pixel.
    QMainWindow::moveEvent(event);
    update();
}

void MainWindow::showEvent(QShowEvent* event) {
    // Ported from MainWindow::showEvent() (src/mainwindow.cpp:2935-2946):
    // the ctor's blur-region/overlay-geometry calls run before the window
    // is shown, when the sidebar isn't visible/laid out yet, so
    // updateSidebarBlurRegion() would otherwise see sidebarColumnRight()==0
    // and disable blur permanently. Recompute once actually visible. The
    // KWin script (see main.cpp's loadWindowTrackerKwinScript()) reports
    // geometry on window discovery, so no direct updateSidebarWallpaperTint()
    // call is needed here either.
    QMainWindow::showEvent(event);
    updateSidebarBlurRegion();
    updateToolbarOverlayGeometry();
    updateWindowStrokeOverlayGeometry();
}

void MainWindow::changeEvent(QEvent* event) {
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        // Collapse the shadow margin to 0 while maximized -- see the ctor's
        // setContentsMargins() doc comment. contentsRect() (used throughout
        // paintEvent()/updateToolbarOverlayGeometry()/
        // updateWindowStrokeOverlayGeometry()/updateSidebarBlurRegion())
        // picks this up automatically once the margins change.
        if (isMaximized())
            setContentsMargins(0, 0, 0, 0);
        else
            setContentsMargins(kShadowMargin, 0, kShadowMargin, kShadowMargin);
        updateToolbarOverlayGeometry();
        updateWindowStrokeOverlayGeometry();
        updateSidebarBlurRegion(); // also calls update()
    } else if (event->type() == QEvent::ActivationChange) {
        windowActive_ = isActiveWindow();
        if (trafficLights_) {
            if (windowActive_) {
                trafficLights_->setGraphicsEffect(nullptr);
            } else {
                auto* greyscale = new QGraphicsColorizeEffect(trafficLights_);
                greyscale->setColor(Qt::gray);
                greyscale->setStrength(1.0);
                trafficLights_->setGraphicsEffect(greyscale);
            }
        }
        updateSidebarBlurRegion(); // also calls update() -- picks up the tint pause too
    }
}
