#include "AboutWindow.h"

#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>

AboutWindow::AboutWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("About This Computer"));
    setFixedSize(340, 590);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    buildUi();
    applySystemTheme();
    loadSystemInfo();
    detectBatteryAndDeviceType();
    setupConnections();
}

void AboutWindow::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(6, 6, 6, 6);
    rootLayout->setSpacing(0);

    auto *windowBackground = new QWidget(this);
    windowBackground->setObjectName(QStringLiteral("windowBackground"));
    windowBackground->setAttribute(Qt::WA_StyledBackground);

    auto *bgLayout = new QVBoxLayout(windowBackground);
    bgLayout->setContentsMargins(0, 0, 0, 0);
    bgLayout->setSpacing(0);

    m_titleBar = new QWidget(windowBackground);
    m_titleBar->setObjectName(QStringLiteral("titleBar"));
    m_titleBar->setFixedHeight(25);
    // Allow dragging the window by interacting with the custom title bar
    m_titleBar->installEventFilter(this);
    m_titleBar->installEventFilter(this);

    auto *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(20, 0, 0, 0);
    titleLayout->setSpacing(6);

    auto createDot = [this](const QString &name, const QString &color) {
        auto *btn = new QPushButton(m_titleBar);
        btn->setObjectName(name);
        btn->setFixedSize(14, 14);
        btn->setCursor(Qt::ArrowCursor);
        btn->setFlat(true);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setProperty("dotColor", color);
        return btn;
    };

    auto *redDot = createDot(QStringLiteral("redDot"), QStringLiteral("#fe5b51"));
    auto *yellowDot = createDot(QStringLiteral("yellowDot"), QStringLiteral("#e6c02a"));
    auto *greenDot = createDot(QStringLiteral("greenDot"), QStringLiteral("#51c329"));

    titleLayout->addWidget(redDot);
    titleLayout->addWidget(yellowDot);
    titleLayout->addWidget(greenDot);
    titleLayout->addStretch();

    auto *contentWrapper = new QWidget(windowBackground);
    auto *contentLayout = new QVBoxLayout(contentWrapper);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(20);

    auto *aboutPage = new QWidget(contentWrapper);
    auto *aboutLayout = new QVBoxLayout(aboutPage);
    aboutLayout->setContentsMargins(0, 20, 0, 0);
    // spațiere mai mică între secțiuni, ca în UI-ul original
    aboutLayout->setSpacing(12);
    aboutLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto *header = new QWidget(aboutPage);
    auto *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(15);
    headerLayout->setAlignment(Qt::AlignHCenter);

    m_deviceImage = new QLabel(header);
    m_deviceImage->setFixedSize(200, 200);
    m_deviceImage->setAlignment(Qt::AlignCenter);
    m_deviceImage->setObjectName(QStringLiteral("deviceImage"));

    m_deviceType = new QLabel(QStringLiteral("Laptop"), header);
    m_deviceType->setObjectName(QStringLiteral("deviceType"));
    m_deviceType->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(m_deviceImage);
    headerLayout->addWidget(m_deviceType);

    auto *infoSection = new QWidget(aboutPage);
    auto *infoLayout = new QVBoxLayout(infoSection);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(3);

    auto addInfoRow = [infoSection, infoLayout](const QString &labelText, QLabel *&valueLabel) {
        auto *row = new QWidget(infoSection);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(8);

        auto *label = new QLabel(labelText, row);
        label->setObjectName(QStringLiteral("infoLabel"));
        label->setMinimumWidth(120);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        valueLabel = new QLabel(QStringLiteral("Unknown"), row);
        valueLabel->setObjectName(QStringLiteral("infoValue"));
        valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        rowLayout->addWidget(label);
        rowLayout->addWidget(valueLabel, 1);

        infoLayout->addWidget(row);
    };

    addInfoRow(QStringLiteral("Chip:"), m_chipLabel);
    addInfoRow(QStringLiteral("Memory:"), m_memoryLabel);
    addInfoRow(QStringLiteral("Serial Number:"), m_serialLabel);

    auto *osSection = new QWidget(aboutPage);
    auto *osLayout = new QHBoxLayout(osSection);
    // fără margin top suplimentar, pentru a elimina rândul liber
    osLayout->setContentsMargins(0, 0, 0, 0);
    osLayout->setSpacing(8);

    m_osNameLabel = new QLabel(QStringLiteral("pearOS"), osSection);
    m_osNameLabel->setObjectName(QStringLiteral("osName"));
    m_osNameLabel->setMinimumWidth(120);
    m_osNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_osVersionLabel = new QLabel(QStringLiteral("Unknown"), osSection);
    m_osVersionLabel->setObjectName(QStringLiteral("osVersion"));
    m_osVersionLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    osLayout->addWidget(m_osNameLabel);
    osLayout->addWidget(m_osVersionLabel, 1);

    auto *actionsWidget = new QWidget(aboutPage);
    auto *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(0);
    actionsLayout->setAlignment(Qt::AlignCenter);

    m_moreInfoButton = new QPushButton(QStringLiteral("More Info..."), actionsWidget);
    m_moreInfoButton->setObjectName(QStringLiteral("moreInfoButton"));
    actionsLayout->addWidget(m_moreInfoButton);

    m_footerLabel = new QLabel(aboutPage);
    m_footerLabel->setObjectName(QStringLiteral("footerLabel"));
    m_footerLabel->setAlignment(Qt::AlignCenter);
    m_footerLabel->setTextFormat(Qt::RichText);
    m_footerLabel->setOpenExternalLinks(true);

    const int year = QDate::currentDate().year();
    m_footerLabel->setText(
        QStringLiteral("Copyright © 2017-%1 "
                       "<a href=\"https://pear-software.com\">Pear Software and Services S.R.L.</a> "
                       "All rights reserved.")
            .arg(year));

    aboutLayout->addWidget(header);
    aboutLayout->addWidget(infoSection);
    aboutLayout->addWidget(osSection);
    aboutLayout->addWidget(actionsWidget);
    aboutLayout->addStretch();
    aboutLayout->addWidget(m_footerLabel);

    contentLayout->addWidget(aboutPage);

    bgLayout->addWidget(m_titleBar);
    bgLayout->addWidget(contentWrapper, 1);

    rootLayout->addWidget(windowBackground);
}

