#include <tileset_view.hpp>

#include <QGraphicsRectItem>
#include <QWheelEvent>

TilesetView::TilesetView(QWidget* parent) :
    QGraphicsView(parent), m_selection_rect(nullptr)
{

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
    int tileX = static_cast<int>(scenePos.x()) / m_tile_size.x;
    int tileY = static_cast<int>(scenePos.y()) / m_tile_size.y;

    int x = tileX * m_tile_size.x;
    int y = tileY * m_tile_size.y;

    if (!m_selection_rect)
    {
        m_selection_rect = scene()->addRect(
            x, y,
            m_tile_size.x, m_tile_size.y,
            QPen(Qt::red, 2)
        );
        emit selected_rect_changed(m_selection_rect);
    }
    else
    {
        m_selection_rect->setRect(x, y, m_tile_size.x, m_tile_size.y);
    }
}