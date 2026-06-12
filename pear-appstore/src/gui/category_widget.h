#ifndef CATEGORY_WIDGET_H
#define CATEGORY_WIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QVector>
#include <QPair>
#include <QSet>
#include "../utils/types.h"

class PackageCard;
class HeroCard;

struct AppSection {
    QString title;
    QVector<QPair<QString, QString>> apps; // {name, repo}
    QString featuredApp;   // name of the hero app (optional, first section only)
};

class CategoryWidget : public QWidget {
    Q_OBJECT

public:
    explicit CategoryWidget(const QString& categoryTitle,
                            const QString& categorySubtitle,
                            const QVector<AppSection>& sections,
                            QWidget* parent = nullptr);
    ~CategoryWidget() override = default;

signals:
    void openPackageRequested(const PackageInfo& info);

private:
    void buildUi(const QString& title, const QString& subtitle,
                 const QVector<AppSection>& sections);
    QWidget* buildGridSection(const QVector<QPair<QString,QString>>& apps, QWidget* parent);
    void updateAllInstallStatus(const QSet<QString>& installed);

    QVBoxLayout*        m_mainLayout = nullptr;
    QVector<PackageCard*> m_allCards;
    QVector<HeroCard*>    m_heroCards;
};

#endif // CATEGORY_WIDGET_H
