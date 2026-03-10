#include "CalculatorWidget.h"
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <cmath>
#include <limits>

namespace {
const int kDisplayFontSizes[] = { 32, 28, 24, 20 };
const int kDisplayFontSizeCount = 4;
const int kDisplayMinFontSize = 20;
const int kGradientOverlayWidth = 28;
}

class GradientOverlay : public QWidget
{
public:
    explicit GradientOverlay(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        QLinearGradient grad(0, 0, width(), 0);
        grad.setColorAt(0, QColor(0x1c, 0x1c, 0x1e, 255));
        grad.setColorAt(1, QColor(0x1c, 0x1c, 0x1e, 0));
        p.fillRect(rect(), grad);
    }
};

namespace {
const char* kPillBg = "#363638";
const char* kNumBg = "#4f4f51";
const char* kGrayOpBg = "#7b7b7d";   // stergere, AC, %
const char* kOrangeOpBg = "#FF9D00"; // / * - + enter
constexpr int kButtonSpacing = 10;   // distanță fixă între butoane, indiferent de dimensiune
}

CalculatorWidget::CalculatorWidget(QWidget *parent)
    : QWidget(parent)
    , m_line1(nullptr)
    , m_line2(nullptr)
    , m_line3(nullptr)
    , m_pillContainer(nullptr)
    , m_enterEqualsButton(nullptr)
    , m_gradientOverlay(nullptr)
    , m_rpnMode(true)
    , m_row1NeedsGradient(false)
    , m_useThousandsSeparator(true)
    , m_hasDecimal(false)
{
    setStyleSheet("CalculatorWidget { background: transparent; }");
    buildUI();
    applyButtonStyles(true);
}

void CalculatorWidget::buildUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(16, 0, 16, 20);

    const char* rowStyle =
        "QLabel { color: #ffffff; font-size: 32px; font-weight: 300; "
        "background: transparent; padding: 6px 8px; }";
    const int rowHeight = 36;

    m_row4 = new QLabel("");
    m_row4->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_row4->setStyleSheet(rowStyle);
    m_row4->setMinimumHeight(rowHeight);
    mainLayout->addWidget(m_row4);

    m_line1 = makeLine();
    mainLayout->addWidget(m_line1);

    m_row3 = new QLabel("");
    m_row3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_row3->setStyleSheet(rowStyle);
    m_row3->setMinimumHeight(rowHeight);
    mainLayout->addWidget(m_row3);

    m_line2 = makeLine();
    mainLayout->addWidget(m_line2);

    m_row2 = new QLabel("");
    m_row2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_row2->setStyleSheet(rowStyle);
    m_row2->setMinimumHeight(rowHeight);
    mainLayout->addWidget(m_row2);

    m_line3 = makeLine();
    mainLayout->addWidget(m_line3);

    m_row1 = new QLabel("0");
    m_row1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_row1->setStyleSheet(rowStyle);
    m_row1->setMinimumHeight(rowHeight);
    mainLayout->addWidget(m_row1);

    m_gradientOverlay = new GradientOverlay(this);
    m_gradientOverlay->setFixedWidth(kGradientOverlayWidth);
    m_gradientOverlay->hide();
    m_gradientOverlay->raise();

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(kButtonSpacing);

    int row = 0;

    m_pillContainer = new QWidget(this);
    QHBoxLayout *pillRow = new QHBoxLayout(m_pillContainer);
    pillRow->setContentsMargins(0, 0, 0, 0);
    pillRow->setSpacing(kButtonSpacing);
    pillRow->addWidget(addPillButton("x↔y", "swap"));
    pillRow->addWidget(addPillButton("R↓", "rollDown"));
    pillRow->addWidget(addPillButton("R↑", "rollUp"));
    pillRow->addWidget(addPillButton("drop", "drop"));
    grid->addWidget(m_pillContainer, row++, 0, 1, 4);

    grid->addWidget(addRoundButton("⌫", kGrayOpBg, "grayOp"), row, 0);
    grid->addWidget(addRoundButton("AC", kGrayOpBg, "grayOp"), row, 1);
    grid->addWidget(addRoundButton("%", kGrayOpBg, "grayOp"), row, 2);
    grid->addWidget(addRoundButton("÷", kOrangeOpBg, "orangeOp"), row, 3);
    row++;

    grid->addWidget(addRoundButton("7", kNumBg, "numeric"), row, 0);
    grid->addWidget(addRoundButton("8", kNumBg, "numeric"), row, 1);
    grid->addWidget(addRoundButton("9", kNumBg, "numeric"), row, 2);
    grid->addWidget(addRoundButton("×", kOrangeOpBg, "orangeOp"), row, 3);
    row++;

    grid->addWidget(addRoundButton("4", kNumBg, "numeric"), row, 0);
    grid->addWidget(addRoundButton("5", kNumBg, "numeric"), row, 1);
    grid->addWidget(addRoundButton("6", kNumBg, "numeric"), row, 2);
    grid->addWidget(addRoundButton("−", kOrangeOpBg, "orangeOp"), row, 3);
    row++;

    grid->addWidget(addRoundButton("1", kNumBg, "numeric"), row, 0);
    grid->addWidget(addRoundButton("2", kNumBg, "numeric"), row, 1);
    grid->addWidget(addRoundButton("3", kNumBg, "numeric"), row, 2);
    grid->addWidget(addRoundButton("+", kOrangeOpBg, "orangeOp"), row, 3);
    row++;

    grid->addWidget(addRoundButton("±", kNumBg, "numeric"), row, 0);
    grid->addWidget(addRoundButton("0", kNumBg, "numeric"), row, 1);
    grid->addWidget(addRoundButton(",", kNumBg, "numeric"), row, 2);
    m_enterEqualsButton = addRoundButton("enter", kOrangeOpBg, "orangeOp");
    grid->addWidget(m_enterEqualsButton, row, 3);

    mainLayout->addSpacing(8);
    mainLayout->addLayout(grid);
}

