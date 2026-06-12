#include <QApplication>
#include <QGuiApplication>
#include "NotchWindow.h"
#include "SettingsDialog.h"
#include "Config.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("pearos-notch");
    app.setOrganizationName("pearos-notch");
    app.setQuitOnLastWindowClosed(false);

    SettingsDialog settings;
    NotchWindow    notch(&settings);

    QObject::connect(&settings, &SettingsDialog::modeChanged,
                     &notch,    &NotchWindow::reinitForMode);

    notch.show();
    return app.exec();
}
