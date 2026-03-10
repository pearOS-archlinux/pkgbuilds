#include "MainWindow.h"
#include "CalculatorWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QScreen>
#include <QEvent>
#include <QCursor>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QKeySequence>
#include <QKeyCombination>
#include <QShortcut>
#include <QShowEvent>
#include <QDir>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QWindow>

namespace {

class OutlineOverlay : public QWidget
{
public:
    explicit OutlineOverlay(int cornerRadius, QWidget *parent = nullptr) : QWidget(parent), m_r(cornerRadius) {}
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(255, 255, 255, 60), 1));
        p.setBrush(Qt::NoBrush);
        const int w = width();
        const int h = height();
        const int r = m_r;
        // Latura de sus (fără colțuri)
        p.drawLine(r, 0, w - r, 0);
        // Latura stânga
        p.drawLine(0, r, 0, h);
        // Latura de jos (fără colțul dreapta-jos)
        p.drawLine(r, h - 1, w - r, h - 1);
        // Colț stânga-jos: doar conturul pe arc (180° → 270°)
        p.drawArc(0, h - 2 * r, 2 * r, 2 * r, 180 * 16, 90 * 16);
    }
private:
    int m_r;
};

class AppIconButton : public QPushButton
{
public:
    explicit AppIconButton(QWidget *parent, int size) : QPushButton(parent), m_size(size) {}
protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPushButton::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(255, 255, 255, 80), 1));
        p.setBrush(Qt::NoBrush);
        const QRect r(0, 0, m_size, m_size);
        // Outline alb pe colțul dreapta-sus (arc 270° → 360°)
        p.drawArc(r, 270 * 16, 90 * 16);
        // Outline alb pe colțul stânga-jos (arc 90° → 180°)
        p.drawArc(r, 90 * 16, 90 * 16);
    }
private:
    int m_size;
};

QString shortcutSymbols(const QKeySequence &seq)
{
    if (seq.isEmpty()) return QString();
    QString s = seq.toString(QKeySequence::PortableText);
    s.replace("Ctrl+", "⌃");
    s.replace("Control+", "⌃");
    s.replace("Shift+", "⇧");
    s.replace("Alt+", "⌥");
    s.replace("Meta+", "⌘");
    s.replace("Command+", "⌘");
    return s;
}
void setActionShortcutDisplay(QWidget *parent, QAction *act, const QString &text, const QKeySequence &shortcut)
{
    if (!shortcut.isEmpty()) {
        QShortcut *sc = new QShortcut(shortcut, parent);
        QObject::connect(sc, &QShortcut::activated, act, [act]() { act->trigger(); });
    }
    act->setText(text + (shortcut.isEmpty() ? QString() : QString("\t") + shortcutSymbols(shortcut)));
}
}

