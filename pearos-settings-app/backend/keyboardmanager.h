#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class KeyboardManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString layout        READ layout        NOTIFY keyboardChanged)
    Q_PROPERTY(QString variant       READ variant       NOTIFY keyboardChanged)
    Q_PROPERTY(bool   repeatEnabled  READ repeatEnabled NOTIFY keyboardChanged)
    Q_PROPERTY(int    repeatDelay    READ repeatDelay   NOTIFY keyboardChanged)
    Q_PROPERTY(int    repeatRate     READ repeatRate    NOTIFY keyboardChanged)
    Q_PROPERTY(QVariantList layouts  READ layouts       NOTIFY keyboardChanged)

public:
    explicit KeyboardManager(QObject *parent = nullptr);
    QString layout()       const { return m_layout; }
    QString variant()      const { return m_variant; }
    bool   repeatEnabled() const { return m_repeatEnabled; }
    int    repeatDelay()   const { return m_repeatDelay; }
    int    repeatRate()    const { return m_repeatRate; }
    QVariantList layouts() const { return m_layouts; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setLayout(const QString &layout, const QString &variant = {});
    Q_INVOKABLE void setRepeatEnabled(bool enabled);
    Q_INVOKABLE void setRepeat(int delayMs, int rateHz);

signals:
    void keyboardChanged();

private:
    QString m_layout, m_variant;
    bool m_repeatEnabled = true;
    int m_repeatDelay = 400, m_repeatRate = 25;
    QVariantList m_layouts;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