void AboutWindow::applySystemTheme()
{
    // Citește starea temei din system-settings (light/dark)
    bool dark = false;
    {
        QFile themeFile(QStringLiteral("/usr/share/extras/system-settings/themeswitcher/state"));
        if (themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString state = QString::fromUtf8(themeFile.readAll()).trimmed().toLower();
            if (state == QLatin1String("dark"))
                dark = true;
            else if (state == QLatin1String("light"))
                dark = false;
            else {
                const QColor bg = palette().color(QPalette::Window);
                dark = bg.lightness() < 128;
            }
        } else {
            const QColor bg = palette().color(QPalette::Window);
            dark = bg.lightness() < 128;
        }
    }

    // Culori inspirate din CSS-ul original (light/dark)
    const QString bgPrimary  = dark ? QStringLiteral("#1D232A") : QStringLiteral("#FFFFFF");
    const QString textPrimary = dark ? QStringLiteral("#FFFFFF") : QStringLiteral("#111111");
    const QString textSecondary = dark ? QStringLiteral("rgba(255, 255, 255, 0.7)")
                                       : QStringLiteral("rgba(0, 0, 0, 0.6)");
    const QString borderColor = dark ? QStringLiteral("#35404C")
                                     : QStringLiteral("rgba(0, 0, 0, 0.1)");

    const QString style = QStringLiteral(R"(
        QWidget#windowBackground {
            background-color: %1;
            border-radius: 25px;
        }
        QWidget#titleBar {
            border-radius: 19px 20px 0px 0px;
            background-color: transparent;
        }
        QPushButton#redDot, QPushButton#yellowDot, QPushButton#greenDot {
            border-radius: 7px;
            border: none;
        }
        QPushButton#redDot { background-color: #fe5b51; }
        QPushButton#yellowDot { background-color: #e6c02a; }
        QPushButton#greenDot { background-color: #51c329; }

        QLabel#deviceType {
            font-size: 28px;
            font-weight: 600;
            color: %2;
        }
        QLabel#infoLabel, QLabel#osName {
            font-size: 13px;
            font-weight: 500;
            color: %2;
        }
        QLabel#infoValue, QLabel#osVersion {
            font-size: 13px;
            color: %2;
        }
        QPushButton#moreInfoButton {
            padding: 8px 20px;
            border-radius: 6px;
            border: 1px solid %3;
            background-color: %1;
            color: %2;
        }
        QLabel#footerLabel {
            font-size: 11px;
            color: %4;
        }
        QLabel#footerLabel a {
            color: #007AFF;
            text-decoration: none;
        }
        QLabel#footerLabel a:hover {
            text-decoration: underline;
        }
    )").arg(bgPrimary, textPrimary, borderColor, textSecondary);

    setStyleSheet(style);
}