const QString MainWindow::kSettingsPath = QStringLiteral("/usr/share/extras/calculator/settings.json");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_dragging(false)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    m_calculatorPanel = new QWidget(this);
    m_calculatorPanel->setObjectName("centralWidget");
    m_calculatorPanel->setStyleSheet(
        "#centralWidget {"
        "  background-color: #1c1c1e;"
        "  border-radius: " + QString::number(kCornerRadius) + "px;"
        "}"
    );
    setCentralWidget(m_calculatorPanel);
    setFixedSize(260, 520);

    m_outlineOverlay = new OutlineOverlay(kCornerRadius, m_calculatorPanel);
    m_outlineOverlay->setAttribute(Qt::WA_TranslucentBackground);
    m_outlineOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_outlineOverlay->setGeometry(m_calculatorPanel->rect());
    m_outlineOverlay->raise();

    QVBoxLayout *mainLayout = new QVBoxLayout(m_calculatorPanel);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupTrafficLights();

    QWidget *titleBar = new QWidget(m_calculatorPanel);
    titleBar->setObjectName(QStringLiteral("titleBar"));
    titleBar->setFixedHeight(44);
    titleBar->setStyleSheet("background: transparent;");
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(12, 10, 12, 0);
    titleLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_trafficContainer = new QWidget(titleBar);
    m_trafficContainer->setFixedSize(
        kTrafficLightSize * 3 + kTrafficLightSpacing * 2,
        kTrafficLightSize
    );
    m_trafficContainer->setStyleSheet("background: transparent;");
    m_trafficContainer->setMouseTracking(true);
    QHBoxLayout *trafficLayout = new QHBoxLayout(m_trafficContainer);
    trafficLayout->setContentsMargins(0, 0, 0, 0);
    trafficLayout->setSpacing(kTrafficLightSpacing);

    m_btnClose = new QPushButton(m_trafficContainer);
    m_btnClose->setFixedSize(kTrafficLightSize, kTrafficLightSize);
    m_btnClose->setCursor(Qt::ArrowCursor);
    connect(m_btnClose, &QPushButton::clicked, this, &QMainWindow::close);

    m_btnMinimize = new QPushButton(m_trafficContainer);
    m_btnMinimize->setFixedSize(kTrafficLightSize, kTrafficLightSize);
    m_btnMinimize->setCursor(Qt::ArrowCursor);
    connect(m_btnMinimize, &QPushButton::clicked, this, [this]() { showMinimized(); });

    m_btnMaximize = new QPushButton(m_trafficContainer);
    m_btnMaximize->setFixedSize(kTrafficLightSize, kTrafficLightSize);
    m_btnMaximize->setCursor(Qt::ArrowCursor);
    m_btnMaximize->setEnabled(false);

    trafficLayout->addWidget(m_btnClose);
    trafficLayout->addWidget(m_btnMinimize);
    trafficLayout->addWidget(m_btnMaximize);

    m_btnClose->installEventFilter(this);
    m_btnMinimize->installEventFilter(this);
    m_btnMaximize->installEventFilter(this);
    m_trafficContainer->installEventFilter(this);

    titleLayout->addWidget(m_trafficContainer);
    titleLayout->addStretch();

    m_btnAppIcon = new AppIconButton(titleBar, kAppIconSize);
    m_btnAppIcon->setFixedSize(kAppIconSize, kAppIconSize);
    m_btnAppIcon->setCursor(Qt::ArrowCursor);
    m_btnAppIcon->setFlat(true);
    QString iconPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../assets/calculator.png"));
    if (!QFile::exists(iconPath))
        iconPath = QStringLiteral("/usr/share/extras/calculator/calculator.png");
    m_btnAppIcon->setIcon(QIcon(iconPath));
    m_btnAppIcon->setIconSize(QSize(20, 20));
    m_btnAppIcon->setStyleSheet(
        "QPushButton { border: none; border-radius: " + QString::number(kAppIconSize / 2) + "px; background: transparent; } "
        "QPushButton:hover { background-color: rgba(255,255,255,0.1); } "
        "QPushButton:pressed { background-color: rgba(255,255,255,0.15); } "
    );
    titleLayout->addWidget(m_btnAppIcon);

    mainLayout->addWidget(titleBar);

    QMenuBar *menuBar = this->menuBar();
    menuBar->setNativeMenuBar(true);
    menuBar->setStyleSheet(
        "QMenuBar { background: transparent; color: #e0e0e0; padding: 4px 0; } "
        "QMenuBar::item:selected { background: #3a3a3c; border-radius: 4px; } "
        "QMenu { background: #2c2c2e; color: #e0e0e0; } "
        "QMenu::item:selected { background: #3a3a3c; } "
        "QMenu::item:disabled { color: #6e6e72; } "
        "QMenu::separator { height: 1px; background: #48484a; } "
    );
    menuBar->installEventFilter(this);
    QMenu *editMenu = menuBar->addMenu(tr("Edit"));
    setActionShortcutDisplay(this, editMenu->addAction(QString()), tr("Undo"), QKeySequence::Undo);
    setActionShortcutDisplay(this, editMenu->addAction(QString()), tr("Redo"), QKeySequence::Redo);
    editMenu->addSeparator();
    setActionShortcutDisplay(this, editMenu->addAction(QString()), tr("Cut"), QKeySequence::Cut);
    setActionShortcutDisplay(this, editMenu->addAction(QString()), tr("Copy"), QKeySequence::Copy);
    setActionShortcutDisplay(this, editMenu->addAction(QString()), tr("Paste"), QKeySequence::Paste);
    editMenu->addAction(tr("Delete"));
    editMenu->addSeparator();
    setActionShortcutDisplay(this, editMenu->addAction(QString()), tr("Select All"), QKeySequence::SelectAll);
    editMenu->addSeparator();
    QMenu *autofillMenu = editMenu->addMenu(tr("Autofill"));
    QAction *contactAct = autofillMenu->addAction(tr("Contact..."));
    contactAct->setEnabled(false);
    QAction *passwordsAct = autofillMenu->addAction(tr("Passwords..."));
    passwordsAct->setEnabled(false);
    QAction *creditCardAct = autofillMenu->addAction(tr("Credit Card..."));
    creditCardAct->setEnabled(false);

    QMenu *viewMenu = menuBar->addMenu(tr("View"));
    QKeySequence cmd1(QKeyCombination(Qt::ControlModifier, Qt::Key_1));
    QKeySequence cmd2(QKeyCombination(Qt::ControlModifier, Qt::Key_2));
    QKeySequence cmd3(QKeyCombination(Qt::ControlModifier, Qt::Key_3));
    QKeySequence optCmdC(QKeyCombination(Qt::AltModifier | Qt::ControlModifier, Qt::Key_C));
    QKeySequence cmdR(QKeyCombination(Qt::ControlModifier, Qt::Key_R));
    QKeySequence optCmdM(QKeyCombination(Qt::AltModifier | Qt::ControlModifier, Qt::Key_M));
    QKeySequence ctrlCmdS(QKeyCombination(Qt::ControlModifier | Qt::MetaModifier, Qt::Key_S));

    setActionShortcutDisplay(this, viewMenu->addAction(QString()), tr("Basic"), cmd1);
    setActionShortcutDisplay(this, viewMenu->addAction(QString()), tr("Scientific"), cmd2);
    setActionShortcutDisplay(this, viewMenu->addAction(QString()), tr("Programmer"), cmd3);
    viewMenu->addSeparator();
    setActionShortcutDisplay(this, viewMenu->addAction(QString()), tr("Convert"), optCmdC);
    QAction *rpnModeAct = viewMenu->addAction(tr("RPN Mode"));
    rpnModeAct->setCheckable(true);
    rpnModeAct->setChecked(true);
    setActionShortcutDisplay(this, rpnModeAct, tr("RPN Mode"), cmdR);
    viewMenu->addSeparator();
    setActionShortcutDisplay(this, viewMenu->addAction(QString()), tr("Math Notes..."), optCmdM);
    viewMenu->addSeparator();
    QAction *thousandsSepAct = viewMenu->addAction(tr("Thousands Separator"));
    thousandsSepAct->setCheckable(true);
    thousandsSepAct->setChecked(true);
    QMenu *decimalPlacesMenu = viewMenu->addMenu(tr("Decimal Places"));
    QActionGroup *decimalGroup = new QActionGroup(this);
    decimalGroup->setExclusive(true);
    QAction *decimalActs[16];
    for (int i = 0; i <= 15; ++i) {
        decimalActs[i] = decimalPlacesMenu->addAction(QString::number(i));
        decimalActs[i]->setCheckable(true);
        decimalGroup->addAction(decimalActs[i]);
        if (i == 8)
            decimalActs[i]->setChecked(true);
    }
    viewMenu->addSeparator();
    QAction *showHistoryAct = viewMenu->addAction(QString());
    setActionShortcutDisplay(this, showHistoryAct, tr("Show Full History"), ctrlCmdS);
    showHistoryAct->setEnabled(false);
    QAction *fullScreenAct = viewMenu->addAction(QString());
    setActionShortcutDisplay(this, fullScreenAct, tr("Enter Full Screen"), QKeySequence::FullScreen);
    fullScreenAct->setEnabled(false);

    QMenu *windowMenu = menuBar->addMenu(tr("Window"));
    QKeySequence cmdW(QKeyCombination(Qt::ControlModifier, Qt::Key_W));
    QKeySequence cmdM(QKeyCombination(Qt::ControlModifier, Qt::Key_M));
    QKeySequence ctrlMetaF(QKeyCombination(Qt::ControlModifier | Qt::MetaModifier, Qt::Key_F));
    QKeySequence ctrlMetaC(QKeyCombination(Qt::ControlModifier | Qt::MetaModifier, Qt::Key_C));

    QAction *closeWinAct = windowMenu->addAction(tr("Close"));
    setActionShortcutDisplay(this, closeWinAct, tr("Close"), cmdW);
    connect(closeWinAct, &QAction::triggered, this, &QMainWindow::close);

    QAction *minimizeAct = windowMenu->addAction(tr("Minimize"));
    setActionShortcutDisplay(this, minimizeAct, tr("Minimize"), cmdM);
    connect(minimizeAct, &QAction::triggered, this, &QMainWindow::showMinimized);

    QAction *minimizeAllAct = windowMenu->addAction(tr("Minimize All"));
    minimizeAllAct->setEnabled(false);

    QAction *zoomAct = windowMenu->addAction(tr("Zoom"));
    zoomAct->setEnabled(false);

    QAction *fillAct = windowMenu->addAction(tr("Fill"));
    setActionShortcutDisplay(this, fillAct, tr("Fill"), ctrlMetaF);
    fillAct->setEnabled(false);

    QAction *centerAct = windowMenu->addAction(tr("Center"));
    setActionShortcutDisplay(this, centerAct, tr("Center"), ctrlMetaC);

    windowMenu->addSeparator();

    QMenu *moveResizeMenu = windowMenu->addMenu(tr("Move & Resize"));
    moveResizeMenu->addAction(tr("Left"));
    moveResizeMenu->addAction(tr("Right"));
    moveResizeMenu->addAction(tr("Top"));
    moveResizeMenu->addAction(tr("Bottom"));
    moveResizeMenu->addSeparator();
    moveResizeMenu->addAction(tr("Top Left"));
    moveResizeMenu->addAction(tr("Top Right"));
    moveResizeMenu->addAction(tr("Bottom Left"));
    moveResizeMenu->addAction(tr("Bottom Right"));
    moveResizeMenu->addSeparator();
    moveResizeMenu->addAction(tr("Left & Right"));
    moveResizeMenu->addAction(tr("Right & Left"));
    moveResizeMenu->addAction(tr("Top & Bottom"));
    moveResizeMenu->addAction(tr("Bottom & Top"));
    moveResizeMenu->addAction(tr("Quarters"));
    moveResizeMenu->addSeparator();
    QAction *returnPrevAct = moveResizeMenu->addAction(tr("Return to Previous Size"));
    returnPrevAct->setEnabled(false);

    QAction *fullScreenTileAct = windowMenu->addAction(tr("Full screen Tile"));
    fullScreenTileAct->setEnabled(false);
    windowMenu->addSeparator();
    QAction *removeFromSetAct = windowMenu->addAction(tr("Remove Window from Set"));
    removeFromSetAct->setEnabled(false);
    windowMenu->addSeparator();

    QAction *alwaysOnTopAct = windowMenu->addAction(tr("Always on Top"));
    alwaysOnTopAct->setCheckable(true);
    alwaysOnTopAct->setChecked(false);
    connect(alwaysOnTopAct, &QAction::toggled, this, [this](bool checked) {
        setWindowFlag(Qt::WindowStaysOnTopHint, checked);
        show();
    });

    QAction *bringAllToFrontAct = windowMenu->addAction(tr("Bring All to Front"));
    connect(bringAllToFrontAct, &QAction::triggered, this, [this]() {
        raise();
        activateWindow();
    });

    QMenu *helpMenu = menuBar->addMenu(tr("Help"));
    QAction *donateAct = helpMenu->addAction(tr("Donate"));
    connect(donateAct, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://pearos.xyz/donate")));
    });
    QAction *discordAct = helpMenu->addAction(tr("Discord"));
    connect(discordAct, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://discord.com/invite/FYGBBgJ3Xx")));
    });

    CalculatorWidget *calc = new CalculatorWidget(m_calculatorPanel);
    mainLayout->addWidget(calc, 1);

    bool loadedRpn = true;
    bool loadedThousands = true;
    loadSettings(&loadedRpn, &loadedThousands);
    rpnModeAct->blockSignals(true);
    rpnModeAct->setChecked(loadedRpn);
    rpnModeAct->blockSignals(false);
    thousandsSepAct->blockSignals(true);
    thousandsSepAct->setChecked(loadedThousands);
    thousandsSepAct->blockSignals(false);
    calc->setRpnMode(loadedRpn);
    calc->setThousandsSeparatorEnabled(loadedThousands);
    setFixedSize(260, loadedRpn ? 520 : 420);

    connect(rpnModeAct, &QAction::toggled, this, [this, calc, rpnModeAct, thousandsSepAct](bool checked) {
        calc->setRpnMode(checked);
        setFixedSize(260, checked ? 520 : 420);
        saveSettings(rpnModeAct->isChecked(), thousandsSepAct->isChecked());
    });
    connect(thousandsSepAct, &QAction::toggled, this, [this, calc, rpnModeAct, thousandsSepAct](bool checked) {
        calc->setThousandsSeparatorEnabled(checked);
        saveSettings(rpnModeAct->isChecked(), thousandsSepAct->isChecked());
    });

    m_calculatorPanel->setMouseTracking(true);
    m_calculatorPanel->installEventFilter(this);
    titleBar->installEventFilter(this);
    m_btnAppIcon->installEventFilter(this);
    setTrafficLightsActive(true);

    setWindowTitle(QStringLiteral("Calculator"));
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    setWindowTitle(QStringLiteral("Calculator"));
}

