#include <palette.hpp>
#include <tileset_view.hpp>

#include <algorithm>
#include <QListWidget>
#include <QSettings>
#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QFileDialog>

#include <SFML/Graphics/Texture.hpp>

Palette::Palette(QWidget* parent) :
    QWidget(parent), m_view(this), m_tile_size({32, 32}), m_tree_view(new QListWidget(this)),
    m_mode_button(new QPushButton("Mode: Tile by Tile", this)),
    m_unselect_button(new QPushButton("Deselect", this))
{
    QSettings settings("Game Academy", "Planetarium");
    settings.beginGroup("palette");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();

    m_tree_view->setSizePolicy({QSizePolicy::Preferred, QSizePolicy::Preferred});
    m_tree_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    auto* load_button = new QPushButton("Add Tileset", this);
    load_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto* remove_button = new QPushButton("Remove Tileset", this);
    remove_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto* tileset_menu_layout = new QHBoxLayout();
    tileset_menu_layout->addWidget(load_button);
    tileset_menu_layout->addWidget(remove_button);

    m_mode_button->setCheckable(true);
    m_mode_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_unselect_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto* tileset_settings_layout = new QHBoxLayout();
    tileset_menu_layout->addWidget(m_mode_button);
    tileset_menu_layout->addWidget(m_unselect_button);

    auto* main_layout = new QVBoxLayout(this);
    main_layout->addLayout(tileset_menu_layout);
    main_layout->addLayout(tileset_settings_layout);
    main_layout->addWidget(m_tree_view);

    auto* scene = new QGraphicsScene(this);
    m_view.setScene(scene);
    main_layout->addWidget(&m_view);

    connect(load_button, &QPushButton::pressed, this, [this, scene]
        {
            if (QString file_path = QFileDialog::getOpenFileName(this, "Load Spritesheet", "", "Images (*.png *.xpm *.jpg)"); !file_path.isEmpty())
            {
                QPixmap pixmap(file_path);
                if (pixmap.isNull()) return;

                auto* item = new QListWidgetItem(QIcon(pixmap), file_path, m_tree_view);
                m_tree_view->addItem(item);
                item->setSelected(true);

                m_view.reset_rect();
                scene->clear();
                scene->addPixmap(pixmap);
                scene->setSceneRect(pixmap.rect());

                m_view.fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
                m_view.set_current_image(pixmap.toImage());
            }
        }
    );

    connect(remove_button, &QPushButton::pressed, this, [this, scene]
        {
            const auto selected = m_tree_view->selectedItems();
            if (selected.isEmpty())
                return;

            auto* item = selected[0];
            int row = m_tree_view->row(item);
            delete m_tree_view->takeItem(row);

            if (m_tree_view->count() > 0)
            {
                int next_row = std::min(row, m_tree_view->count() - 1);
                auto* next_item = m_tree_view->item(next_row);
                if (next_item)
                    next_item->setSelected(true);
            }
            else
            {
                m_view.reset_rect();
                scene->clear();
                emit tileset_selected(false);
            }
        }
    );

    connect(m_mode_button, &QPushButton::toggled, this, [this](bool checked)
        {
            m_view.set_auto_selection_mode(checked);
            m_mode_button->setText(checked ? "Mode: Auto Selection" : "Mode: Tile by Tile");
            emit selection_mode_changed(checked);
        }
    );

    connect(m_unselect_button, &QPushButton::pressed, this, [this]
        {
            m_view.reset_rect();
        }
    );

    connect(m_tree_view, &QListWidget::itemSelectionChanged, this, [this, scene]()
        {
            const auto selected = m_tree_view->selectedItems();
            if (selected.isEmpty())
            {
                scene->clear();
                m_view.reset_rect();
                emit tileset_selected(false);
                return;
            }

            auto* item = selected[0];
            std::string path = item->text().toStdString();

            QPixmap pixmap(path.c_str());
            if (pixmap.isNull()) return;

            m_view.reset_rect();
            scene->clear();
            scene->addPixmap(pixmap);
            scene->setSceneRect(pixmap.rect());

            m_view.fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
            m_view.set_current_image(pixmap.toImage());

            emit texture_changed(path);
            emit tileset_selected(true);
        }
    );
}

Palette::~Palette()
{
    QSettings settings("Game Academy", "Planetarium");
    settings.beginGroup("palette");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

const TilesetView* Palette::view() const
{
    return &m_view;
}

void Palette::reset_selection()
{
    m_tree_view->clearSelection();
    m_view.reset_rect();
}

void Palette::clear_tileset_selection()
{
    bool blocked = m_tree_view->blockSignals(true);
    m_tree_view->clearSelection();
    m_tree_view->blockSignals(blocked);
    m_view.reset_rect();
}

void Palette::remove_selected_tileset()
{
    const auto selected = m_tree_view->selectedItems();
    if (selected.isEmpty())
        return;

    int row = m_tree_view->row(selected[0]);
    delete m_tree_view->takeItem(row);

    if (m_tree_view->count() > 0)
    {
        int next_row = std::min(row, m_tree_view->count() - 1);
        auto* next_item = m_tree_view->item(next_row);
        if (next_item)
            next_item->setSelected(true);
    }
    else
    {
        m_view.reset_rect();
        emit tileset_selected(false);
    }
}

