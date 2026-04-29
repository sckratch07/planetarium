#include <main_window.hpp>
#include <map_preview.hpp>

#include <tool_bar.hpp>

#include <QSettings>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent)
{
    QSettings settings("Game Academy", "EarthPower");

    settings.beginGroup("main_window");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();

    auto* tool_bar = new ToolBar(this);
    addToolBar(tool_bar);

    auto* map_preview = new MapPreview(this);
    setCentralWidget(map_preview);
}

MainWindow::~MainWindow()
{
    QSettings settings("Game Academy", "EarthPower");

    settings.beginGroup("main_window");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}