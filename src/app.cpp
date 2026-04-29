#include <main_window.hpp>
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Game Academy");
    QCoreApplication::setApplicationName("EarthPower");

    QApplication a(argc, argv);

    MainWindow main_window;

    main_window.show();
    return a.exec();
}