#pragma once

#include <QWidget>
#include <QPoint>
#include <QEvent>

class QLabel;
class QPushButton;

class AboutWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AboutWindow(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void buildUi();
    void applySystemTheme();
    void loadSystemInfo();
    void detectBatteryAndDeviceType();
    void setupConnections();

    QString runCommand(const QString &command, int timeoutMs = 4000) const;
    QString resolveAssetPath(const QString &fileName) const;
    QString formatChipName(const QString &cpuName) const;

    void openSystemSettingsAbout();

    QWidget *m_titleBar = nullptr;
    QLabel *m_deviceImage = nullptr;
    QLabel *m_deviceType = nullptr;
    QLabel *m_chipLabel = nullptr;
    QLabel *m_memoryLabel = nullptr;
    QLabel *m_serialLabel = nullptr;
    QLabel *m_osNameLabel = nullptr;
    QLabel *m_osVersionLabel = nullptr;
    QLabel *m_footerLabel = nullptr;
    QPushButton *m_moreInfoButton = nullptr;

    QPoint m_dragStartPos;
    bool m_dragging = false;
};

