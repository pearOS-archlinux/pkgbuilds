#ifndef HOME_WIDGET_H
#define HOME_WIDGET_H

#include <QWidget>
#include <QVector>
#include <QPair>
#include <QScrollArea>
#include <QGridLayout>
#include <QTimer>
#include <QStackedWidget>
#include <QFrame>
#include <QLabel>
#include "../utils/types.h"

class PackageCard;
class LoadingOverlay;

/**
 * @brief Widget displaying featured packages on the home screen.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 * - m_packageCards contains non-owning pointers to cards owned by m_contentWidget
 */
class HomeWidget : public QWidget {
    Q_OBJECT
    
signals:
    void openPackageRequested(const PackageInfo& info);
    
public:
    explicit HomeWidget(QWidget* parent = nullptr);
    ~HomeWidget() override = default;

    /** Reîmprospătează lista Discover din repos (apelat după ce ALPM e inițializat). */
    void refreshWhenRepositoriesReady();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi();
    void loadFeaturedPackages();
    void createPackageCards();
    void buildFeaturedListAndRefreshUi(const QVector<QPair<QString, QString>>& featured);
    void checkInstalledPackages();
    void refreshGridLayout();
    void onInitialLoad();

    static int columnCountForWidth(int width);

    QVector<PackageInfo> m_featuredPackages;
    QVector<PackageCard*> m_packageCards;  // Non-owning pointers, owned by m_contentWidget
    int m_columnCount = 2;

    // Qt parent-child managed widgets (non-owning pointers)
    QStackedWidget* m_contentStack = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QGridLayout* m_gridLayout = nullptr;
    QTimer* m_updateTimer = nullptr;
    QFrame* m_bannerCard = nullptr;
    QFrame* m_promoCard1 = nullptr;
    QFrame* m_promoCard2 = nullptr;
    QLabel* m_bannerImage = nullptr;
    QLabel* m_promoImage1 = nullptr;
    QLabel* m_promoImage2 = nullptr;

    void loadPromoImages();

private slots:
    void onPackageClicked(const PackageInfo& info);
    void onUpdateTimer();
};

#endif // HOME_WIDGET_H
