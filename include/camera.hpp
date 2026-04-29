#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <QObject>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class Camera : public QObject
{
    Q_OBJECT
public:
    explicit Camera(sf::RenderWindow& window, QObject* parent = nullptr);
    
public slots:
    void event(const std::optional<sf::Event>& event, sf::RenderWindow& target);

private:
    sf::View m_view;
    float m_zoom = 1.f;
};

#endif