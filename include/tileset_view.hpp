#ifndef TILESET_VIEW_HPP
#define TILESET_VIEW_HPP

#include <QGraphicsView>
#include <QImage>
#include <SFML/System/Vector2.hpp>

class TilesetView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit TilesetView(QWidget* parent = nullptr);

    void reset_rect();
    void set_auto_selection_mode(bool enabled);
    void set_current_image(const QImage& img);

public slots:
    void tile_size_changed(const sf::Vector2i& new_size);

signals:
    void selected_rect_changed(QGraphicsRectItem* rect);
    void selected_rect_dimensions_changed(int width_tiles, int height_tiles);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    static constexpr int DRAG_THRESHOLD = 3; // pixels

    QGraphicsRectItem* m_selection_rect;
    sf::Vector2f m_tile_size;
    bool m_is_dragging;
    bool m_actual_drag_started;
    QPointF m_drag_start;
    int m_start_tile_x;
    int m_start_tile_y;
    int m_selected_width_tiles;
    int m_selected_height_tiles;
    bool m_auto_selection_mode;
    QImage m_current_image;
};

#endif