MainWindow::~MainWindow() = default;

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        const bool active = isActiveWindow();
        CalculatorWidget *calc = findChild<CalculatorWidget *>();
        if (calc)
            calc->setWindowActive(active);
        setTrafficLightsActive(active);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // Drag doar pe bara cu traffic lights și zona din dreapta, nu pe butoane.
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *w = qobject_cast<QWidget *>(watched);
        if (w && w->objectName() == QStringLiteral("titleBar")) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                QWidget *childAtPos = w->childAt(me->pos());
                // Nu permitem drag dacă se apasă direct pe traffic lights sau pe icon.
                if (childAtPos == m_trafficContainer ||
                    childAtPos == m_btnClose ||
                    childAtPos == m_btnMinimize ||
                    childAtPos == m_btnMaximize ||
                    childAtPos == m_btnAppIcon) {
                    return false;
                }
                if (QWindow *win = windowHandle()) {
                    qDebug() << "[MainWindow::eventFilter] startSystemMove from titleBar, local"
                             << me->pos() << "global" << me->globalPosition();
                    win->startSystemMove();
                    return true;
                }
            }
        }
    }

    if (event->type() == QEvent::Enter &&
        (watched == m_btnClose || watched == m_btnMinimize || watched == m_btnMaximize)) {
        m_btnClose->setText(QString::fromUtf8("×"));
        m_btnMinimize->setText(QString::fromUtf8("−"));
        m_btnMaximize->setText("");
        return false;
    }
    if (event->type() == QEvent::Leave) {
        if (watched == m_trafficContainer) {
            m_btnClose->setText("");
            m_btnMinimize->setText("");
            m_btnMaximize->setText("");
        } else if (watched == m_btnClose || watched == m_btnMinimize || watched == m_btnMaximize) {
            QWidget *under = QApplication::widgetAt(QCursor::pos());
            bool stillOverTraffic = (under == m_trafficContainer || under == m_btnClose
                                    || under == m_btnMinimize || under == m_btnMaximize);
            if (!stillOverTraffic) {
                m_btnClose->setText("");
                m_btnMinimize->setText("");
                m_btnMaximize->setText("");
            }
        }
        return false;
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setupTrafficLights()
{
}

void MainWindow::setTrafficLightsActive(bool active)
{
    const QString base = QStringLiteral("QPushButton { border-radius: 6px; border: none; color: #2c2c2e; font-size: 9px; font-weight: bold; } ");
    const QString maximizeStyle = QStringLiteral("QPushButton { background-color: #414143; } ");
    if (active) {
        m_btnClose->setStyleSheet(base +
            QStringLiteral("QPushButton { background-color: #ff5f57; } "
                           "QPushButton:pressed { background-color: #ff7b73; }"));
        m_btnMinimize->setStyleSheet(base +
            QStringLiteral("QPushButton { background-color: #febc2e; } "
                           "QPushButton:pressed { background-color: #ffd04a; }"));
        m_btnMaximize->setStyleSheet(base + maximizeStyle);
    } else {
        const QString inactive = QStringLiteral("QPushButton { background-color: #414143; } "
                                                 "QPushButton:pressed { background-color: #505052; }");
        m_btnClose->setStyleSheet(base + inactive);
        m_btnMinimize->setStyleSheet(base + inactive);
        m_btnMaximize->setStyleSheet(base + maximizeStyle);
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(rect(), Qt::transparent);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_dragging = true;
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton))
        move(event->globalPosition().toPoint() - m_dragPos);
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragging = false;
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_outlineOverlay && m_calculatorPanel)
        m_outlineOverlay->setGeometry(m_calculatorPanel->rect());
    updateMask();
}