QFrame *CalculatorWidget::makeLine()
{
    QFrame *line = new QFrame();
    line->setFixedHeight(1);
    line->setStyleSheet("QFrame { background-color: #5a5a5c; border: none; }");
    line->setFrameShape(QFrame::HLine);
    return line;
}

QPushButton *CalculatorWidget::addRoundButton(const QString &text,
                                               const QString &bgColor,
                                               const QString &buttonRole,
                                               const QString &fontColor,
                                               int minSize)
{
    Q_UNUSED(bgColor);
    QPushButton *btn = new QPushButton(text);
    btn->setObjectName(buttonRole);
    btn->setMinimumSize(minSize, minSize);
    btn->setMaximumSize(minSize, minSize);
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btn->setCursor(Qt::ArrowCursor);
    btn->setProperty("calcRole", text);
    connect(btn, &QPushButton::clicked, this, &CalculatorWidget::onDigitClicked);
    return btn;
}

QPushButton *CalculatorWidget::addPillButton(const QString &text, const QString &pillRole, int minHeight)
{
    Q_UNUSED(minHeight);
    QPushButton *btn = new QPushButton(text);
    btn->setObjectName("pill");
    btn->setMinimumHeight(32);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setCursor(Qt::ArrowCursor);
    btn->setProperty("pillRole", pillRole);
    connect(btn, &QPushButton::clicked, this, &CalculatorWidget::onPillButtonClicked);
    return btn;
}

QString CalculatorWidget::xValue() const
{
    return m_currentInput.isEmpty() ? m_stack[0] : m_currentInput;
}

void CalculatorWidget::setWindowActive(bool active)
{
    applyButtonStyles(active);
}

void CalculatorWidget::setThousandsSeparatorEnabled(bool enabled)
{
    if (m_useThousandsSeparator == enabled) return;
    m_useThousandsSeparator = enabled;
    updateDisplay();
}

