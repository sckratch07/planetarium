#include <grid.hpp>

#include <utils/color_picker.hpp>

#include <QSettings>
#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include <SFML/Graphics/VertexArray.hpp>

Grid::Grid(QWidget* parent) :
    QWidget(parent)
{
    QSettings settings("Game Academy", "Planetarium");

    settings.beginGroup("grid");
    m_grid_size.x = settings.value("grid_size_x", 50).toInt();
    m_grid_size.y = settings.value("grid_size_y", 30).toInt();

    m_cell_size.x = settings.value("cell_size_x", 32).toInt();
    m_cell_size.y = settings.value("cell_size_y", 32).toInt();

    m_color.r = settings.value("color_r", 255).toInt();
    m_color.g = settings.value("color_g", 255).toInt();
    m_color.b = settings.value("color_b", 255).toInt();
    m_color.a = settings.value("color_a", 255).toInt();
    settings.endGroup();

    auto* grid_width_layout = new QHBoxLayout();

    auto* label_grid_width = new QLabel("Grid width: ", this);
    label_grid_width->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    grid_width_layout->addWidget(label_grid_width);

    auto* grid_size_width = new QSpinBox(this);
    grid_size_width->setRange(5, 300);
    grid_size_width->setValue(m_grid_size.x);
    grid_size_width->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    grid_width_layout->addWidget(grid_size_width);

    auto* grid_height_layout = new QHBoxLayout();

    auto* label_grid_height = new QLabel("Grid height: ", this);
    label_grid_height->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    grid_height_layout->addWidget(label_grid_height);

    auto* grid_size_height = new QSpinBox(this);
    grid_size_height->setRange(3, 100);
    grid_size_height->setValue(m_grid_size.y);
    grid_size_height->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    grid_height_layout->addWidget(grid_size_height);

    auto* cell_width_layout = new QHBoxLayout();

    auto* label_cell_width = new QLabel("Cell width: ", this);
    label_cell_width->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cell_width_layout->addWidget(label_cell_width);

    auto* cell_size_width = new QSpinBox(this);
    cell_size_width->setRange(5, 300);
    cell_size_width->setValue(m_cell_size.x);
    cell_size_width->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    cell_width_layout->addWidget(cell_size_width);

    auto* cell_height_layout = new QHBoxLayout();

    auto* label_cell_height = new QLabel("Cell height: ", this);
    label_cell_height->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cell_height_layout->addWidget(label_cell_height);

    auto* cell_size_height = new QSpinBox(this);
    cell_size_height->setRange(5, 300);
    cell_size_height->setValue(m_cell_size.y);
    cell_size_height->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    cell_height_layout->addWidget(cell_size_height);

    auto* main_layout = new QVBoxLayout(this);

    main_layout->addLayout(grid_width_layout);
    main_layout->addLayout(grid_height_layout);

    main_layout->addSpacing(5);

    main_layout->addLayout(cell_width_layout);
    main_layout->addLayout(cell_height_layout);
    
    main_layout->addSpacing(10);

    auto* color_picker = new ColorPickerWidget(this);
    main_layout->addWidget(color_picker);

    connect(grid_size_width, &QSpinBox::valueChanged, [this](int value)
        {
            m_grid_size.x = value;
        }
    );

    connect(grid_size_height, &QSpinBox::valueChanged, [this](int value)
        {
            m_grid_size.y = value;
        }
    );

    connect(cell_size_width, &QSpinBox::valueChanged, [this](int value)
        {
            m_cell_size.x = value;
        }
    );

    connect(cell_size_height, &QSpinBox::valueChanged, [this](int value)
        {
            m_cell_size.y = value;
        }
    );

    connect(color_picker, &ColorPickerWidget::color_changed, [this](const QColor& c)
        {
            m_color.r = c.red();
            m_color.g = c.green();
            m_color.b = c.blue();
            m_color.a = c.alpha();
        }
    );
}

Grid::~Grid()
{
    QSettings settings("Game Academy", "Planetarium");

    settings.beginGroup("grid");
    settings.setValue("grid_size_x", m_grid_size.x);
    settings.setValue("grid_size_y", m_grid_size.y);

    settings.setValue("cell_size_x", m_cell_size.x);
    settings.setValue("cell_size_y", m_cell_size.y);

    settings.setValue("color_r", m_color.r);
    settings.setValue("color_g", m_color.g);
    settings.setValue("color_b", m_color.b);
    settings.setValue("color_a", m_color.a);
    settings.endGroup();
}

void Grid::draw(sf::RenderWindow& target) const
{
    float left   = 0.f;
    float top    = 0.f;
    float right  = m_grid_size.x * m_cell_size.x;
    float bottom = m_grid_size.y * m_cell_size.y;

    // Aligner sur la grille
    int firstX = static_cast<int>(std::floor(left / m_cell_size.x)) * m_cell_size.x;
    int lastX  = static_cast<int>(std::ceil(right / m_cell_size.x)) * m_cell_size.x;

    int firstY = static_cast<int>(std::floor(top / m_cell_size.y)) * m_cell_size.y;
    int lastY  = static_cast<int>(std::ceil(bottom / m_cell_size.y)) * m_cell_size.y;

    sf::VertexArray lines(sf::PrimitiveType::Lines);

    for (float x = firstX; x <= lastX; x += m_cell_size.x)
    {
        lines.append(sf::Vertex(sf::Vector2f(x, top), m_color));
        lines.append(sf::Vertex(sf::Vector2f(x, bottom), m_color));
    }

    for (float y = firstY; y <= lastY; y += m_cell_size.y)
    {
        lines.append(sf::Vertex(sf::Vector2f(left, y), m_color));
        lines.append(sf::Vertex(sf::Vector2f(right, y), m_color));
    }

    target.draw(lines);
}