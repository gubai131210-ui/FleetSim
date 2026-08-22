#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("FleetSim"));
    QApplication::setOrganizationName(QStringLiteral("FleetSim"));

    fleetsim::ui::MainWindow window;
    window.show();
    return QApplication::exec();
}
