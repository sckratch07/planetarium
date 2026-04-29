#ifndef PALETTE_HPP
#define PALETTE_HPP

#include <tileset_view.hpp>
#include <SFML/Graphics/Texture.hpp>

class Palette : public QWidget
{
    Q_OBJECT
public:
    explicit Palette(QWidget* parent = nullptr);
    ~Palette() override;

    const TilesetView* view() const;

signals:
    void tile_size_changed(const sf::Vector2f& new_size);
    void texture_changed(const std::string& path);

private:
    TilesetView m_view;
    sf::Vector2f m_tile_size;
};

#endif