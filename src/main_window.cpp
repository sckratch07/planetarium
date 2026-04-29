#include <main_window.hpp>

#include <tool_bar.hpp>
#include <map_preview.hpp>
#include <grid.hpp>
#include <palette.hpp>
#include <tilemap.hpp>

#include <QSettings>
#include <QDockWidget>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
    setAnimated(false);
    
    QWidget* central = new QWidget(this);
    central->setFixedSize(0, 0);
    setCentralWidget(central);
    
    auto* tool_bar = new ToolBar(this);
    addToolBar(tool_bar);
    
    auto* map_preview = new MapPreview(this);
    auto* map_preview_dock = new QDockWidget("Map Preview", this);
    map_preview_dock->setObjectName("MapPreviewDock");
    map_preview_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    map_preview_dock->setWidget(map_preview);
    
    auto* grid = new Grid(this);
    auto* grid_dock = new QDockWidget("Grid", this);
    grid_dock->setObjectName("GridDock");
    grid_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    grid_dock->setWidget(grid);

    auto* palette = new Palette(this);
    auto* palette_dock = new QDockWidget("Palette", this);
    palette_dock->setObjectName("PaletteDock");
    palette_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    palette_dock->setWidget(palette);

    auto* tilemap = new Tilemap(this);
    
    addDockWidget(Qt::LeftDockWidgetArea, grid_dock);
    addDockWidget(Qt::LeftDockWidgetArea, map_preview_dock);
    addDockWidget(Qt::RightDockWidgetArea, palette_dock);
    
    QSettings settings("Game Academy", "Planetarium");
    settings.beginGroup("main_window");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();

    SfmlWindow* sfml_window = &map_preview->sfml_window();

    connect(sfml_window, &SfmlWindow::rendered, grid, &Grid::draw);

    connect(sfml_window, &SfmlWindow::rendered, tilemap, &Tilemap::draw);
    connect(sfml_window, &SfmlWindow::event, tilemap, &Tilemap::event);

    connect(grid, &Grid::grid_size_changed, tilemap, &Tilemap::grid_size_changed);
    connect(grid, &Grid::cell_size_changed, tilemap, &Tilemap::cell_size_changed);

    connect(palette, &Palette::texture_changed, tilemap, &Tilemap::texture_changed);
    connect(palette->view(), &TilesetView::selected_rect_changed, tilemap, &Tilemap::selected_rect_changed);

    auto* save_action = new QAction("Save", tool_bar);
    save_action->setShortcut(QKeySequence("Ctrl + S"));
    tool_bar->addAction(save_action);

    connect(save_action, &QAction::triggered, tilemap, &Tilemap::save);

    emit grid->grid_size_changed(grid->grid_size());
    emit grid->cell_size_changed(grid->cell_size());
}

MainWindow::~MainWindow()
{
    QSettings settings("Game Academy", "Planetarium");

    settings.beginGroup("main_window");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}