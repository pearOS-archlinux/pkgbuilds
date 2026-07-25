#pragma once
#include <QObject>
#include <QVariantList>

// Generic live editor for Filer's own settings.conf (~/.config/filer/default/settings.conf).
// Reads every [Group]/Key=Value pair, infers a UI control per value (bool/color/int/string),
// and writes changes straight back via kwriteconfig6 — Filer picks up the file change itself.
class FilerConfigManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList groups READ groups NOTIFY changed)

public:
    explicit FilerConfigManager(QObject *parent = nullptr);

    QVariantList groups() const { return m_groups; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setValue(const QString &group, const QString &key, const QVariant &value);

signals:
    void changed();

private:
    QVariantList m_groups;
    static QString configRelPath();
};
