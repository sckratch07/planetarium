#include <camera.hpp>

Camera::Camera(sf::RenderWindow& window, QObject* parent) :
    QObject(parent), m_view(sf::FloatRect({0.f, 0.f}, sf::Vector2f(window.getSize())))
{
    window.setView(m_view);
}

void Camera::event(const std::optional<sf::Event>& event, sf::RenderWindow& target)
{
    if (!event) return;

    if (auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>())
    {
        float factor = 1.f - wheel->delta * 0.1f;

        m_view.zoom(factor);
        target.setView(m_view);
    }
    else if (auto* resized = event->getIf<sf::Event::Resized>())
    {
        m_view.setSize(static_cast<sf::Vector2f>(resized->size));
        target.setView(m_view);
    }
    else if (auto* key_press = event->getIf<sf::Event::KeyPressed>())
    {
        sf::Vector2f center = m_view.getCenter();
        if (key_press->scancode == sf::Keyboard::Scancode::R)
        {
            m_view.setCenter(static_cast<sf::Vector2f>(target.getSize()) / 2.f);
            m_view.setSize(static_cast<sf::Vector2f>(target.getSize()));
            target.setView(m_view);
        }
        else if (key_press->scancode == sf::Keyboard::Scancode::W)
        {
            center.y -= 10.f;
            m_view.setCenter(center);
            target.setView(m_view);
        }
        else if (key_press->scancode == sf::Keyboard::Scancode::A)
        {
            center.x -= 10.f;
            m_view.setCenter(center);
            target.setView(m_view);
        }
        else if (key_press->scancode == sf::Keyboard::Scancode::S)
        {
            center.y += 10.f;
            m_view.setCenter(center);
            target.setView(m_view);
        }
        else if (key_press->scancode == sf::Keyboard::Scancode::D)
        {
            center.x += 10.f;
            m_view.setCenter(center);
            target.setView(m_view);
        }
    }
}