void AboutWindow::loadSystemInfo()
{
    QString hostname = runCommand(QStringLiteral("hostname"));
    QString chipRaw = runCommand(QStringLiteral("lscpu | grep \"Model name\" | cut -d\":\" -f2 | xargs"));
    QString memory = runCommand(QStringLiteral("free -h --si | grep \"Mem:\" | awk '{print $2}'"));
    QString serial = runCommand(QStringLiteral("cat /sys/class/dmi/id/product_serial 2>/dev/null || echo \"Not Available\""));

    if (chipRaw.isEmpty())
        chipRaw = QStringLiteral("Unknown");
    if (memory.isEmpty())
        memory = QStringLiteral("Unknown");
    if (serial.isEmpty())
        serial = QStringLiteral("Not Available");

    const QString chip = formatChipName(chipRaw);

    if (m_chipLabel)
        m_chipLabel->setText(chip);
    if (m_memoryLabel)
        m_memoryLabel->setText(memory);
    if (m_serialLabel) {
        const QString cleanedSerial = (serial == QLatin1String("Not Available") ||
                                       serial == QLatin1String("To be filled by a.E.M."))
                                          ? QStringLiteral("Not Available")
                                          : serial;
        m_serialLabel->setText(cleanedSerial);
    }

    QString osName = runCommand(QStringLiteral("grep \"^PRETTY_NAME=\" /etc/os-release 2>/dev/null | cut -d\"=\" -f2"));
    if (osName.isEmpty())
        osName = QStringLiteral("pearOS (?)");

    QString osVersion = runCommand(
        QStringLiteral("grep \"^VERSION=\" /etc/os-release 2>/dev/null | cut -d\"=\" -f2 | tr -d '\"' || "
                       "grep \"^VERSION_ID=\" /etc/os-release 2>/dev/null | cut -d\"=\" -f2 | tr -d '\"' || "
                       "grep DISTRIB_RELEASE /etc/lsb-release 2>/dev/null | cut -d\"=\" -f2 | tr -d '\"'"));
    if (osVersion.isEmpty())
        osVersion = QStringLiteral("Unknown");

    if (m_osNameLabel)
        m_osNameLabel->setText(osName.trimmed());
    if (m_osVersionLabel)
        m_osVersionLabel->setText(osVersion.trimmed());
}

