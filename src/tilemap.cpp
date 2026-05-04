#include <tilemap.hpp>

#include <QGraphicsRectItem>
#include <QFileDialog>
#include <SFML/Graphics/RectangleShape.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>

Tilemap::Tilemap(QObject* parent) :
    QObject(parent), m_active_layer_index(0), m_selected_type(""), m_selected_rect(nullptr), m_texture(nullptr)
{
    m_layers.push_back({"Layer 1", true, {}});
}

Tilemap::Layer& Tilemap::active_layer()
{
    if (m_active_layer_index < 0 || m_active_layer_index >= static_cast<int>(m_layers.size()))
        m_active_layer_index = 0;
    return m_layers[m_active_layer_index];
}

const Tilemap::Layer& Tilemap::active_layer() const
{
    if (m_active_layer_index < 0 || m_active_layer_index >= static_cast<int>(m_layers.size()))
        return m_layers.front();
    return m_layers[m_active_layer_index];
}

void Tilemap::event(const std::optional<sf::Event>& event, sf::RenderWindow& target)
{
    if (!event) return;

    sf::Vector2f mouse_pos = target.mapPixelToCoords(sf::Mouse::getPosition(target));
    if (target.getView().getViewport().contains(mouse_pos)) return;

    sf::Vector2i grid_pos(
        static_cast<int>(mouse_pos.x) / m_cell_size.x,
        static_cast<int>(mouse_pos.y) / m_cell_size.y
    );

    if (grid_pos.x < 0 || grid_pos.y < 0 ||
        grid_pos.x >= m_grid_size.x || grid_pos.y >= m_grid_size.y)
        return;

    auto& layer = active_layer();
    auto existing = std::find_if(layer.tiles.begin(), layer.tiles.end(), [&](const Tile& tile)
    {
        return tile.pos == grid_pos;
    });

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    {
        if (!m_selected_type.empty() && !m_selected_rect)
        {
            if (existing != layer.tiles.end())
                existing->type = m_selected_type;
            return;
        }

        if (existing != layer.tiles.end()) return;
        if (!m_selected_rect) return;

        QRectF rect = m_selected_rect->rect();

        layer.tiles.push_back({
            grid_pos,
            sf::IntRect(
                { static_cast<int>(rect.x()), static_cast<int>(rect.y()) },
                { static_cast<int>(rect.width()), static_cast<int>(rect.height()) }
            ),
            m_selected_type
        });
    }
    else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
    {
        if (m_selected_type.empty())
        {
            std::erase_if(layer.tiles, [&](const Tile& tile)
            {
                return tile.pos == grid_pos;
            });
        }
        else if (existing != layer.tiles.end())
        {
            existing->type = "None";
        }
    }
}

void Tilemap::draw(sf::RenderWindow& target)
{
    if (!m_texture) return;

    sf::RectangleShape shape(m_cell_size);
    shape.setTexture(m_texture.get());

    for (int layer_index = 0; layer_index < static_cast<int>(m_layers.size()); ++layer_index)
    {
        const auto& layer = m_layers[layer_index];
        if (!layer.visible) continue;

        const bool is_active = layer_index == m_active_layer_index;
        shape.setFillColor(is_active ? sf::Color::White : sf::Color(255, 255, 255, 140));

        for (const auto& tile : layer.tiles)
        {
            shape.setPosition({ tile.pos.x * m_cell_size.x, tile.pos.y * m_cell_size.y });
            shape.setTextureRect(tile.rect);
            target.draw(shape);

            if (is_active && !m_selected_type.empty() && tile.type == m_selected_type)
            {
                sf::RectangleShape highlight(m_cell_size);
                highlight.setPosition({ tile.pos.x * m_cell_size.x, tile.pos.y * m_cell_size.y });
                highlight.setFillColor(sf::Color(255, 0, 0, 80));
                highlight.setOutlineColor(sf::Color::Red);
                highlight.setOutlineThickness(1.f);
                target.draw(highlight);
            }
        }
    }
}

void Tilemap::grid_size_changed(const sf::Vector2i& new_size)
{
    m_grid_size = new_size;
    for (auto& layer : m_layers)
    {
        std::erase_if(layer.tiles, [&](const Tile& tile)
        {
            return tile.pos.x >= new_size.x || tile.pos.y >= new_size.y;
        });
    }
}

void Tilemap::cell_size_changed(const sf::Vector2i& new_size)
{
    m_cell_size = sf::Vector2f(new_size);
}

void Tilemap::texture_changed(const std::string path)
{
    if (m_texture)
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
    for (auto& layer : m_layers)
    {
        for (auto& tile : layer.tiles)
        {
            if (tile.type == removed)
                tile.type.clear();
        }
    }
}

void Tilemap::layer_added(const QString& name)
{
    std::string layer_name = name.toStdString();
    if (layer_name.empty()) return;

    m_layers.push_back({layer_name, true, {}});
    m_active_layer_index = static_cast<int>(m_layers.size()) - 1;
}

void Tilemap::layer_removed(const QString& name)
{
    const std::string layer_name = name.toStdString();
    auto removed = std::remove_if(m_layers.begin(), m_layers.end(), [&](const Layer& layer)
    {
        return layer.name == layer_name;
    });

    if (removed == m_layers.end()) return;

    int removed_index = static_cast<int>(std::distance(m_layers.begin(), removed));
    m_layers.erase(removed, m_layers.end());

    if (m_layers.empty())
    {
        m_layers.push_back({"Layer 1", true, {}});
        m_active_layer_index = 0;
    }
    else if (m_active_layer_index >= static_cast<int>(m_layers.size()) || m_active_layer_index == removed_index)
    {
        m_active_layer_index = static_cast<int>(m_layers.size()) - 1;
    }
}

void Tilemap::layer_selected(const QString& name)
{
    const std::string layer_name = name.toStdString();
    for (int index = 0; index < static_cast<int>(m_layers.size()); ++index)
    {
        if (m_layers[index].name == layer_name)
        {
            m_active_layer_index = index;
            break;
        }
    }
}

void Tilemap::layer_visibility_changed(const QString& name, bool visible)
{
    const std::string layer_name = name.toStdString();
    for (auto& layer : m_layers)
    {
        if (layer.name == layer_name)
        {
            layer.visible = visible;
            break;
        }
    }
}

void Tilemap::save()
{
    if (QString path = QFileDialog::getExistingDirectory(nullptr, "Select save folder", ""); !path.isEmpty())
    {
        std::string path_str = path.toStdString() + "/map.json";

        nlohmann::json data;

        data["map_size_x"] = m_grid_size.x;
        data["map_size_y"] = m_grid_size.y;

        data["cell_size_x"] = m_cell_size.x;
        data["cell_size_y"] = m_cell_size.y;

        data["layers"] = nlohmann::json::array();
        for (const auto& layer : m_layers)
        {
            nlohmann::json l;
            l["name"] = layer.name;
            l["visible"] = layer.visible;
            data["layers"].push_back(l);
        }

        data["tiles"] = nlohmann::json::array();
        for (const auto& layer : m_layers)
        {
            for (const auto& tile : layer.tiles)
            {
                nlohmann::json t;
                t["grid_x"] = tile.pos.x;
                t["grid_y"] = tile.pos.y;
                t["rect_left"] = tile.rect.position.x;
                t["rect_top"] = tile.rect.position.y;
                t["rect_width"] = tile.rect.size.x;
                t["rect_height"] = tile.rect.size.y;
                t["type"] = tile.type;
                t["layer"] = layer.name;
                data["tiles"].push_back(t);
            }
        }

        std::ofstream file(path_str);
        file << data;
        file.close();
    }
}