void MainWindow::loadSettings(bool *outRpnMode, bool *outThousandsSeparator) const
{
    if (!outRpnMode || !outThousandsSeparator) return;
    *outRpnMode = true;
    *outThousandsSeparator = true;

    QStringList paths;
    paths << kSettingsPath
          << (QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
              + QStringLiteral("/extras/calculator/settings.json"));

    for (const QString &path : paths) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
            continue;
        QByteArray data = file.readAll();
        file.close();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject())
            continue;
        QJsonObject obj = doc.object();
        if (obj.contains(QStringLiteral("rpnMode")) && obj["rpnMode"].isBool())
            *outRpnMode = obj["rpnMode"].toBool();
        if (obj.contains(QStringLiteral("thousandsSeparator")) && obj["thousandsSeparator"].isBool())
            *outThousandsSeparator = obj["thousandsSeparator"].toBool();
        break;
    }
}

void MainWindow::saveSettings(bool rpnMode, bool thousandsSeparator) const
{
    QJsonObject obj;
    obj[QStringLiteral("rpnMode")] = rpnMode;
    obj[QStringLiteral("thousandsSeparator")] = thousandsSeparator;
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson(QJsonDocument::Indented);

    QString path = kSettingsPath;
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
               + QStringLiteral("/extras/calculator/settings.json");
        QDir().mkpath(QFileInfo(path).absolutePath());
        file.setFileName(path);
        if (!file.open(QIODevice::WriteOnly))
            return;
    }
    if (file.isOpen()) {
        file.write(json);
        file.close();
    }
}

void MainWindow::updateMask()
{
    if (!m_calculatorPanel) return;
    m_calculatorPanel->setMask(roundedRegion(m_calculatorPanel->rect(), kCornerRadius));
}

QRegion MainWindow::roundedRegion(const QRect &rect, int radius) const
{
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    return QRegion(path.toFillPolygon().toPolygon());
}
