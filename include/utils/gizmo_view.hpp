#ifndef GIZMO_VIEW_HPP
#define GIZMO_VIEW_HPP

#include <utils/gizmo_arrow.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <QWidget>
#include <optional>

namespace sf
{
    class Event;
}

struct Tile;
struct Layer;

enum DragState { Idle, Hovering, Dragging };

struct Drag
{
    DragState state = DragState::Idle;
    GizmoArrow::Axis active_axis = GizmoArrow::Axis::X;
    sf::Vector2f start_mouse;
};

class GizmoView : public QWidget
{
    Q_OBJECT
public:
    explicit GizmoView(QWidget* parent = nullptr);

    void event(Tile* tile, const std::optional<sf::Event>& event, sf::RenderWindow& target, const sf::FloatRect& bounds);
    void draw(Tile* tile, sf::RenderWindow& target);

    bool arrow_handle();
    void change_layer(Layer* layer);

private:
    GizmoArrow m_arrow_x;
    GizmoArrow m_arrow_y;
    
    Drag m_drag;
    GizmoArrow::Axis m_selected_axis;

    Layer* m_layer;
};

#endif