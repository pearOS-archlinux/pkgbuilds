#ifndef AUR_HELPER_H
#define AUR_HELPER_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <memory>
#include "../utils/types.h"

/**
 * @brief Helper class for interacting with the Arch User Repository (AUR).
 * 
 * Memory Management:
 * - m_networkManager: Owned by std::unique_ptr for RAII-style cleanup
 * - Network replies are managed via Qt parent-child and deleteLater()
 */
class AurHelper : public QObject {
    Q_OBJECT
    
public:
    explicit AurHelper(QObject* parent = nullptr);
    ~AurHelper() override;
    
    void searchPackages(const QString& query);
    void getPackageInfo(const QString& packageName);
    QVector<UpdateInfo> checkAurUpdates();
    
signals:
    void searchCompleted(const QVector<PackageInfo>& results);
    void packageInfoReceived(const PackageInfo& info);
    void error(const QString& message);
    
private slots:
    void onSearchFinished();
    void onPackageInfoFinished();
    
private:
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    
    PackageInfo parseAurPackage(const QJsonObject& obj);
};

#endif // AUR_HELPER_H
