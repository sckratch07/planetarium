#ifndef GRID_HPP
#define GRID_HPP

#include <QWidget>
#include <QString>
#include <SFML/Graphics/RenderWindow.hpp>

class QListWidget;
class QSpinBox;

class Grid : public QWidget
{
    Q_OBJECT
public:
    explicit Grid(QWidget* parent = nullptr);
    ~Grid() override;

    const sf::Vector2i& grid_size() const;
    const sf::Vector2i& cell_size() const;

public slots:
    void draw(sf::RenderWindow& target) const;
    void clear_type_selection();
    void set_grid_size(const sf::Vector2i& new_size);
    void set_cell_size(const sf::Vector2i& new_size);
    void set_types(const QStringList& types);

signals:
    void grid_size_changed(const sf::Vector2i& new_size);
    void cell_size_changed(const sf::Vector2i& new_size);
    void tile_type_changed(const QString& type);
    void tile_type_added(const QString& type);
    void tile_type_removed(const QString& type);
    void tile_type_selection_cleared();

private:
    sf::Vector2i m_grid_size;
    sf::Vector2i m_cell_size;
    QSpinBox* m_grid_size_width;
    QSpinBox* m_grid_size_height;
    QSpinBox* m_cell_size_width;
    QSpinBox* m_cell_size_height;
    QListWidget* m_type_list;
    QString m_selected_type;
};

#endif