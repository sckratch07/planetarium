#include <utils/gizmo_arrow.hpp>
#include <SFML/Graphics/VertexArray.hpp>

GizmoArrow::GizmoArrow(Axis axis, float length, float thickness) :
    m_axis(axis), m_length(length), m_thickness(thickness)
{

}

bool GizmoArrow::interact(sf::Vector2f& entity_pos, sf::Vector2f& mouse_pos)
{
    sf::FloatRect rect = arrow_rect(entity_pos);
    return rect.contains(mouse_pos);
}

void GizmoArrow::paint(sf::RenderWindow& renderer, sf::Vector2f& entity_pos, bool hovered)
{
    sf::Color col = (m_axis == Axis::X) ? sf::Color(220, 60, 60) : sf::Color(60, 180, 60);

    if (hovered) col += sf::Color(50, 50, 50);
    
    sf::FloatRect r = arrow_rect(entity_pos);

    sf::VertexArray vertex_array;
    vertex_array.setPrimitiveType(sf::PrimitiveType::Triangles);

    vertex_array.append(sf::Vertex(sf::Vector2f(r.position.x, r.position.y), col));
    vertex_array.append(sf::Vertex(sf::Vector2f(r.position.x + r.size.x , r.position.y), col));
    vertex_array.append(sf::Vertex(sf::Vector2f(r.position.x, r.position.y + r.size.y), col));

    vertex_array.append(sf::Vertex(sf::Vector2f(r.position.x, r.position.y + r.size.y), col));
    vertex_array.append(sf::Vertex(sf::Vector2f(r.position.x + r.size.x, r.position.y), col));
    vertex_array.append(sf::Vertex(sf::Vector2f(r.position.x + r.size.x, r.position.y + r.size.y), col));

    if (m_axis == Axis::X)
    {
        float tx = r.position.x + r.size.x;
        float ty = entity_pos.y;

        vertex_array.append(sf::Vertex(sf::Vector2f(tx, ty - 8), col));
        vertex_array.append(sf::Vertex(sf::Vector2f(tx + 14, ty), col));
        vertex_array.append(sf::Vertex(sf::Vector2f(tx, ty + 8), col));
        
    }
    else
    {
        float tx = entity_pos.x;
        float ty = r.position.y;
        
        vertex_array.append(sf::Vertex(sf::Vector2f(tx - 8, ty), col));
        vertex_array.append(sf::Vertex(sf::Vector2f(tx, ty - 14), col));
        vertex_array.append(sf::Vertex(sf::Vector2f(tx + 8, ty), col));
    }

    renderer.draw(vertex_array);
}

sf::FloatRect GizmoArrow::arrow_rect(sf::Vector2f& entity_pos)
{
    if (m_axis == Axis::X)
    {
        return sf::FloatRect(
            {entity_pos.x, entity_pos.y - m_thickness * 0.5f},
            {m_length, m_thickness}
        );
    }
    else
    {
        return sf::FloatRect(
            {entity_pos.x - m_thickness * 0.5f, entity_pos.y - m_length},
            {m_thickness, m_length}
        );
    }
}