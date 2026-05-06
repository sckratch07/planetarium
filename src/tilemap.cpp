#include <tilemap.hpp>

#include <QGraphicsRectItem>
#include <QFileDialog>
#include <SFML/Graphics/RectangleShape.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>

Tilemap::Tilemap(QObject* parent) :
    QObject(parent), m_active_layer_index(0), m_selected_type(""), m_selected_rect(nullptr), m_texture(nullptr),
    m_selected_rect_width_tiles(1), m_selected_rect_height_tiles(1),
    m_preview_position(-1, -1), m_has_preview(false)
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
    sf::Vector2i grid_pos(
        static_cast<int>(mouse_pos.x) / m_cell_size.x,
        static_cast<int>(mouse_pos.y) / m_cell_size.y
    );

    if (grid_pos.x < 0 || grid_pos.y < 0 ||
        grid_pos.x >= m_grid_size.x || grid_pos.y >= m_grid_size.y)
    {
        m_has_preview = false;
        return;
    }

    m_preview_position = grid_pos;
    m_has_preview = true;

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

        if (!m_selected_rect) return;

        QRectF rect = m_selected_rect->rect();
        int tile_size_x = static_cast<int>(rect.width()) / m_selected_rect_width_tiles;
        int tile_size_y = static_cast<int>(rect.height()) / m_selected_rect_height_tiles;
        int rect_start_x = static_cast<int>(rect.x());
        int rect_start_y = static_cast<int>(rect.y());

        // Place a block of tiles
        for (int dy = 0; dy < m_selected_rect_height_tiles; ++dy)
        {
            for (int dx = 0; dx < m_selected_rect_width_tiles; ++dx)
            {
                sf::Vector2i tile_pos(grid_pos.x + dx, grid_pos.y + dy);

                // Check bounds
                if (tile_pos.x < 0 || tile_pos.y < 0 ||
                    tile_pos.x >= m_grid_size.x || tile_pos.y >= m_grid_size.y)
                    continue;

                // Check if tile already exists
                auto existing_tile = std::find_if(layer.tiles.begin(), layer.tiles.end(), [&](const Tile& tile)
                {
                    return tile.pos == tile_pos;
                });

                if (existing_tile != layer.tiles.end())
                {
                    existing_tile->rect = sf::IntRect(
                        { rect_start_x + dx * tile_size_x, rect_start_y + dy * tile_size_y },
                        { tile_size_x, tile_size_y }
                    );
                    existing_tile->type = m_selected_type;
                    continue;
                }

                layer.tiles.push_back({
                    tile_pos,
                    sf::IntRect(
                        { rect_start_x + dx * tile_size_x, rect_start_y + dy * tile_size_y },
                        { tile_size_x, tile_size_y }
                    ),
                    m_selected_type
                });
            }
        }
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

    if (m_has_preview && m_texture)
    {
        if (m_selected_rect)
        {
            QRectF rect = m_selected_rect->rect();
            int tile_size_x = static_cast<int>(rect.width()) / m_selected_rect_width_tiles;
            int tile_size_y = static_cast<int>(rect.height()) / m_selected_rect_height_tiles;
            sf::RectangleShape preview_shape(m_cell_size);
            preview_shape.setTexture(m_texture.get());
            preview_shape.setFillColor(sf::Color(255, 255, 255, 160));
            preview_shape.setOutlineColor(sf::Color(0, 200, 255, 180));
            preview_shape.setOutlineThickness(1.f);

            for (int dy = 0; dy < m_selected_rect_height_tiles; ++dy)
            {
                for (int dx = 0; dx < m_selected_rect_width_tiles; ++dx)
                {
                    int tile_x = m_preview_position.x + dx;
                    int tile_y = m_preview_position.y + dy;
                    if (tile_x < 0 || tile_y < 0 || tile_x >= m_grid_size.x || tile_y >= m_grid_size.y)
                        continue;

                    preview_shape.setPosition({ tile_x * m_cell_size.x,
                                                tile_y * m_cell_size.y });
                    preview_shape.setTextureRect(sf::IntRect(
                        { static_cast<int>(rect.x()) + dx * tile_size_x,
                        static_cast<int>(rect.y()) + dy * tile_size_y },
                        { tile_size_x, tile_size_y }
                    ));
                    target.draw(preview_shape);
                }
            }
        }
        else if (!m_selected_type.empty())
        {
            sf::RectangleShape preview_shape(m_cell_size);
            preview_shape.setPosition({ m_preview_position.x * m_cell_size.x, m_preview_position.y * m_cell_size.y });
            preview_shape.setFillColor(sf::Color(0, 255, 0, 64));
            preview_shape.setOutlineColor(sf::Color(0, 255, 0, 128));
            preview_shape.setOutlineThickness(1.f);
            target.draw(preview_shape);
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

void Tilemap::selected_rect_dimensions_changed(int width_tiles, int height_tiles)
{
    m_selected_rect_width_tiles = width_tiles;
    m_selected_rect_height_tiles = height_tiles;
}

void Tilemap::selected_type_changed(const QString& type)
{
    m_selected_type = type.toStdString();
}

void Tilemap::clear_selected_type()
{
    m_selected_type.clear();
}

void Tilemap::add_type(const QString& type)
{
    const QString normalized = type.trimmed();
    if (normalized.isEmpty()) return;
    if (m_types.contains(normalized)) return;
    m_types.push_back(normalized);
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
    m_types.removeAll(type);
}

void Tilemap::layer_added(const QString& name)
{
    std::string layer_name = name.toStdString();
    if (layer_name.empty()) return;

    m_layers.push_back({layer_name, true, {}});
    m_active_layer_index = static_cast<int>(m_layers.size()) - 1;
}

void Tilemap::layer_removed(int index)
{
    if (index < 0 || index >= static_cast<int>(m_layers.size())) return;

    m_layers.erase(m_layers.begin() + index);

    if (m_layers.empty())
    {
        m_layers.push_back({"Layer 1", true, {}});
        m_active_layer_index = 0;
    }
    else if (m_active_layer_index >= static_cast<int>(m_layers.size()) || m_active_layer_index == index)
    {
        m_active_layer_index = static_cast<int>(m_layers.size()) - 1;
    }
}

void Tilemap::layer_selected(int index)
{
    if (index < 0 || index >= static_cast<int>(m_layers.size())) return;
    m_active_layer_index = index;
}

void Tilemap::layer_visibility_changed(int index, bool visible)
{
    if (index < 0 || index >= static_cast<int>(m_layers.size())) return;
    m_layers[index].visible = visible;
}

void Tilemap::layer_moved(int from, int to)
{
    if (from < 0 || from >= static_cast<int>(m_layers.size())) return;
    if (to < 0 || to >= static_cast<int>(m_layers.size())) return;
    if (from == to) return;

    Layer layer = std::move(m_layers[from]);
    m_layers.erase(m_layers.begin() + from);
    m_layers.insert(m_layers.begin() + to, std::move(layer));

    if (m_active_layer_index == from)
    {
        m_active_layer_index = to;
    }
    else if (from < to && m_active_layer_index > from && m_active_layer_index <= to)
    {
        m_active_layer_index -= 1;
    }
    else if (from > to && m_active_layer_index >= to && m_active_layer_index < from)
    {
        m_active_layer_index += 1;
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

        data["types"] = nlohmann::json::array();
        for (const auto& type : m_types)
        {
            data["types"].push_back(type.toStdString());
        }

        data["tiles"] = nlohmann::json::array();
        for (int layer_index = 0; layer_index < static_cast<int>(m_layers.size()); ++layer_index)
        {
            const auto& layer = m_layers[layer_index];
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
                t["layer"] = layer_index;
                data["tiles"].push_back(t);
            }
        }

        std::ofstream file(path_str);
        file << data.dump(4);
        file.close();
    }
}

void Tilemap::load()
{
    if (QString file_path = QFileDialog::getOpenFileName(nullptr, "Load project", "", "JSON Files (*.json)"); !file_path.isEmpty())
    {
        std::ifstream file(file_path.toStdString());
        if (!file.is_open()) return;

        try
        {
            nlohmann::json data = nlohmann::json::parse(file);
            file.close();

            // Restore grid and cell sizes
            if (data.contains("map_size_x") && data.contains("map_size_y"))
            {
                m_grid_size.x = data["map_size_x"].get<int>();
                m_grid_size.y = data["map_size_y"].get<int>();
            }

            if (data.contains("cell_size_x") && data.contains("cell_size_y"))
            {
                m_cell_size.x = data["cell_size_x"].get<float>();
                m_cell_size.y = data["cell_size_y"].get<float>();
            }

            // Clear existing layers and tiles
            m_layers.clear();

            // Restore layers
            if (data.contains("layers") && data["layers"].is_array())
            {
                for (const auto& layer_data : data["layers"])
                {
                    Layer layer;
                    if (layer_data.contains("name"))
                        layer.name = layer_data["name"].get<std::string>();
                    if (layer_data.contains("visible"))
                        layer.visible = layer_data["visible"].get<bool>();
                    layer.tiles.clear();
                    m_layers.push_back(layer);
                }
            }

            // Ensure we have at least one layer
            if (m_layers.empty())
                m_layers.push_back({"Layer 1", true, {}});

            // Restore types
            m_types.clear();
            if (data.contains("types") && data["types"].is_array())
            {
                for (const auto& type_value : data["types"])
                {
                    if (type_value.is_string())
                        m_types.push_back(QString::fromStdString(type_value.get<std::string>()));
                }
            }

            // Restore tiles
            if (data.contains("tiles") && data["tiles"].is_array())
            {
                for (const auto& tile_data : data["tiles"])
                {
                    int layer_index = 0;
                    if (tile_data.contains("layer"))
                        layer_index = tile_data["layer"].get<int>();

                    if (layer_index < 0 || layer_index >= static_cast<int>(m_layers.size()))
                        continue;

                    Tile tile;
                    if (tile_data.contains("grid_x"))
                        tile.pos.x = tile_data["grid_x"].get<int>();
                    if (tile_data.contains("grid_y"))
                        tile.pos.y = tile_data["grid_y"].get<int>();
                    if (tile_data.contains("rect_left") && tile_data.contains("rect_top"))
                        tile.rect.position = {
                            tile_data["rect_left"].get<int>(),
                            tile_data["rect_top"].get<int>()
                        };
                    if (tile_data.contains("rect_width") && tile_data.contains("rect_height"))
                        tile.rect.size = {
                            tile_data["rect_width"].get<int>(),
                            tile_data["rect_height"].get<int>()
                        };
                    if (tile_data.contains("type"))
                        tile.type = tile_data["type"].get<std::string>();

                    m_layers[layer_index].tiles.push_back(tile);
                }
            }

            m_active_layer_index = 0;

            QStringList layer_names;
            QList<bool> layer_visibility;
            for (const auto& layer : m_layers)
            {
                layer_names.push_back(QString::fromStdString(layer.name));
                layer_visibility.push_back(layer.visible);
            }

            emit grid_size_updated(m_grid_size);
            emit cell_size_updated(sf::Vector2i(static_cast<int>(m_cell_size.x), static_cast<int>(m_cell_size.y)));
            emit types_loaded(m_types);
            emit layers_loaded(layer_names, layer_visibility, m_active_layer_index);
        }
        catch (const std::exception& e)
        {
            // Handle JSON parsing error
            qWarning() << "Failed to load project:" << e.what();
        }
    }
}