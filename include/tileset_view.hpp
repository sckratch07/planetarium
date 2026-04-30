#ifndef TILESET_VIEW_HPP
#define TILESET_VIEW_HPP

#include <QGraphicsView>
#include <SFML/System/Vector2.hpp>

class TilesetView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit TilesetView(QWidget* parent = nullptr);

    void reset_rect();

public slots:
    void tile_size_changed(const sf::Vector2i& new_size);

signals:
    void selected_rect_changed(QGraphicsRectItem* rect);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QGraphicsRectItem* m_selection_rect;
    sf::Vector2f m_tile_size;
};

#endif