#ifndef CALCULATORWIDGET_H
#define CALCULATORWIDGET_H

#include <QWidget>
#include <QString>

class QLabel;
class QGridLayout;
class QPushButton;
class QFrame;

class CalculatorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CalculatorWidget(QWidget *parent = nullptr);
    void setWindowActive(bool active);
    void setRpnMode(bool rpn);
    bool rpnMode() const { return m_rpnMode; }
    void setThousandsSeparatorEnabled(bool enabled);

private slots:
    void onDigitClicked();
    void onOperatorClicked();
    void onClearClicked();
    void onAllClearClicked();
    void onPercentClicked();
    void onNegateClicked();
    void onDecimalClicked();
    void onEnterClicked();
    void onPillButtonClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void buildUI();
    QPushButton *addRoundButton(const QString &text, const QString &bgColor,
                                const QString &buttonRole,
                                const QString &fontColor = "#ffffff",
                                int minSize = 48);
    QPushButton *addPillButton(const QString &text, const QString &pillRole, int minHeight = 40);
    QString xValue() const;   // valoarea curentă din X (m_currentInput sau m_stack[0])
    QFrame *makeLine();
    void updateDisplay();
    void applyButtonStyles(bool active);
    double currentValue() const;
    static double toDouble(const QString &s);
    QString formatNumber(double value) const;
    QString formatForDisplay(const QString &numStr) const;

    void applyRpnModeUi();
    void updateDisplayNormal();
    double evaluateInfix(const QString &a, const QString &op, const QString &b) const;
    int fontSizeForWidth(QLabel *label, const QString &text, int availableWidth) const;
    void applyRowStyle(QLabel *label, const QString &text, int fontPx);
    void updateGradientOverlay();

    QLabel *m_row1;  // X – registrul curent (unde tastezi)
    QLabel *m_row2;  // Y – numărul introdus anterior
    QLabel *m_row3;  // Z – nivel superior stivă
    QLabel *m_row4;  // T – vârful stivei / (mod normal) expresia de sus
    QFrame *m_line1;
    QFrame *m_line2;
    QFrame *m_line3;
    QWidget *m_pillContainer;
    QPushButton *m_enterEqualsButton;
    QWidget *m_gradientOverlay;
    bool m_rpnMode;
    bool m_row1NeedsGradient;
    bool m_useThousandsSeparator;

    QString m_currentInput;   // ce tastezi acum (în X)
    QString m_stack[4];       // X=0, Y=1, Z=2, T=3 (X poate fi și m_currentInput)
    bool m_hasDecimal;

    QString m_infixExpression;  // mod normal: expresia afișată sus (ex. "9 × 3")
    QString m_infixFirst;       // primul operand pentru următoarea operație
    QString m_infixOp;          // operator pending (×, ÷, −, +)
};

#endif // CALCULATORWIDGET_H