void CalculatorWidget::setRpnMode(bool rpn)
{
    if (m_rpnMode == rpn) return;
    m_rpnMode = rpn;
    if (!m_rpnMode) {
        m_infixExpression.clear();
        m_infixFirst.clear();
        m_infixOp.clear();
        if (m_currentInput.isEmpty() || m_currentInput == "0") m_currentInput = "0";
    }
    applyRpnModeUi();
    updateDisplay();
}

void CalculatorWidget::applyRpnModeUi()
{
    const bool showRpn = m_rpnMode;
    m_row2->setVisible(showRpn);
    m_row3->setVisible(showRpn);
    m_line1->setVisible(showRpn);
    m_line2->setVisible(showRpn);
    m_line3->setVisible(showRpn);
    m_pillContainer->setVisible(showRpn);
    m_enterEqualsButton->setText(showRpn ? "enter" : "=");
    if (!showRpn) {
        m_row4->setStyleSheet(
            "QLabel { color: #8e8e93; font-size: 20px; font-weight: 300; "
            "background: transparent; padding: 4px 8px; }");
    } else {
        m_row4->setStyleSheet(
            "QLabel { color: #ffffff; font-size: 32px; font-weight: 300; "
            "background: transparent; padding: 6px 8px; }");
    }
}

void CalculatorWidget::applyButtonStyles(bool active)
{
    QString base = "CalculatorWidget QPushButton { color: #ffffff; border: none; font-weight: 400; } ";
    QString numStyle, grayStyle, orangeStyle, pillStyle;
    const int roundRadius = 24;
    const int pillRadius = 16;
    if (active) {
        numStyle =
            "CalculatorWidget QPushButton#numeric { background-color: #4f4f51; border-radius: " + QString::number(roundRadius) + "px; font-size: 22px; } "
            "CalculatorWidget QPushButton#numeric:pressed { background-color: #5f5f61; } ";
        grayStyle =
            "CalculatorWidget QPushButton#grayOp { background-color: #7b7b7d; border-radius: " + QString::number(roundRadius) + "px; font-size: 22px; } "
            "CalculatorWidget QPushButton#grayOp:pressed { background-color: #8a8a8c; } ";
        orangeStyle =
            "CalculatorWidget QPushButton#orangeOp { background-color: #FF9D00; border-radius: " + QString::number(roundRadius) + "px; font-size: 22px; } "
            "CalculatorWidget QPushButton#orangeOp:pressed { background-color: #ffb033; } ";
        pillStyle =
            "CalculatorWidget QPushButton#pill { background-color: #363638; border-radius: " + QString::number(pillRadius) + "px; font-size: 12px; } "
            "CalculatorWidget QPushButton#pill:pressed { background-color: #454547; } ";
    } else {
        numStyle =
            "CalculatorWidget QPushButton#numeric { background-color: #333336; border-radius: " + QString::number(roundRadius) + "px; font-size: 22px; } "
            "CalculatorWidget QPushButton#numeric:pressed { background-color: #404042; } ";
        grayStyle =
            "CalculatorWidget QPushButton#grayOp { background-color: #3d3d3e; border-radius: " + QString::number(roundRadius) + "px; font-size: 22px; } "
            "CalculatorWidget QPushButton#grayOp:pressed { background-color: #4a4a4c; } ";
        orangeStyle =
            "CalculatorWidget QPushButton#orangeOp { background-color: #3d3d3e; border-radius: " + QString::number(roundRadius) + "px; font-size: 22px; } "
            "CalculatorWidget QPushButton#orangeOp:pressed { background-color: #4a4a4c; } ";
        pillStyle =
            "CalculatorWidget QPushButton#pill { background-color: #3d3d3e; border-radius: " + QString::number(pillRadius) + "px; font-size: 12px; } "
            "CalculatorWidget QPushButton#pill:pressed { background-color: #4a4a4c; } ";
    }
    setStyleSheet("CalculatorWidget { background: transparent; } " + base + numStyle + grayStyle + orangeStyle + pillStyle);
}

