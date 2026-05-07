#ifndef GIZMO_ARROW_HPP
#define GIZMO_ARROW_HPP

#include <SFML/Graphics/RenderWindow.hpp>

class GizmoArrow
{
public:
    enum Axis { None, X, Y };

    GizmoArrow(Axis axis, float lenght = 80.f, float thikness = 8.f);

    bool interact(sf::Vector2f& entity_pos, sf::Vector2f& mouse_pos);
    void paint(sf::RenderWindow& renderer, sf::Vector2f& entity_pos, bool hovered);

private:
    sf::FloatRect arrow_rect(sf::Vector2f& entity_pos);

private:
    Axis  m_axis;

    float m_length;
    float m_thickness;
};

#endif