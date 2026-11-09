#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"

#ifndef RDPQT_VERSION_STRING
#define RDPQT_VERSION_STRING "1.4.0"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Metadata for QSettings and desktop environment
    app.setApplicationName("rdpqt");
    app.setApplicationDisplayName("rdpqt");
    app.setOrganizationName("FreeRDPClient");
    app.setApplicationVersion(QStringLiteral(RDPQT_VERSION_STRING));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app-icon")));

    // Use Fusion style for a clean, consistent cross-desktop look on Linux
    if (QStyleFactory::keys().contains("Fusion", Qt::CaseInsensitive)) {
        app.setStyle(QStyleFactory::create("Fusion"));
    }

    MainWindow window;
    window.show();

    return app.exec();
}
