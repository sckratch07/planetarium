#include <tilemap.hpp>

#include <QGraphicsRectItem>
#include <QFileDialog>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>

Tilemap::Tilemap(QObject* parent) :
    QObject(parent), m_active_layer_index(0), m_selected_type(""), m_selected_rect(nullptr), m_texture(nullptr),
    m_selected_rect_width_tiles(1), m_selected_rect_height_tiles(1), m_selected_tile(nullptr),
    m_preview_position(-1, -1), m_has_preview(false), m_auto_selection_mode(false)
{
    m_layers.push_back({"Layer 1", true, {}});
    m_gizmo.change_layer(&m_layers[0]);
}

Layer& Tilemap::active_layer()
{
    if (m_active_layer_index < 0 || m_active_layer_index >= static_cast<int>(m_layers.size()))
        m_active_layer_index = 0;
    return m_layers[m_active_layer_index];
}

const Layer& Tilemap::active_layer() const
{
    if (m_active_layer_index < 0 || m_active_layer_index >= static_cast<int>(m_layers.size()))
        return m_layers.front();
    return m_layers[m_active_layer_index];
}

void Tilemap::event(const std::optional<sf::Event>& event, sf::RenderWindow& target)
{
    if (!event) return;

    sf::Vector2f mouse_pos = target.mapPixelToCoords(sf::Mouse::getPosition(target));
    sf::Vector2f grid_pos_float(mouse_pos.x / m_cell_size.x, mouse_pos.y / m_cell_size.y);
    sf::Vector2i grid_pos(static_cast<int>(grid_pos_float.x), static_cast<int>(grid_pos_float.y));

    if (m_auto_selection_mode)
    {
        m_preview_position = mouse_pos;
        m_has_preview = true;
    }
    else
    {
        if (grid_pos.x < 0 || grid_pos.y < 0 ||
            grid_pos.x >= m_grid_size.x || grid_pos.y >= m_grid_size.y)
        {
            m_has_preview = false;
            return;
        }
        m_preview_position = sf::Vector2f(grid_pos.x, grid_pos.y);
        m_has_preview = true;
    }

    auto& layer = active_layer();

    if (!m_selected_rect && m_selected_type.empty())
    {
        if (m_selected_tile)
        {
            sf::FloatRect map_bounds({ 0.f, 0.f },
                { static_cast<float>(m_grid_size.x * m_cell_size.x),
                  static_cast<float>(m_grid_size.y * m_cell_size.y) });
            m_gizmo.event(m_selected_tile, event, target, map_bounds);
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !m_gizmo.arrow_handle())
        {
            auto existing = std::find_if(layer.tiles.begin(), layer.tiles.end(), [&mouse_pos](const Tile& tile)
                {
                    if (!tile.is_pixel_placed) return false;

                    sf::FloatRect object_rect(tile.pos, sf::Vector2f(static_cast<float>(tile.rect.size.x), static_cast<float>(tile.rect.size.y)));
                    return object_rect.contains(mouse_pos);
                }
            );

            if (existing != layer.tiles.end())
                m_selected_tile = &(*existing);
            else
                m_selected_tile = nullptr;
        }
        return;
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    {
        if (!m_selected_type.empty() && !m_selected_rect)
        {
            if (m_auto_selection_mode)
            {
                auto existing = std::find_if(layer.tiles.begin(), layer.tiles.end(), [&mouse_pos](const Tile& tile)
                {
                    if (!tile.is_pixel_placed)
                        return false;

                    sf::FloatRect object_rect(tile.pos,
                        sf::Vector2f(static_cast<float>(tile.rect.size.x), static_cast<float>(tile.rect.size.y)));
                    return object_rect.contains(mouse_pos);
                });

                if (existing != layer.tiles.end())
                    existing->type = m_selected_type;
            }
            else
            {
                auto existing = std::find_if(layer.tiles.begin(), layer.tiles.end(), [&grid_pos](const Tile& tile)
                {
                    return !tile.is_pixel_placed && tile.pos == sf::Vector2f(grid_pos.x, grid_pos.y);
                });

                if (existing != layer.tiles.end())
                    existing->type = m_selected_type;
            }
            return;
        }

        if (!m_selected_rect) return;

        QRectF rect = m_selected_rect->rect();

        if (m_auto_selection_mode)
        {
            if (!event->is<sf::Event::MouseButtonPressed>()) return;

            sf::FloatRect world_rect(sf::Vector2f(mouse_pos.x, mouse_pos.y),
                sf::Vector2f(static_cast<float>(rect.width()), static_cast<float>(rect.height())));
            sf::FloatRect grid_bounds(sf::Vector2f(0.f, 0.f),
                sf::Vector2f(static_cast<float>(m_grid_size.x * m_cell_size.x),
                             static_cast<float>(m_grid_size.y * m_cell_size.y)));

            auto clipped_rect_opt = world_rect.findIntersection(grid_bounds);
            if (!clipped_rect_opt) return;

            const sf::FloatRect clipped_rect = *clipped_rect_opt;
            int clip_left = static_cast<int>(std::round(clipped_rect.position.x - world_rect.position.x));
            int clip_top = static_cast<int>(std::round(clipped_rect.position.y - world_rect.position.y));
            int clipped_width = static_cast<int>(std::floor(clipped_rect.size.x));
            int clipped_height = static_cast<int>(std::floor(clipped_rect.size.y));

            if (clipped_width <= 0 || clipped_height <= 0) return;

            Tile new_tile;
            new_tile.pos = sf::Vector2f(clipped_rect.position.x, clipped_rect.position.y);
            new_tile.rect = sf::IntRect(
                { static_cast<int>(rect.x()) + clip_left, static_cast<int>(rect.y()) + clip_top },
                { clipped_width, clipped_height }
            );
            new_tile.type = m_selected_type;
            new_tile.is_pixel_placed = true;
            layer.tiles.push_back(new_tile);

            return;
        }

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

                auto existing_tile = std::find_if(layer.tiles.begin(), layer.tiles.end(), [&](const Tile& tile)
                {
                    return !tile.is_pixel_placed && tile.pos == sf::Vector2f(tile_pos.x, tile_pos.y);
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

                Tile new_tile;
                new_tile.pos = sf::Vector2f(tile_pos.x, tile_pos.y);
                new_tile.rect = sf::IntRect(
                    { rect_start_x + dx * tile_size_x, rect_start_y + dy * tile_size_y },
                    { tile_size_x, tile_size_y }
                );
                new_tile.type = m_selected_type;
                new_tile.is_pixel_placed = false;
                layer.tiles.push_back(new_tile);
            }
        }
    }
    else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
    {
        if (m_auto_selection_mode)
        {
            std::erase_if(layer.tiles, [&](const Tile& tile)
            {
                if (!tile.is_pixel_placed)
                    return false;

                sf::FloatRect world_rect(tile.pos,
                    sf::Vector2f(static_cast<float>(tile.rect.size.x), static_cast<float>(tile.rect.size.y)));
                return world_rect.contains(mouse_pos);
            });
        }
        else if (m_selected_type.empty())
        {
            std::erase_if(layer.tiles, [&](const Tile& tile)
            {
                return !tile.is_pixel_placed && tile.pos == sf::Vector2f(grid_pos.x, grid_pos.y);
            });
        }
        else
        {
            auto existing = std::find_if(layer.tiles.begin(), layer.tiles.end(), [&](const Tile& tile)
            {
                return !tile.is_pixel_placed && tile.pos == sf::Vector2f(grid_pos.x, grid_pos.y);
            });
            if (existing != layer.tiles.end())
                existing->type = "";
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
            sf::Vector2f position = tile.pos;
            if (tile.is_pixel_placed)
            {
                shape.setPosition(position);
            }
            else
            {
                position = { tile.pos.x * m_cell_size.x, tile.pos.y * m_cell_size.y };
                shape.setPosition(position);
            }
            
            shape.setSize(sf::Vector2f(static_cast<float>(tile.rect.size.x), static_cast<float>(tile.rect.size.y)));
            shape.setTextureRect(tile.rect);
            target.draw(shape);

            if (is_active && !m_selected_type.empty() && tile.type == m_selected_type)
            {
                sf::RectangleShape highlight(sf::Vector2f(static_cast<float>(tile.rect.size.x), static_cast<float>(tile.rect.size.y)));
                highlight.setPosition(position);
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
            if (m_auto_selection_mode) {
                sf::RectangleShape preview_shape(sf::Vector2f(rect.width(), rect.height()));
                preview_shape.setTexture(m_texture.get());
                preview_shape.setPosition(m_preview_position);
                preview_shape.setTextureRect(sf::IntRect({static_cast<int>(rect.x()), static_cast<int>(rect.y())}, {static_cast<int>(rect.width()), static_cast<int>(rect.height())}));
                preview_shape.setFillColor(sf::Color(255, 255, 255, 160));
                preview_shape.setOutlineColor(sf::Color(0, 200, 255, 180));
                preview_shape.setOutlineThickness(1.f);
                target.draw(preview_shape);
            }
            else
            {
                int tile_size_x = static_cast<int>(rect.width()) / m_selected_rect_width_tiles;
                int tile_size_y = static_cast<int>(rect.height()) / m_selected_rect_height_tiles;
                const sf::FloatRect grid_bounds({ 0.f, 0.f },
                    { static_cast<float>(m_grid_size.x * m_cell_size.x),
                      static_cast<float>(m_grid_size.y * m_cell_size.y) });

                for (int dy = 0; dy < m_selected_rect_height_tiles; ++dy)
                {
                    for (int dx = 0; dx < m_selected_rect_width_tiles; ++dx)
                    {
                        sf::FloatRect source_world_rect(sf::Vector2f(
                            static_cast<float>(static_cast<int>(m_preview_position.x) + dx) * m_cell_size.x,
                            static_cast<float>(static_cast<int>(m_preview_position.y) + dy) * m_cell_size.y),
                            sf::Vector2f(static_cast<float>(tile_size_x), static_cast<float>(tile_size_y)));

                        auto clipped_rect_opt = source_world_rect.findIntersection(grid_bounds);
                        if (!clipped_rect_opt)
                            continue;

                        const sf::FloatRect clipped_rect = *clipped_rect_opt;
                        int clip_left = static_cast<int>(std::round(clipped_rect.position.x - source_world_rect.position.x));
                        int clip_top = static_cast<int>(std::round(clipped_rect.position.y - source_world_rect.position.y));
                        int clipped_width = static_cast<int>(std::floor(clipped_rect.size.x));
                        int clipped_height = static_cast<int>(std::floor(clipped_rect.size.y));
                        if (clipped_width <= 0 || clipped_height <= 0)
                            continue;

                        sf::RectangleShape preview_shape({ static_cast<float>(clipped_width), static_cast<float>(clipped_height) });
                        preview_shape.setTexture(m_texture.get());
                        preview_shape.setFillColor(sf::Color(255, 255, 255, 160));
                        preview_shape.setOutlineColor(sf::Color(0, 200, 255, 180));
                        preview_shape.setOutlineThickness(1.f);
                        preview_shape.setPosition({ clipped_rect.position.x, clipped_rect.position.y });
                        preview_shape.setTextureRect(sf::IntRect(
                            { static_cast<int>(rect.x()) + dx * tile_size_x + clip_left,
                              static_cast<int>(rect.y()) + dy * tile_size_y + clip_top },
                            { clipped_width, clipped_height }
                        ));
                        target.draw(preview_shape);
                    }
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

    if (m_selected_tile)
    {
        m_gizmo.draw(m_selected_tile, target);
    }
}

void Tilemap::grid_size_changed(const sf::Vector2i& new_size)
{
    m_grid_size = new_size;
    for (auto& layer : m_layers)
    {
        std::erase_if(layer.tiles, [&](const Tile& tile)
        {
            return !tile.is_pixel_placed &&
                (tile.pos.x >= new_size.x || tile.pos.y >= new_size.y);
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
    m_gizmo.change_layer(&m_layers[m_active_layer_index]);
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
    m_gizmo.change_layer(&m_layers[m_active_layer_index]);
}

void Tilemap::layer_selected(int index)
{
    if (index < 0 || index >= static_cast<int>(m_layers.size())) return;
    m_active_layer_index = index;
    m_gizmo.change_layer(&m_layers[index]);
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
    m_gizmo.change_layer(&m_layers[m_active_layer_index]);
}

void Tilemap::set_auto_selection_mode(bool enabled)
{
    m_auto_selection_mode = enabled;
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
                t["pixel_pos"] = tile.is_pixel_placed;
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
                    {
                        tile.rect.position = {
                            tile_data["rect_left"].get<int>(),
                            tile_data["rect_top"].get<int>()
                        };
                    }

                    if (tile_data.contains("rect_width") && tile_data.contains("rect_height"))
                    {
                        tile.rect.size = {
                            tile_data["rect_width"].get<int>(),
                            tile_data["rect_height"].get<int>()
                        };
                    }

                    if (tile_data.contains("type"))
                        tile.type = tile_data["type"].get<std::string>();

                    if (tile_data.contains("pixel_pos"))
                        tile.is_pixel_placed = tile_data["pixel_pos"].get<bool>();

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