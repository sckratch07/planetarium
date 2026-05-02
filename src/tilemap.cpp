#include <tilemap.hpp>

#include <QGraphicsRectItem>
#include <QFileDialog>
#include <SFML/Graphics/RectangleShape.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>

Tilemap::Tilemap(QObject* parent) :
    QObject(parent), m_selected_rect(nullptr), m_texture(nullptr)
{
    
}

void Tilemap::event(const std::optional<sf::Event>& event, sf::RenderWindow& target)
{
    if (!event) return;

    sf::Vector2f mouse_pos = target.mapPixelToCoords(sf::Mouse::getPosition(target));
    sf::Vector2i grid_pos(
        (int)mouse_pos.x / m_cell_size.x,
        (int)mouse_pos.y / m_cell_size.y
    );

    if (grid_pos.x < 0 || grid_pos.y < 0 ||
        grid_pos.x >= m_grid_size.x || grid_pos.y >= m_grid_size.y)
        return;

    auto existing = std::find_if(m_tiles.begin(), m_tiles.end(), [&](const Tile& tile)
    {
        return tile.pos == grid_pos;
    });

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    {
        if (!m_selected_type.empty() && !m_selected_rect)
        {
            if (existing != m_tiles.end())
                existing->type = m_selected_type;
            return;
        }

        if (existing != m_tiles.end()) return;
        if (!m_selected_rect) return;

        QRectF rect = m_selected_rect->rect();

        m_tiles.push_back({
            grid_pos,
            sf::IntRect(
                { (int)rect.x(), (int)rect.y() },
                { (int)rect.width(), (int)rect.height() }
            ),
            m_selected_type
        });
    }
    else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
    {
        if (!m_selected_type.empty())
        {
            std::erase_if(m_tiles, [&](const Tile& tile)
            {
                return tile.pos == grid_pos;
            });
        }
        else
        {
            existing->type = m_selected_type;
        }
    }
}

void Tilemap::draw(sf::RenderWindow& target)
{
    if (!m_texture || m_tiles.empty()) return;

    sf::RectangleShape shape(m_cell_size);
    shape.setTexture(m_texture.get());

    for (const auto& tile : m_tiles)
    {
        shape.setPosition(
            { tile.pos.x * m_cell_size.x,
            tile.pos.y * m_cell_size.y }
        );

        shape.setTextureRect(tile.rect);

        target.draw(shape);

        if (!m_selected_type.empty() && tile.type == m_selected_type)
        {
            sf::RectangleShape highlight(m_cell_size);
            highlight.setPosition(
                { tile.pos.x * m_cell_size.x,
                tile.pos.y * m_cell_size.y }
            );
            highlight.setFillColor(sf::Color(255, 0, 0, 80));
            highlight.setOutlineColor(sf::Color::Red);
            highlight.setOutlineThickness(1.f);
            target.draw(highlight);
        }
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

void Tilemap::texture_changed(const std::string path)
{
    if(m_texture)
        m_texture.reset();

    m_texture = std::make_unique<sf::Texture>(path);
}

void Tilemap::selected_rect_changed(QGraphicsRectItem* rect)
{
    m_selected_rect = rect;
}

void Tilemap::selected_type_changed(const QString& type)
{
    m_selected_type = type.toStdString();
}

void Tilemap::clear_selected_type()
{
    m_selected_type.clear();
}

void Tilemap::removed_type(const QString& type)
{
    const std::string removed = type.toStdString();
    for (auto& tile : m_tiles)
    {
        if (tile.type == removed)
            tile.type.clear();
    }
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
            t["type"] = tile.type;

            data["tiles"].push_back(t);
        }

        std::ofstream file(path_str);
        file << data;
        file.close();
    }
}