#include <utils/gizmo_view.hpp>
#include <tilemap.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Event.hpp>

#include <QPainter>
#include <QMouseEvent>

#include <algorithm>

GizmoView::GizmoView(QWidget* parent) :
    QWidget(parent),
    m_arrow_x(GizmoArrow::Axis::X),
    m_arrow_y(GizmoArrow::Axis::Y),
    m_selected_axis(GizmoArrow::Axis::None),
    m_layer(nullptr)
{
    
}

static sf::Vector2f clamp_tile_position(const sf::Vector2f& position, const sf::Vector2f& size, const sf::FloatRect& bounds)
{
    sf::Vector2f result = position;
    float min_x = bounds.position.x;
    float min_y = bounds.position.y;
    float max_x = std::max(min_x, bounds.position.x + bounds.size.x - size.x);
    float max_y = std::max(min_y, bounds.position.y + bounds.size.y - size.y);

    result.x = std::clamp(result.x, min_x, max_x);
    result.y = std::clamp(result.y, min_y, max_y);
    
    return result;
}

void GizmoView::event(Tile* tile, const std::optional<sf::Event>& event, sf::RenderWindow& target, const sf::FloatRect& bounds)
{
    if (auto* pressed = event->getIf<sf::Event::MouseButtonPressed>())
    {
        if (pressed->button != sf::Mouse::Button::Left) return;

        sf::Vector2f position_mouse = target.mapPixelToCoords(pressed->position);
        if (m_arrow_x.interact(tile->pos, position_mouse))
        {
            m_drag = { DragState::Dragging, GizmoArrow::Axis::X, position_mouse };
            setCursor(Qt::ClosedHandCursor);
        }
        else if (m_arrow_y.interact(tile->pos, position_mouse))
        {
            m_drag = { DragState::Dragging, GizmoArrow::Axis::Y, position_mouse };
            setCursor(Qt::ClosedHandCursor);
        }
    }
    else if (auto* new_position = event->getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f position_mouse = target.mapPixelToCoords(new_position->position);
        if (m_drag.state == DragState::Dragging)
        {
            sf::Vector2f delta = position_mouse - m_drag.start_mouse;
            
            if (m_drag.active_axis == GizmoArrow::Axis::X)
                tile->pos.x += delta.x;
            else
                tile->pos.y += delta.y;

            tile->pos = clamp_tile_position(tile->pos,
                sf::Vector2f(static_cast<float>(tile->rect.size.x), static_cast<float>(tile->rect.size.y)),
                bounds);

            for (const auto& tile2 : m_layer->tiles)
            {
                if (tile == &tile2) continue;

                if (m_selected_axis == GizmoArrow::Axis::X)
                {
                    float left_tile = tile->pos.x;
                    float left_tile2 = tile2.is_pixel_placed ? tile2.pos.x : tile2.pos.x * tile2.rect.size.x;
                    float right_tile = tile->pos.x + tile->rect.size.x;
                    float right_tile2 = tile2.is_pixel_placed ? tile2.pos.x + tile2.rect.size.x : (tile2.pos.x + 1) * tile2.rect.size.x;

                    if (std::abs(left_tile - right_tile2) <= 5)
                        tile->pos.x = right_tile2;
                    else if (std::abs(left_tile2 - right_tile) <= 5)
                        tile->pos.x = left_tile2 - tile->rect.size.x;
                }
                else
                {
                    float top_tile = tile->pos.y;
                    float top_tile2 = tile2.is_pixel_placed ? tile2.pos.y : tile2.pos.y * tile2.rect.size.y;
                    float bottom_tile = tile->pos.y + tile->rect.size.y;
                    float bottom_tile2 = tile2.is_pixel_placed ? tile2.pos.y + tile2.rect.size.y : (tile2.pos.y + 1) * tile2.rect.size.y;

                    if (std::abs(top_tile - bottom_tile2) <= 5)
                        tile->pos.y = bottom_tile2;
                    else if (std::abs(top_tile2 - bottom_tile) <= 5)
                        tile->pos.y = top_tile2 - tile->rect.size.y;
                }
            }
            m_drag.start_mouse = position_mouse;
        }
        else
        {
            if (m_arrow_x.interact(tile->pos, position_mouse))
            {
                m_selected_axis = GizmoArrow::Axis::X;
                setCursor(Qt::OpenHandCursor);
            }
            else if (m_arrow_y.interact(tile->pos, position_mouse))
            {
                m_selected_axis = GizmoArrow::Axis::Y;
                setCursor(Qt::OpenHandCursor);
            }
            else
            {
                m_selected_axis = GizmoArrow::Axis::None;
                setCursor(Qt::ArrowCursor);
            }
        }
    }
    else if (auto* release = event->getIf<sf::Event::MouseButtonReleased>())
    {
        if (release->button != sf::Mouse::Button::Left) return;
        m_drag.state = DragState::Idle;
        setCursor(Qt::ArrowCursor);
    }
}

bool GizmoView::arrow_handle()
{
    bool handle_x = (m_selected_axis == GizmoArrow::Axis::X) || (m_drag.state == DragState::Dragging && m_drag.active_axis == GizmoArrow::Axis::X);
    bool handle_y = (m_selected_axis == GizmoArrow::Axis::Y) || (m_drag.state == DragState::Dragging && m_drag.active_axis == GizmoArrow::Axis::Y);
    return (handle_x) || (handle_y);
}

void GizmoView::change_layer(Layer* layer)
{
    m_layer = layer;
}

void GizmoView::draw(Tile* tile, sf::RenderWindow& renderer)
{
    bool handle_x = (m_selected_axis == GizmoArrow::Axis::X) || (m_drag.state == DragState::Dragging && m_drag.active_axis == GizmoArrow::Axis::X);
    bool handle_y = (m_selected_axis == GizmoArrow::Axis::Y) || (m_drag.state == DragState::Dragging && m_drag.active_axis == GizmoArrow::Axis::Y);

    m_arrow_x.paint(renderer, tile->pos, handle_x);
    m_arrow_y.paint(renderer, tile->pos, handle_y);

    sf::VertexArray vertex_array;
    vertex_array.setPrimitiveType(sf::PrimitiveType::Triangles);

    sf::Color col(220, 220, 220);
    vertex_array.append({ {tile->pos.x - 6, tile->pos.y - 6}, col });
    vertex_array.append({ {tile->pos.x + 6, tile->pos.y - 6}, col });
    vertex_array.append({ {tile->pos.x - 6, tile->pos.y + 6}, col });

    vertex_array.append({ {tile->pos.x - 6, tile->pos.y + 6}, col });
    vertex_array.append({ {tile->pos.x + 6, tile->pos.y - 6}, col });
    vertex_array.append({ {tile->pos.x + 6, tile->pos.y + 6}, col });

    renderer.draw(vertex_array);
}