void CalculatorWidget::onDigitClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString role = btn->property("calcRole").toString();

    if (role == "⌫") {
        onClearClicked();
        return;
    }
    if (role == "AC") {
        onAllClearClicked();
        return;
    }
    if (role == "%") {
        onPercentClicked();
        return;
    }
    if (role == "÷" || role == "×" || role == "−" || role == "+") {
        if (!m_rpnMode) {
            QString cur = m_currentInput.isEmpty() ? "0" : m_currentInput;
            if (!m_infixOp.isEmpty() && !m_infixFirst.isEmpty()) {
                double res = evaluateInfix(m_infixFirst, m_infixOp, cur);
                m_infixFirst = formatNumber(res);
            } else {
                m_infixFirst = cur;
            }
            m_infixOp = role;
            m_currentInput.clear();
            m_hasDecimal = false;
            updateDisplay();
            return;
        }
        double xVal = toDouble(m_currentInput.isEmpty() ? m_stack[0] : m_currentInput);
        double yVal = toDouble(m_stack[1]);
        double res = 0;
        if (role == "÷") {
            if (xVal == 0)   // RPN: Y ÷ X, împărțitorul e X
                m_stack[0] = QStringLiteral("Undefined");
            else
                res = yVal / xVal, m_stack[0] = formatNumber(res);
        } else {
            if (role == "×") res = yVal * xVal;
            else if (role == "−") res = yVal - xVal;
            else if (role == "+") res = yVal + xVal;
            m_stack[0] = formatNumber(res);
        }
        m_stack[1] = m_stack[2];
        m_stack[2] = m_stack[3];
        m_stack[3].clear();
        m_currentInput.clear();
        m_hasDecimal = m_stack[0].contains(",");
        updateDisplay();
        return;
    }
    if (role == "±") {
        onNegateClicked();
        return;
    }
    if (role == ",") {
        onDecimalClicked();
        return;
    }
    if (role == "enter") {
        if (!m_rpnMode) {
            if (m_infixOp.isEmpty() || m_infixFirst.isEmpty()) {
                m_infixExpression.clear();
                updateDisplay();
                return;
            }
            QString cur = m_currentInput.isEmpty() ? "0" : m_currentInput;
            double res = evaluateInfix(m_infixFirst, m_infixOp, cur);
            m_infixExpression = formatForDisplay(m_infixFirst) + " " + m_infixOp + " " + formatForDisplay(cur);
            m_currentInput = formatNumber(res);
            m_infixFirst.clear();
            m_infixOp.clear();
            m_hasDecimal = m_currentInput.contains(",");
            updateDisplay();
            return;
        }
        onEnterClicked();
        return;
    }

    if (m_currentInput == "0" && role != "0")
        m_currentInput = role;
    else if (m_currentInput != "0" || role != "0")
        m_currentInput += role;
    updateDisplay();
}

void CalculatorWidget::onOperatorClicked() { /* handled in onDigitClicked */ }

void CalculatorWidget::onClearClicked()
{
    if (m_currentInput.isEmpty()) return;
    m_currentInput.chop(1);
    if (m_currentInput.isEmpty() || m_currentInput == "-")
        m_currentInput = "0";
    updateDisplay();
}

void CalculatorWidget::onAllClearClicked()
{
    m_currentInput = "0";
    for (int i = 0; i < 4; ++i) m_stack[i].clear();
    m_hasDecimal = false;
    if (!m_rpnMode) {
        m_infixExpression.clear();
        m_infixFirst.clear();
        m_infixOp.clear();
    }
    updateDisplay();
}

void CalculatorWidget::onPercentClicked()
{
    double v = currentValue();
    m_currentInput = formatNumber(v / 100.0);
    updateDisplay();
}

void CalculatorWidget::onNegateClicked()
{
    if (m_currentInput.isEmpty() || m_currentInput == "0") return;
    if (m_currentInput.startsWith("-"))
        m_currentInput = m_currentInput.mid(1);
    else
        m_currentInput.prepend("-");
    updateDisplay();
}

