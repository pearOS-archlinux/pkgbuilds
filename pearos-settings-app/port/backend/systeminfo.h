#pragma once
#include <QObject>
#include <functional>

class SystemInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString osName      READ osName      NOTIFY infoChanged)
    Q_PROPERTY(QString osVersion   READ osVersion   NOTIFY infoChanged)
    Q_PROPERTY(QString kernelVersion READ kernelVersion NOTIFY infoChanged)
    Q_PROPERTY(QString cpuModel    READ cpuModel    NOTIFY infoChanged)
    Q_PROPERTY(int     cpuCores    READ cpuCores    NOTIFY infoChanged)
    Q_PROPERTY(QString gpuModel    READ gpuModel    NOTIFY infoChanged)
    Q_PROPERTY(qint64  totalRam    READ totalRam    NOTIFY infoChanged)
    Q_PROPERTY(qint64  usedRam     READ usedRam     NOTIFY infoChanged)
    Q_PROPERTY(qint64  totalDisk   READ totalDisk   NOTIFY infoChanged)
    Q_PROPERTY(qint64  usedDisk    READ usedDisk    NOTIFY infoChanged)
    Q_PROPERTY(QString diskName    READ diskName    NOTIFY infoChanged)
    Q_PROPERTY(QString hostName    READ hostName    NOTIFY infoChanged)
    Q_PROPERTY(QString serialNumber READ serialNumber NOTIFY infoChanged)
    Q_PROPERTY(double storageApps      READ storageApps      NOTIFY storageChanged)
    Q_PROPERTY(double storageDocuments READ storageDocuments NOTIFY storageChanged)
    Q_PROPERTY(double storagePhotos    READ storagePhotos    NOTIFY storageChanged)
    Q_PROPERTY(double storageDownloads READ storageDownloads NOTIFY storageChanged)
    Q_PROPERTY(double storageDesktop   READ storageDesktop   NOTIFY storageChanged)
    Q_PROPERTY(double storageSystem    READ storageSystem    NOTIFY storageChanged)

public:
    explicit SystemInfo(QObject *parent = nullptr);
    QString osName()        const { return m_osName; }
    QString osVersion()     const { return m_osVersion; }
    QString kernelVersion() const { return m_kernelVersion; }
    QString cpuModel()      const { return m_cpuModel; }
    int     cpuCores()      const { return m_cpuCores; }
    QString gpuModel()      const { return m_gpuModel; }
    qint64  totalRam()      const { return m_totalRam; }
    qint64  usedRam()       const { return m_usedRam; }
    qint64  totalDisk()     const { return m_totalDisk; }
    qint64  usedDisk()      const { return m_usedDisk; }
    QString diskName()      const { return m_diskName; }
    QString hostName()      const { return m_hostName; }
    QString serialNumber()  const { return m_serialNumber; }
    double storageApps()      const { return m_storageApps; }
    double storageDocuments() const { return m_storageDocs; }
    double storagePhotos()    const { return m_storagePhotos; }
    double storageDownloads() const { return m_storageDownloads; }
    double storageDesktop()   const { return m_storageDesktop; }
    double storageSystem()    const { return m_storageSystem; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshStorage();

signals:
    void infoChanged();
    void storageChanged();

private:
    QString m_osName, m_osVersion, m_kernelVersion;
    QString m_cpuModel, m_gpuModel, m_hostName, m_serialNumber, m_diskName;
    int     m_cpuCores = 0;
    qint64  m_totalRam = 0, m_usedRam = 0, m_totalDisk = 0, m_usedDisk = 0;
    double  m_storageApps = 0, m_storageDocs = 0, m_storagePhotos = 0;
    double  m_storageDownloads = 0, m_storageDesktop = 0, m_storageSystem = 0;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
