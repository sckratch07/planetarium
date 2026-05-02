#ifndef TILEMAP_HPP
#define TILEMAP_HPP

#include <QObject>
#include <QString>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <vector>

class QGraphicsRectItem;

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
    void selected_type_changed(const QString& type);
    void clear_selected_type();
    void removed_type(const QString& type);

    void save();
    
private:
    struct Tile
    {
        sf::Vector2i pos;
        sf::IntRect rect;
        std::string type = "None";
    };

    std::vector<Tile> m_tiles;
    std::unique_ptr<sf::Texture> m_texture;

    sf::Vector2i m_grid_size;
    sf::Vector2f m_cell_size;
    QGraphicsRectItem* m_selected_rect;
    std::string m_selected_type;
};

#endif