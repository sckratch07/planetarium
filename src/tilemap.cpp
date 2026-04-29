#include <tilemap.hpp>

#include <QGraphicsRectItem>
#include <QFileDialog>
#include <SFML/Graphics/RectangleShape.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>

Tilemap::Tilemap(QObject* parent) :
    QObject(parent), m_selected_rect(nullptr)
{
    
}

void Tilemap::event(const std::optional<sf::Event>& event, sf::RenderWindow& target)
{
    if (!event || !m_selected_rect) return;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    {
        sf::Vector2f mouse_pos = target.mapPixelToCoords(sf::Mouse::getPosition(target));
        sf::Vector2i grid_pos(
            (int)mouse_pos.x / m_cell_size.x,
            (int)mouse_pos.y / m_cell_size.y
        );

        if (grid_pos.x >= m_grid_size.x || grid_pos.y >= m_grid_size.y)
            return;

        for (const auto& tile : m_tiles)
        {
            if (tile.pos == grid_pos) return;
        }

        QRectF rect = m_selected_rect->rect();

        Tile tile;
        tile.pos = grid_pos;
        tile.rect = sf::IntRect(
            { (int)rect.x(), (int)rect.y() },
            { (int)rect.width(), (int)rect.height() }
        );

        m_tiles.push_back(tile);
    }
    else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
    {
        sf::Vector2f mouse_pos = target.mapPixelToCoords(sf::Mouse::getPosition(target));
        sf::Vector2i grid_pos(
            (int)mouse_pos.x / m_cell_size.x,
            (int)mouse_pos.y / m_cell_size.y
        );

        if (grid_pos.x >= m_grid_size.x || grid_pos.y >= m_grid_size.y)
            return;

        std::erase_if(m_tiles, [&](const Tile& tile)
        {
            return tile.pos == grid_pos;
        });
    }
}

void Tilemap::draw(sf::RenderWindow& target)
{
    sf::RectangleShape shape(m_cell_size);
    shape.setTexture(&m_texture);

    for (const auto& tile : m_tiles)
    {
        shape.setPosition(
            { tile.pos.x * m_cell_size.x,
            tile.pos.y * m_cell_size.y }
        );

        shape.setTextureRect(tile.rect);

        target.draw(shape);
    }
}

void Tilemap::grid_size_changed(const sf::Vector2i& new_size)
{
    m_grid_size = new_size;
    std::erase_if(m_tiles, [&](const Tile& tile)
    {
        return tile.pos.x >= new_size.x ||
            tile.pos.y >= new_size.y;
    });
}

void Tilemap::cell_size_changed(const sf::Vector2i& new_size)
{
    m_cell_size = sf::Vector2f(new_size);
}

void Tilemap::texture_changed(const std::string& path)
{
    m_texture.loadFromFile(path);
}

void Tilemap::selected_rect_changed(QGraphicsRectItem* rect)
{
    m_selected_rect = rect;
}

void Tilemap::save()
{
    if (QString path = QFileDialog::getExistingDirectory(nullptr, "Select save folder", ""); !path.isEmpty())
    {
        std::string path_str = path.toStdString() + "/" + "map.json";

        nlohmann::json data;

        data["map_size_x"] = m_grid_size.x;
        data["map_size_y"] = m_grid_size.y;

        data["cell_size_x"] = m_cell_size.x;
        data["cell_size_y"] = m_cell_size.y;

        data["tiles"] = nlohmann::json::array();

        for (const auto& tile : m_tiles)
        {
            nlohmann::json t;

            t["grid_x"] = tile.pos.x;
            t["grid_y"] = tile.pos.y;

            t["rect_left"] = tile.rect.position.x;
            t["rect_top"] = tile.rect.position.y;
            t["rect_width"] = tile.rect.size.x;
            t["rect_height"] = tile.rect.size.y;

            data["tiles"].push_back(t);
        }

        std::ofstream file(path_str);
        file << data;
        file.close();
    }
}