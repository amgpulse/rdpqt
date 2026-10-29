#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Metadata for QSettings and desktop environment
    app.setApplicationName("rdpqt");
    app.setApplicationDisplayName("rdpqt");
    app.setOrganizationName("FreeRDPClient");
    app.setApplicationVersion("1.0.0");

    // Use Fusion style for a clean, consistent cross-desktop look on Linux
    if (QStyleFactory::keys().contains("Fusion", Qt::CaseInsensitive)) {
        app.setStyle(QStyleFactory::create("Fusion"));
    }

    MainWindow window;
    window.show();

    return app.exec();
}
