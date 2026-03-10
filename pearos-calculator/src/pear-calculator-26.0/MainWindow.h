#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>

class QPushButton;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void setupTrafficLights();
    void setTrafficLightsActive(bool active);
    void updateMask();
    void loadSettings(bool *outRpnMode, bool *outThousandsSeparator) const;
    void saveSettings(bool rpnMode, bool thousandsSeparator) const;
    QRegion roundedRegion(const QRect &rect, int radius) const;

    static const QString kSettingsPath;

    QWidget *m_calculatorPanel;
    QWidget *m_outlineOverlay;
    QWidget *m_trafficContainer;
    QPushButton *m_btnClose;
    QPushButton *m_btnMinimize;
    QPushButton *m_btnMaximize;
    QPushButton *m_btnAppIcon;
    QPoint m_dragPos;
    bool m_dragging;
    static constexpr int kCornerRadius = 24;
    static constexpr int kTrafficLightSize = 12;
    static constexpr int kTrafficLightSpacing = 8;
    static constexpr int kAppIconSize = 38;
};

#endif // MAINWINDOW_H
