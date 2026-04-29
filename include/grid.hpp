#ifndef GRID_HPP
#define GRID_HPP

#include <QWidget>
#include <SFML/Graphics/RenderWindow.hpp>

class Grid : public QWidget
{
    Q_OBJECT
public:
    explicit Grid(QWidget* parent = nullptr);
    ~Grid() override;

    const sf::Vector2i& grid_size() const;
    const sf::Vector2i& cell_size() const;

public slots:
    void draw(sf::RenderWindow& target) const;

signals:
    void grid_size_changed(const sf::Vector2i& new_size);
    void cell_size_changed(const sf::Vector2i& new_size);

private:
    sf::Vector2i m_grid_size;
    sf::Vector2i m_cell_size;
    sf::Color m_color;
};

#endif