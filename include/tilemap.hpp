#ifndef TILEMAP_HPP
#define TILEMAP_HPP

#include <utils/gizmo_view.hpp>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include <optional>
#include <vector>

class QGraphicsRectItem;

struct Tile
{
    sf::Vector2f pos;
    sf::IntRect rect;
    std::string type = "None";
    bool is_pixel_placed = false;
};

class Tilemap : public QObject
{
    Q_OBJECT
public:
    Tilemap(QObject* parent = nullptr);
    
public slots:
    void event(const std::optional<sf::Event>& event, sf::RenderWindow& target);
    void draw(sf::RenderWindow& target);

    void grid_size_changed(const sf::Vector2i& new_size);
    void cell_size_changed(const sf::Vector2i& new_size);
    void texture_changed(const std::string path);
    void selected_rect_changed(QGraphicsRectItem* rect);
    void selected_rect_dimensions_changed(int width_tiles, int height_tiles);
    void selected_type_changed(const QString& type);
    void clear_selected_type();
    void add_type(const QString& type);
    void removed_type(const QString& type);

    void layer_added(const QString& name);
    void layer_removed(int index);
    void layer_selected(int index);
    void layer_visibility_changed(int index, bool visible);
    void layer_moved(int from, int to);

    void set_auto_selection_mode(bool enabled);

    void save();
    void load();

signals:
    void grid_size_updated(const sf::Vector2i& new_size);
    void cell_size_updated(const sf::Vector2i& new_size);
    void types_loaded(const QStringList& types);
    void layers_loaded(const QStringList& names, const QList<bool>& visibility, int selected_index);

private:
    struct Layer
    {
        std::string name;
        bool visible = true;
        std::vector<Tile> tiles;
    };

    Layer& active_layer();
    const Layer& active_layer() const;

    std::vector<Layer> m_layers;
    int m_active_layer_index;
    QStringList m_types;

    std::unique_ptr<sf::Texture> m_texture;

    sf::Vector2i m_grid_size;
    sf::Vector2f m_cell_size;
    QGraphicsRectItem* m_selected_rect;
    std::string m_selected_type;
    int m_selected_rect_width_tiles;
    int m_selected_rect_height_tiles;

    sf::Vector2f m_preview_position;
    bool m_has_preview;
    bool m_auto_selection_mode;

    Tile* m_selected_tile;
    GizmoView m_gizmo;
};

#endif