void CalculatorWidget::onDecimalClicked()
{
    if (m_currentInput.isEmpty() || m_currentInput == "-") {
        m_currentInput = "0,";
    } else if (!m_currentInput.contains(",")) {
        m_currentInput += ",";
    }
    m_hasDecimal = true;
    updateDisplay();
}

void CalculatorWidget::onEnterClicked()
{
    QString xVal = m_currentInput.isEmpty() ? m_stack[0] : m_currentInput;
    if (xVal.isEmpty()) xVal = "0";
    m_stack[3] = m_stack[2];
    m_stack[2] = m_stack[1];
    m_stack[1] = xVal;
    m_stack[0].clear();
    m_currentInput.clear();
    m_hasDecimal = false;
    updateDisplay();
}

void CalculatorWidget::onPillButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString role = btn->property("pillRole").toString();
    const QString x = xValue().isEmpty() ? "0" : xValue();

    if (role == "swap") {
        QString y = m_stack[1];
        m_stack[0] = y;
        m_stack[1] = x;
        m_currentInput.clear();
    } else if (role == "rollDown") {
        const QString oldT = m_stack[3];
        m_stack[3] = x;
        m_stack[0] = m_stack[1];
        m_stack[1] = m_stack[2];
        m_stack[2] = oldT;
        m_currentInput.clear();
    } else if (role == "rollUp") {
        const QString oldY = m_stack[1];
        const QString oldZ = m_stack[2];
        const QString oldT = m_stack[3];
        m_stack[0] = oldZ;
        m_stack[1] = oldT;
        m_stack[2] = x;
        m_stack[3] = oldY;
        m_currentInput.clear();
    } else if (role == "drop") {
        m_stack[0] = m_stack[1];
        m_stack[1] = m_stack[2];
        m_stack[2] = m_stack[3];
        m_currentInput.clear();
    }
    m_hasDecimal = false;
    updateDisplay();
    btn->clearFocus();
}

int CalculatorWidget::fontSizeForWidth(QLabel *label, const QString &text, int availableWidth) const
{
    if (text.isEmpty()) return kDisplayFontSizes[0];
    for (int i = 0; i < kDisplayFontSizeCount; ++i) {
        QFont f = label->font();
        f.setPixelSize(kDisplayFontSizes[i]);
        if (QFontMetrics(f).horizontalAdvance(text) <= availableWidth)
            return kDisplayFontSizes[i];
    }
    return kDisplayMinFontSize;
}

void CalculatorWidget::applyRowStyle(QLabel *label, const QString &text, int fontPx)
{
    label->setStyleSheet(QStringLiteral("QLabel { color: #ffffff; font-size: %1px; font-weight: 300; background: transparent; padding: 6px 8px; }").arg(fontPx));
    label->setText(text);
}

void CalculatorWidget::updateGradientOverlay()
{
    if (!m_gradientOverlay || !m_row1) return;
    if (m_row1NeedsGradient && m_row1->isVisible()) {
        m_gradientOverlay->setFixedHeight(m_row1->height());
        m_gradientOverlay->move(m_row1->x(), m_row1->y());
        m_gradientOverlay->raise();
        m_gradientOverlay->show();
    } else {
        m_gradientOverlay->hide();
    }
}

void CalculatorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    QTimer::singleShot(0, this, &CalculatorWidget::updateGradientOverlay);
}

