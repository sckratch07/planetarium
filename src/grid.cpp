#include <grid.hpp>

#include <QSettings>
#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QDialog>
#include <QLineEdit>

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

    auto* add_type_button = new QPushButton("Add Type", this);
    add_type_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto* remove_type_button = new QPushButton("Remove Type", this);
    remove_type_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_type_list = new QListWidget(this);
    m_type_list->setSizePolicy({QSizePolicy::Preferred, QSizePolicy::Preferred});
    m_type_list->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    auto* button_layout = new QHBoxLayout();
    button_layout->addWidget(add_type_button);
    button_layout->addWidget(remove_type_button);

    auto* main_layout = new QVBoxLayout(this);

    main_layout->addLayout(grid_width_layout);
    main_layout->addLayout(grid_height_layout);
    main_layout->addLayout(cell_width_layout);
    main_layout->addLayout(cell_height_layout);
    main_layout->addLayout(button_layout);
    main_layout->addWidget(m_type_list);

    auto* dialog = new QDialog(this);
    auto* enter_name_label = new QLabel("Enter type's name", dialog);
    auto* name_edit = new QLineEdit(dialog);
    auto* dialog_layout = new QHBoxLayout(dialog);
    dialog_layout->addWidget(enter_name_label);
    dialog_layout->addWidget(name_edit);

    connect(grid_size_width, &QSpinBox::valueChanged, [this](int value)
        {
            m_grid_size.x = value;
            emit grid_size_changed(m_grid_size);
        }
    );

    connect(grid_size_height, &QSpinBox::valueChanged, [this](int value)
        {
            m_grid_size.y = value;
            emit grid_size_changed(m_grid_size);
        }
    );

    connect(cell_size_width, &QSpinBox::valueChanged, [this](int value)
        {
            m_cell_size.x = value;
            emit cell_size_changed(m_cell_size);
        }
    );

    connect(cell_size_height, &QSpinBox::valueChanged, [this](int value)
        {
            m_cell_size.y = value;
            emit cell_size_changed(m_cell_size);
        }
    );

    connect(add_type_button, &QPushButton::pressed, this, [dialog]
        {
            dialog->open();
        }
    );

    connect(name_edit, &QLineEdit::editingFinished, this, [this, dialog, name_edit]
        {
            if (name_edit->text().isEmpty()) return;
            auto* item = new QListWidgetItem(name_edit->text(), m_type_list);
            m_type_list->addItem(item);
            item->setSelected(true);
            dialog->close();
        }
    );

    connect(remove_type_button, &QPushButton::pressed, this, [this]
        {
            const auto selected = m_type_list->selectedItems();
            if (selected.isEmpty()) return;

            auto* item = selected[0];
            QString removed_type = item->text();

            m_type_list->takeItem(m_type_list->row(item));
            delete item;

            m_selected_type.clear();
            emit tile_type_selection_cleared();
            emit tile_type_removed(removed_type);
        }
    );

    connect(m_type_list, &QListWidget::itemSelectionChanged, this, [this]
        {
            const auto selected = m_type_list->selectedItems();
            if (selected.isEmpty())
            {
                m_selected_type.clear();
                emit tile_type_selection_cleared();
                return;
            }

            auto* item = selected[0];
            m_selected_type = item->text();
            emit tile_type_changed(m_selected_type);
        }
    );
}

void Grid::clear_type_selection()
{
    bool blocked = m_type_list->blockSignals(true);
    m_type_list->clearSelection();
    m_type_list->blockSignals(blocked);
    m_selected_type.clear();
    emit tile_type_selection_cleared();
}

Grid::~Grid()
{
    QSettings settings("Game Academy", "Planetarium");

    settings.beginGroup("grid");
    settings.setValue("grid_size_x", m_grid_size.x);
    settings.setValue("grid_size_y", m_grid_size.y);

    settings.setValue("cell_size_x", m_cell_size.x);
    settings.setValue("cell_size_y", m_cell_size.y);
    settings.endGroup();
}

const sf::Vector2i& Grid::grid_size() const
{
    return m_grid_size;
}

const sf::Vector2i& Grid::cell_size() const
{
    return m_cell_size;
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
        lines.append(sf::Vertex(sf::Vector2f(x, top), sf::Color::White));
        lines.append(sf::Vertex(sf::Vector2f(x, bottom), sf::Color::White));
    }

    for (float y = firstY; y <= lastY; y += m_cell_size.y)
    {
        lines.append(sf::Vertex(sf::Vector2f(left, y), sf::Color::White));
        lines.append(sf::Vertex(sf::Vector2f(right, y), sf::Color::White));
    }

    target.draw(lines);
}