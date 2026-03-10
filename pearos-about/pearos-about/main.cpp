#include "AboutWindow.h"

#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AboutWindow window;

    const QSize size = window.size();
    const QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    const int x = screenGeometry.center().x() - size.width() / 2;
    const int y = screenGeometry.center().y() - size.height() / 2;
    window.move(x, y);

    window.show();

    return app.exec();
}

