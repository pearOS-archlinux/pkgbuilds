#pragma once
#include <QObject>
#include <functional>

// Controls the two macOS-style scrollbar behaviors that actually exist on
// this stack. There is no Qt/Breeze equivalent — QtWidgets/Kirigami scrollbars
// have no user-facing auto-hide or click-to-jump toggle — so this only ever
// affects GTK3/GTK4 applications (Files, GTK-based browsers, etc).
class ScrollbarManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool alwaysVisible READ alwaysVisible NOTIFY changed)
    Q_PROPERTY(bool clickToJump   READ clickToJump   NOTIFY changed)

public:
    explicit ScrollbarManager(QObject *parent = nullptr);

    bool alwaysVisible() const { return m_alwaysVisible; }
    bool clickToJump()   const { return m_clickToJump; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setAlwaysVisible(bool v);
    Q_INVOKABLE void setClickToJump(bool v);

signals:
    void changed();

private:
    bool m_alwaysVisible = false;
    bool m_clickToJump   = true;

    void writeGtkKey(const QString &key, const QString &value) const;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
