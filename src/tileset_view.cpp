#include <tileset_view.hpp>

#include <QGraphicsRectItem>
#include <QWheelEvent>
#include <algorithm>
#include <utils/sprite_sheet_cutting.hpp>

TilesetView::TilesetView(QWidget* parent) :
    QGraphicsView(parent), m_selection_rect(nullptr), m_tile_size({32, 32}),
    m_is_dragging(false), m_actual_drag_started(false), m_drag_start({0, 0}), m_start_tile_x(0), m_start_tile_y(0),
    m_selected_width_tiles(1), m_selected_height_tiles(1), m_auto_selection_mode(false)
{

}

void TilesetView::reset_rect()
{
    if (!m_selection_rect)
        return;

    QGraphicsScene* s = scene();
    if (s && m_selection_rect->scene() == s)
        s->removeItem(m_selection_rect);

    delete m_selection_rect;
    m_selection_rect = nullptr;
    emit selected_rect_changed(nullptr);
}

void TilesetView::set_auto_selection_mode(bool enabled)
{
    m_auto_selection_mode = enabled;
}

void TilesetView::set_current_image(const QImage& img)
{
    m_current_image = img;
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

    if (m_auto_selection_mode)
    {
        QPointF scene_pos = mapToScene(event->pos());

        int x = static_cast<int>(scene_pos.x());
        int y = static_cast<int>(scene_pos.y());

        Element elem = cut_sprite_sheet(m_current_image, x, y);
        if (elem.width > 0 && elem.height > 0)
        {
            if (m_selection_rect)
            {
                scene()->removeItem(m_selection_rect);
                delete m_selection_rect;
                m_selection_rect = nullptr;
            }

            m_selection_rect = scene()->addRect(elem.x, elem.y, elem.width, elem.height, QPen(Qt::red, 2));

            emit selected_rect_changed(m_selection_rect);
            
            int width_tiles = (elem.width + static_cast<int>(m_tile_size.x) - 1) / static_cast<int>(m_tile_size.x);
            int height_tiles = (elem.height + static_cast<int>(m_tile_size.y) - 1) / static_cast<int>(m_tile_size.y);

            emit selected_rect_dimensions_changed(width_tiles, height_tiles);
        }
        return;
    }

    QPointF scene_pos = mapToScene(event->pos());
    m_start_tile_x = static_cast<int>(scene_pos.x()) / m_tile_size.x;
    m_start_tile_y = static_cast<int>(scene_pos.y()) / m_tile_size.y;
    m_drag_start = scene_pos;
    
    m_is_dragging = true;
    m_actual_drag_started = false;
}

void TilesetView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_auto_selection_mode) return;

    if (!m_is_dragging || !scene()) return;

    QPointF scenePos = mapToScene(event->pos());

    // Check if drag threshold has been exceeded
    if (!m_actual_drag_started)
    {
        QPointF delta = scenePos - m_drag_start;
        if (std::abs(delta.x()) < DRAG_THRESHOLD && std::abs(delta.y()) < DRAG_THRESHOLD)
            return; // Haven't moved far enough to start actual dragging
        m_actual_drag_started = true;
    }

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
    if (m_auto_selection_mode) return;

    if (m_is_dragging && !m_actual_drag_started)
    {
        // Simple click without dragging - select single tile
        QPointF scenePos = mapToScene(event->pos());
        int tile_x = static_cast<int>(scenePos.x()) / m_tile_size.x;
        int tile_y = static_cast<int>(scenePos.y()) / m_tile_size.y;

        int x = tile_x * m_tile_size.x;
        int y = tile_y * m_tile_size.y;
        int width = m_tile_size.x;
        int height = m_tile_size.y;

        if (!m_selection_rect)
        {
            m_selection_rect = scene()->addRect(
                x, y, width, height,
                QPen(Qt::red, 2)
            );
            emit selected_rect_changed(m_selection_rect);
        }
        else
        {
            m_selection_rect->setRect(x, y, width, height);
        }
        m_selected_width_tiles = 1;
        m_selected_height_tiles = 1;
        emit selected_rect_dimensions_changed(1, 1);
    }

    m_is_dragging = false;
    m_actual_drag_started = false;
}