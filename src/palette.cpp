#include <palette.hpp>
#include <tileset_view.hpp>

#include <QListWidget>
#include <QSettings>
#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QFileDialog>

#include <SFML/Graphics/Texture.hpp>

Palette::Palette(QWidget* parent) :
    QWidget(parent), m_view(this), m_tile_size({32, 32}), m_tree_view(new QListWidget(this))
{
    QSettings settings("Game Academy", "Planetarium");
    settings.beginGroup("palette");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();

    auto* load_button = new QPushButton("Add Tileset", this);
    load_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_tree_view->setSizePolicy({QSizePolicy::Preferred, QSizePolicy::Preferred});
    m_tree_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    auto* main_layout = new QVBoxLayout(this);
    main_layout->addWidget(load_button);
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

                scene->clear();
                m_view.reset_rect();
                scene->addPixmap(pixmap);
                scene->setSceneRect(pixmap.rect());

                m_view.fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
            }
        }
    );

    connect(m_tree_view, &QListWidget::itemSelectionChanged, this, [this, scene]()
        {
            const auto selected = m_tree_view->selectedItems();
            if (selected.isEmpty())
            {
                emit tileset_selected(false);
                return;
            }

            auto* item = selected[0];
            std::string path = item->text().toStdString();

            QPixmap pixmap(path.c_str());
            if (pixmap.isNull()) return;

            scene->clear();
            m_view.reset_rect();
            scene->addPixmap(pixmap);
            scene->setSceneRect(pixmap.rect());

            m_view.fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

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

