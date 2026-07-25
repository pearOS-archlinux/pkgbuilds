#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class PrintersManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList printers READ printers NOTIFY changed)
    Q_PROPERTY(QVariantList jobs      READ jobs      NOTIFY jobsChanged)
    Q_PROPERTY(QString defaultPrinter READ defaultPrinter NOTIFY changed)

public:
    explicit PrintersManager(QObject *parent = nullptr);

    QVariantList printers()      const { return m_printers; }
    QVariantList jobs()          const { return m_jobs; }
    QString      defaultPrinter() const { return m_defaultPrinter; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshJobs();
    Q_INVOKABLE void setDefaultPrinter(const QString &name);
    Q_INVOKABLE void setPrinterEnabled(const QString &name, bool enabled);
    Q_INVOKABLE void removePrinter(const QString &name);
    Q_INVOKABLE void addPrinterByUri(const QString &name, const QString &uri);
    Q_INVOKABLE void cancelJob(const QString &jobId);

signals:
    void changed();
    void jobsChanged();
    void actionResult(bool success, const QString &message);

private:
    QVariantList m_printers;
    QVariantList m_jobs;
    QString m_defaultPrinter;
    void run(const QString &cmd, std::function<void(QString, int)> cb);
};
