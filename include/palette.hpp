#ifndef PALETTE_HPP
#define PALETTE_HPP

#include <tileset_view.hpp>
#include <SFML/Graphics/Texture.hpp>

class QListWidget;
class QPushButton;

class Palette : public QWidget
{
    Q_OBJECT
public:
    explicit Palette(QWidget* parent = nullptr);
    ~Palette() override;

    const TilesetView* view() const;

public slots:
    void reset_selection();
    void clear_tileset_selection();
    void remove_selected_tileset();

signals:
    void tile_size_changed(const sf::Vector2f& new_size);
    void texture_changed(const std::string& path);
    void tileset_selected(bool selected);
    void selection_mode_changed(bool auto_selection);

private:
    QListWidget* m_tree_view;
    TilesetView m_view;
    sf::Vector2f m_tile_size;
    QPushButton* m_mode_button;
};

#endif