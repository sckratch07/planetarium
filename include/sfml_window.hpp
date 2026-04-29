#ifndef SFML_WINDOW_HPP
#define SFML_WINDOW_HPP

#include <SFML/Graphics/RenderWindow.hpp>

#include <QWindow>
#include <QTimer>

#include <memory>

class SfmlWindow : public QWindow
{
    Q_OBJECT
public:
    explicit SfmlWindow(QWindow* parent = nullptr);
    ~SfmlWindow();

    sf::RenderWindow& renderer();

protected:
    void exposeEvent(QExposeEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

signals:
    void event(const std::optional<sf::Event>& event, sf::RenderWindow& target);
    void rendered(sf::RenderWindow& target);

private slots:
    void render();

private:
    void init();

private:
    std::unique_ptr<sf::RenderWindow> m_renderer;

    QTimer m_timer;
    bool m_initialized = false;
};

#endif