#include <palette.hpp>
#include <tileset_view.hpp>

#include <QSettings>
#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QFileDialog>

#include <SFML/Graphics/Texture.hpp>

Palette::Palette(QWidget* parent) :
    QWidget(parent), m_view(this), m_tile_size({32, 32})
{
    QSettings settings("Game Academy", "Planetarium");
    settings.beginGroup("palette");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();

    auto* width_layout = new QHBoxLayout();
    
    auto* width_label = new QLabel("Width: ", this);
    width_label->setSizePolicy({QSizePolicy::Fixed, QSizePolicy::Fixed});
    width_layout->addWidget(width_label);

    auto* width_spin_box = new QSpinBox(this);
    width_spin_box->setSizePolicy({QSizePolicy::Expanding, QSizePolicy::Fixed});
    width_spin_box->setRange(5, 300);
    width_spin_box->setValue(m_tile_size.x);
    width_layout->addWidget(width_spin_box);

    auto* height_layout = new QHBoxLayout();
    
    auto* height_label = new QLabel("Height: ", this);
    height_label->setSizePolicy({QSizePolicy::Fixed, QSizePolicy::Fixed});
    height_layout->addWidget(height_label);

    auto* height_spin_box = new QSpinBox(this);
    height_spin_box->setSizePolicy({QSizePolicy::Expanding, QSizePolicy::Fixed});
    height_spin_box->setRange(5, 300);
    height_spin_box->setValue(m_tile_size.y);
    height_layout->addWidget(height_spin_box);

    auto* button_layout = new QHBoxLayout();

    auto* load_button = new QPushButton("Load Spritesheet", this);
    load_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    button_layout->addWidget(load_button);

    auto* main_layout = new QVBoxLayout(this);
    main_layout->addLayout(width_layout);
    main_layout->addLayout(height_layout);
    main_layout->addSpacing(15);
    main_layout->addLayout(button_layout);

    auto* scene = new QGraphicsScene(this);
    m_view.setScene(scene);
    main_layout->addWidget(&m_view);

    connect(width_spin_box, &QSpinBox::valueChanged, [this](int value)
        {
            m_tile_size.x = value;
            emit tile_size_changed(m_tile_size);
        }
    );
    
    connect(height_spin_box, &QSpinBox::valueChanged, [this](int value)
        {
            m_tile_size.y = value;
            emit tile_size_changed(m_tile_size);
        }
    );

    connect(load_button, &QPushButton::pressed, this, [this, scene]
        {
            if (QString file_path = QFileDialog::getOpenFileName(this, "Load Spritesheet", "", "Images (*.png *.xpm *.jpg)"); !file_path.isEmpty())
            {
                QPixmap pixmap(file_path);
                if (pixmap.isNull()) return;
                emit texture_changed(file_path.toStdString());

                scene->clear();
                scene->addPixmap(pixmap);
                scene->setSceneRect(pixmap.rect());

                m_view.fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
            }
        }
    );

    connect(this, &Palette::tile_size_changed, &m_view, &TilesetView::tile_size_changed);
    emit tile_size_changed(m_tile_size);
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