void AboutWindow::detectBatteryAndDeviceType()
{
    bool hasBattery = false;

    QString upowerOutput = runCommand(QStringLiteral("upower -i $(upower -e | grep battery) 2>/dev/null"));
    if (!upowerOutput.isEmpty()) {
        const QRegularExpression re(QStringLiteral("percentage:\\s*(\\d+)"));
        const QRegularExpressionMatch match = re.match(upowerOutput);
        if (match.hasMatch()) {
            hasBattery = true;
        }
    } else {
        QString capacity = runCommand(QStringLiteral("cat /sys/class/power_supply/BAT*/capacity 2>/dev/null | head -1"));
        if (!capacity.trimmed().isEmpty())
            hasBattery = true;
    }

    const QString deviceImagePath = hasBattery
                                        ? resolveAssetPath(QStringLiteral("laptop.svg"))
                                        : resolveAssetPath(QStringLiteral("computer.svg"));

    if (m_deviceImage) {
        QPixmap pix(deviceImagePath);
        if (!pix.isNull()) {
            m_deviceImage->setPixmap(pix.scaled(m_deviceImage->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    if (m_deviceType) {
        m_deviceType->setText(hasBattery ? QStringLiteral("Laptop") : QStringLiteral("Computer"));
    }
}

void AboutWindow::setupConnections()
{
    if (!m_titleBar)
        return;

    const auto buttons = m_titleBar->findChildren<QPushButton *>();
    for (auto *btn : buttons) {
        if (!btn)
            continue;
        if (btn->objectName() == QLatin1String("redDot")) {
            connect(btn, &QPushButton::clicked, this, &QWidget::close);
        } else if (btn->objectName() == QLatin1String("yellowDot")) {
            connect(btn, &QPushButton::clicked, this, [this]() { showMinimized(); });
        } else if (btn->objectName() == QLatin1String("greenDot")) {
            connect(btn, &QPushButton::clicked, this, [this]() {
                if (isMaximized())
                    showNormal();
                else
                    showMaximized();
            });
        }
    }

    if (m_moreInfoButton) {
        connect(m_moreInfoButton, &QPushButton::clicked, this, &AboutWindow::openSystemSettingsAbout);
    }
}

QString AboutWindow::runCommand(const QString &command, int timeoutMs) const
{
    QProcess proc;
    proc.start(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), command});
    if (!proc.waitForFinished(timeoutMs)) {
        proc.kill();
        proc.waitForFinished();
        return {};
    }
    const QByteArray out = proc.readAllStandardOutput();
    return QString::fromLocal8Bit(out).trimmed();
}

QString AboutWindow::resolveAssetPath(const QString &fileName) const
{
    const QStringList candidates = {
        QStringLiteral("/usr/share/extras/pearos-about/assets/%1").arg(fileName),
        QStringLiteral("/usr/share/extras/system-settings/assets/%1").arg(fileName),
        QStringLiteral(":/assets/%1").arg(fileName)
    };

    for (const QString &path : candidates) {
        QFileInfo fi(path);
        if (fi.exists() && fi.isFile()) {
            return path;
        }
    }

    // dacă nu găsim nimic, întoarcem ultima variantă (qrc)
    return QStringLiteral(":/assets/%1").arg(fileName);
}

bool AboutWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Drag only when interacting with the custom title bar
    if (obj == m_titleBar) {
        auto *me = dynamic_cast<QMouseEvent *>(event);
        if (!me)
            return QWidget::eventFilter(obj, event);

        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if (me->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragStartPos = me->globalPosition().toPoint() - frameGeometry().topLeft();
                return false; // let buttons still receive clicks
            }
            break;
        case QEvent::MouseMove:
            if (m_dragging && (me->buttons() & Qt::LeftButton)) {
                move(me->globalPosition().toPoint() - m_dragStartPos);
                return true; // consume move
            }
            break;
        case QEvent::MouseButtonRelease:
            if (me->button() == Qt::LeftButton && m_dragging) {
                m_dragging = false;
                return false;
            }
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(obj, event);
}

QString AboutWindow::formatChipName(const QString &cpuName) const
{
    if (cpuName.isEmpty())
        return QStringLiteral("Unknown");

    const QRegularExpression freqRe(QStringLiteral("@\\s*([\\d.]+)\\s*(GHz|MHz)"), QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch freqMatch = freqRe.match(cpuName);
    QString frequency;
    if (freqMatch.hasMatch()) {
        frequency = QStringLiteral("@ %1%2").arg(freqMatch.captured(1), freqMatch.captured(2));
    }

    const QRegularExpression intelRe(
        QStringLiteral("(?:Intel\\s+)?(?:\\(R\\)\\s+)?(?:Core\\s+)?(?:\\(TM\\)\\s+)?"
                       "(?:11th\\s+Gen\\s+)?(?:12th\\s+Gen\\s+)?(?:13th\\s+Gen\\s+)?(?:14th\\s+Gen\\s+)?"
                       "(i[3579]|i\\d+-[\\w-]+)"),
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpression amdRe(
        QStringLiteral("(?:AMD\\s+)?(Ryzen\\s+[\\d\\s\\w-]+|Athlon\\s+[\\w-]+|EPYC\\s+[\\w-]+|"
                       "Threadripper\\s+[\\w-]+|FX[\\s-][\\w-]+|Phenom\\s+[\\w-]+)"),
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpression otherRe(
        QStringLiteral("(Pentium\\s+[\\w-]+|Celeron\\s+[\\w-]+|Atom\\s+[\\w-]+|Xeon\\s+[\\w-]+)"),
        QRegularExpression::CaseInsensitiveOption);

    QString model;
    QRegularExpressionMatch match = intelRe.match(cpuName);
    if (match.hasMatch()) {
        model = match.captured(1).isEmpty() ? match.captured(0) : match.captured(1);
    } else {
        match = amdRe.match(cpuName);
        if (match.hasMatch()) {
            model = match.captured(1).isEmpty() ? match.captured(0) : match.captured(1);
        } else {
            match = otherRe.match(cpuName);
            if (match.hasMatch()) {
                model = match.captured(1).isEmpty() ? match.captured(0) : match.captured(1);
            }
        }
    }

    if (!model.isEmpty()) {
        model = model.simplified();
    }

    if (!model.isEmpty() && !frequency.isEmpty()) {
        return model + QLatin1Char(' ') + frequency;
    } else if (!model.isEmpty()) {
        return model;
    } else if (!frequency.isEmpty()) {
        return frequency;
    }

    return cpuName;
}

void AboutWindow::openSystemSettingsAbout()
{
    const QString settingsDir = QStringLiteral("/usr/share/extras/system-settings");
    QString program = QStringLiteral("npm");
    QStringList args;
    args << QStringLiteral("start") << QStringLiteral("about");

    QProcess::startDetached(program, args, settingsDir);
}

void AboutWindow::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void AboutWindow::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void AboutWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}

void AboutWindow::closeEvent(QCloseEvent *event)
{
    // Asigură-te că la închiderea ferestrei se oprește și aplicația
    QApplication::quit();
    QWidget::closeEvent(event);
}

void AboutWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    qDebug().noquote() << "=== Window size ===";
    qDebug().noquote() << "Inner (viewport):" << QString::number(width()) + "x" + QString::number(height());
    qDebug().noquote() << "Outer (full window):" << QString::number(frameGeometry().width()) + "x" + QString::number(frameGeometry().height());
    qDebug().noquote() << "===================";
}

