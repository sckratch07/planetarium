#ifndef MAP_PREVIEW_HPP
#define MAP_PREVIEW_HPP

#include <sfml_window.hpp>

#include <QWidget>

class MapPreview : public QWidget
{
    Q_OBJECT
public:
    explicit MapPreview(QWidget* parent = nullptr);
    ~MapPreview() override;

    SfmlWindow& sfml_window();

private:
    SfmlWindow m_sfml_window;
};

#endif