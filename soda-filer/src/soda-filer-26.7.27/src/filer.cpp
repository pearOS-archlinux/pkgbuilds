#include <libfm/fm.h>
#include "application.h"
#include "libfmqt.h"
#include <QDebug>
#include <QLibraryInfo>
#include <QMessageBox>

int main(int argc, char** argv) {
  // ensure that glib integration of Qt is not turned off
  // This fixes #168: https://github.com/lxde/filer-qt/issues/168
  qunsetenv("QT_NO_GLIB");

  // Finder-like chrome: MainWindow hides its own in-window QMenuBar (see
  // mainwindow.cpp) so the only visible menu is filer-topbar's global
  // org.kde.plasma.appmenu applet -- but that only works if this process's
  // own QMenuBar is actually exported over DBusMenu, which needs the KDE
  // Plasma Qt platform theme active (its plugin key is "kde", provided by
  // /usr/lib/qt6/plugins/platformthemes/KDEPlasmaPlatformTheme6.so -- a
  // standard Qt6 plugin path, already searched with no QT_PLUGIN_PATH
  // changes needed). A full Plasma session sets QT_QPA_PLATFORMTHEME=kde
  // itself, so this is only a fallback for setups like pearOS's bare-KWin
  // session that don't set it -- must run before QApplication/Application
  // is constructed, and must not clobber a value the session already chose.
  if(qEnvironmentVariableIsEmpty("QT_QPA_PLATFORMTHEME"))
      qputenv("QT_QPA_PLATFORMTHEME", "kde");

  Filer::Application app(argc, argv);
  app.init();


  QTranslator *qtTranslator = new QTranslator(&app);
  QTranslator *translator = new QTranslator(&app);

  // Install the translations built-into Qt itself
  if (! qtTranslator->load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath))){
      // Other than qDebug, qCritical also works in Release builds
      qCritical() << "Failed qtTranslator->load";
  } else {
      if (! qApp->installTranslator(qtTranslator)){
          qCritical() << "Failed qApp->installTranslator(qtTranslator)";
      }
  }

  // Install our own translations
  if (! translator->load("filer-qt_" + QLocale::system().name(), QCoreApplication::applicationDirPath() + QString("/Resources/translations/"))) { // probono: .app bundle
      if (! translator->load("filer-qt_" + QLocale::system().name(), QCoreApplication::applicationDirPath())) { // probono: When qm files are next to the executable ("uninstalled"), useful during development
          qCritical() << "Failed translator->load";
      }
  }

  if (! translator->isEmpty()) {
      if (! qApp->installTranslator(translator)){
          qCritical() << "Failed qApp->installTranslator(translator)";
      }
  }

  return app.exec();

}
