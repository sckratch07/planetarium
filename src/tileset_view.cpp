#include <tileset_view.hpp>

#include <QGraphicsRectItem>
#include <QWheelEvent>
#include <algorithm>

TilesetView::TilesetView(QWidget* parent) :
    QGraphicsView(parent), m_selection_rect(nullptr), m_tile_size({32, 32}),
    m_is_dragging(false), m_drag_start({0, 0}), m_start_tile_x(0), m_start_tile_y(0),
    m_selected_width_tiles(1), m_selected_height_tiles(1)
{

}

void TilesetView::reset_rect()
{
    if (m_selection_rect)
    {
        if (scene())
            scene()->removeItem(m_selection_rect);
        delete m_selection_rect;
        m_selection_rect = nullptr;
    }
    emit selected_rect_changed(nullptr);
}

void TilesetView::tile_size_changed(const sf::Vector2i& new_size)
{
    m_tile_size = sf::Vector2f(new_size);
}

void TilesetView::wheelEvent(QWheelEvent* event)
{
    double current_zoom = transform().m11();
    if (current_zoom < 0.1 && event->angleDelta().y() < 0 || current_zoom > 10.0 && event->angleDelta().y() > 0) return;

    float factor = 1.15f;
    if (event->angleDelta().y() > 0)
        scale(factor, factor);
    else
        scale(1.0 / factor, 1.0 / factor);
}

void TilesetView::mousePressEvent(QMouseEvent* event)
{
    if (!scene()) return;

    QPointF scenePos = mapToScene(event->pos());
    m_start_tile_x = static_cast<int>(scenePos.x()) / m_tile_size.x;
    m_start_tile_y = static_cast<int>(scenePos.y()) / m_tile_size.y;
    m_drag_start = scenePos;
    m_is_dragging = true;
}

void TilesetView::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_is_dragging || !scene()) return;

    QPointF scenePos = mapToScene(event->pos());
    int current_tile_x = static_cast<int>(scenePos.x()) / m_tile_size.x;
    int current_tile_y = static_cast<int>(scenePos.y()) / m_tile_size.y;

    // Ensure we have valid tile coordinates
    int min_x = std::min(m_start_tile_x, current_tile_x);
    int min_y = std::min(m_start_tile_y, current_tile_y);
    int max_x = std::max(m_start_tile_x, current_tile_x);
    int max_y = std::max(m_start_tile_y, current_tile_y);

    // Calculate tile dimensions
    m_selected_width_tiles = max_x - min_x + 1;
    m_selected_height_tiles = max_y - min_y + 1;

    int x = min_x * m_tile_size.x;
    int y = min_y * m_tile_size.y;
    int width = m_selected_width_tiles * m_tile_size.x;
    int height = m_selected_height_tiles * m_tile_size.y;

    if (!m_selection_rect)
    {
        m_selection_rect = scene()->addRect(
            x, y, width, height,
            QPen(Qt::red, 2)
        );
        emit selected_rect_changed(m_selection_rect);
        emit selected_rect_dimensions_changed(m_selected_width_tiles, m_selected_height_tiles);
    }
    else
    {
        m_selection_rect->setRect(x, y, width, height);
        emit selected_rect_dimensions_changed(m_selected_width_tiles, m_selected_height_tiles);
    }
}

void TilesetView::mouseReleaseEvent(QMouseEvent* event)
{
    m_is_dragging = false;
}