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

public slots:
    void draw(sf::RenderWindow& target) const;

private:
    sf::Vector2i m_grid_size;
    sf::Vector2i m_cell_size;
    sf::Color m_color;
};

#endif