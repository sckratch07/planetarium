#include <main_window.hpp>

#include <tool_bar.hpp>
#include <map_preview.hpp>
#include <grid.hpp>
#include <palette.hpp>
#include <tilemap.hpp>
#include <layer_panel.hpp>

#include <QSettings>
#include <QDockWidget>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
    setAnimated(false);
    
    auto* tool_bar = new ToolBar(this);
    addToolBar(tool_bar);
    
    auto* map_preview = new MapPreview(this);
    // auto* map_preview_dock = new QDockWidget("Map Preview", this);
    // map_preview_dock->setObjectName("MapPreviewDock");
    // map_preview_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    // map_preview_dock->setWidget(map_preview);
    setCentralWidget(map_preview);
    
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

    auto* layer_panel = new LayerPanel(this);
    auto* layer_panel_dock = new QDockWidget("Layers", this);
    layer_panel_dock->setObjectName("LayerPanelDock");
    layer_panel_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    layer_panel_dock->setWidget(layer_panel);

    auto* tilemap = new Tilemap(this);
    
    addDockWidget(Qt::LeftDockWidgetArea, grid_dock);
    addDockWidget(Qt::LeftDockWidgetArea, layer_panel_dock);
    //addDockWidget(Qt::LeftDockWidgetArea, map_preview_dock);
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
    connect(grid, &Grid::cell_size_changed, palette->view(), &TilesetView::tile_size_changed);

    connect(palette, &Palette::texture_changed, tilemap, &Tilemap::texture_changed);
    connect(palette->view(), &TilesetView::selected_rect_changed, tilemap, &Tilemap::selected_rect_changed);

    connect(palette, &Palette::tileset_selected, grid, [grid](bool selected)
        {
            if (selected) grid->clear_type_selection();
        }
    );

    connect(grid, &Grid::tile_type_changed, palette, &Palette::clear_tileset_selection);
    connect(grid, &Grid::tile_type_changed, tilemap, &Tilemap::selected_type_changed);
    connect(grid, &Grid::tile_type_selection_cleared, tilemap, &Tilemap::clear_selected_type);
    connect(grid, &Grid::tile_type_removed, tilemap, &Tilemap::removed_type);

    connect(layer_panel, &LayerPanel::layer_added, tilemap, &Tilemap::layer_added);
    connect(layer_panel, &LayerPanel::layer_removed, tilemap, &Tilemap::layer_removed);
    connect(layer_panel, &LayerPanel::layer_selected, tilemap, &Tilemap::layer_selected);
    connect(layer_panel, &LayerPanel::layer_visibility_changed, tilemap, &Tilemap::layer_visibility_changed);

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