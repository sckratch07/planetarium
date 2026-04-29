#include <main_window.hpp>

#include <tool_bar.hpp>
#include <map_preview.hpp>
#include <grid.hpp>
#include <palette.hpp>

#include <QSettings>
#include <QDockWidget>

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

    auto* grid = new Grid(this);
    auto* grid_dock = new QDockWidget("Grid", this);
    grid_dock->setObjectName("GridDock");
    grid_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    grid_dock->setWidget(grid);

    addDockWidget(Qt::LeftDockWidgetArea, grid_dock);

    connect(&map_preview->sfml_window(), &SfmlWindow::rendered, grid, &Grid::draw);
}

MainWindow::~MainWindow()
{
    QSettings settings("Game Academy", "EarthPower");

    settings.beginGroup("main_window");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}