void CalculatorWidget::updateDisplay()
{
    if (!m_rpnMode) {
        updateDisplayNormal();
        return;
    }
    const int availW = qMax(150, width() - 32);
    const QString row4Display = formatForDisplay(m_stack[3]);
    const QString row3Display = formatForDisplay(m_stack[2]);
    const QString row2Display = formatForDisplay(m_stack[1]);
    const QString row1Raw = m_currentInput.isEmpty() ? (m_stack[0].isEmpty() ? "0" : m_stack[0]) : m_currentInput;
    const QString row1Text = formatForDisplay(row1Raw);
    applyRowStyle(m_row4, row4Display, fontSizeForWidth(m_row4, row4Display, availW));
    applyRowStyle(m_row3, row3Display, fontSizeForWidth(m_row3, row3Display, availW));
    applyRowStyle(m_row2, row2Display, fontSizeForWidth(m_row2, row2Display, availW));
    int row1Font = fontSizeForWidth(m_row1, row1Text, availW);
    QFont f1 = m_row1->font();
    f1.setPixelSize(kDisplayMinFontSize);
    m_row1NeedsGradient = (row1Font == kDisplayMinFontSize && QFontMetrics(f1).horizontalAdvance(row1Text) > availW);
    applyRowStyle(m_row1, row1Text, row1Font);
    QTimer::singleShot(0, this, &CalculatorWidget::updateGradientOverlay);
}

void CalculatorWidget::updateDisplayNormal()
{
    const int availW = qMax(150, width() - 32);
    applyRowStyle(m_row4, m_infixExpression, fontSizeForWidth(m_row4, m_infixExpression, availW));
    QString mainText;
    if (m_infixFirst.isEmpty() && m_infixOp.isEmpty())
        mainText = formatForDisplay(m_currentInput.isEmpty() ? QStringLiteral("0") : m_currentInput);
    else
        mainText = formatForDisplay(m_infixFirst) + " " + m_infixOp + (m_currentInput.isEmpty() ? "" : " " + formatForDisplay(m_currentInput));
    int row1Font = fontSizeForWidth(m_row1, mainText, availW);
    QFont f1 = m_row1->font();
    f1.setPixelSize(kDisplayMinFontSize);
    m_row1NeedsGradient = (row1Font == kDisplayMinFontSize && QFontMetrics(f1).horizontalAdvance(mainText) > availW);
    applyRowStyle(m_row1, mainText, row1Font);
    QTimer::singleShot(0, this, &CalculatorWidget::updateGradientOverlay);
}

double CalculatorWidget::evaluateInfix(const QString &a, const QString &op, const QString &b) const
{
    double x = toDouble(a);
    double y = toDouble(b);
    if (op == "÷") return (y != 0) ? (x / y) : std::numeric_limits<double>::quiet_NaN();
    if (op == "×") return x * y;
    if (op == "−") return x - y;
    if (op == "+") return x + y;
    return x;
}

double CalculatorWidget::currentValue() const
{
    QString s = m_currentInput.isEmpty() ? m_stack[0] : m_currentInput;
    return toDouble(s.isEmpty() ? "0" : s);
}

double CalculatorWidget::toDouble(const QString &s)
{
    if (s.isEmpty()) return 0;
    QString t = s;
    t.remove(QLatin1Char('.'));  // thousands separator
    t.replace(QLatin1Char(','), QLatin1Char('.'));
    return t.toDouble();
}

QString CalculatorWidget::formatNumber(double value) const
{
    if (std::isnan(value)) return QStringLiteral("Undefined");
    QString s = QString::number(value);
    if (s.contains('.')) {
        while (s.endsWith('0') && !s.endsWith(".0"))
            s.chop(1);
        if (s.endsWith('.'))
            s.chop(1);
    }
    s.replace('.', ',');
    return s;
}

QString CalculatorWidget::formatForDisplay(const QString &numStr) const
{
    if (!m_useThousandsSeparator || numStr.isEmpty()) return numStr;
    int commaPos = numStr.indexOf(QLatin1Char(','));
    QString intPart = commaPos < 0 ? numStr : numStr.left(commaPos);
    QString decPart = commaPos < 0 ? QString() : numStr.mid(commaPos);
    if (intPart.startsWith(QLatin1Char('-'))) {
        for (int i = intPart.length() - 3; i > 1; i -= 3)
            intPart.insert(i, QLatin1Char('.'));
    } else {
        for (int i = intPart.length() - 3; i > 0; i -= 3)
            intPart.insert(i, QLatin1Char('.'));
    }
    return intPart + decPart;
}
