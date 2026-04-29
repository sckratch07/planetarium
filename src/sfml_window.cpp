#include <sfml_window.hpp>

SfmlWindow::SfmlWindow(QWindow* parent) :
    QWindow(parent)
{
    setSurfaceType(QSurface::OpenGLSurface);
    connect(&m_timer, &QTimer::timeout, this, &SfmlWindow::render);
    m_timer.start(16);
}

SfmlWindow::~SfmlWindow()
{
    if (m_renderer)
        m_renderer->close();
}

void SfmlWindow::init()
{
    if (m_initialized) return;

    m_renderer = std::make_unique<sf::RenderWindow>(reinterpret_cast<sf::WindowHandle>(winId()));
    m_renderer->setVerticalSyncEnabled(true);

    m_initialized = true;
}

void SfmlWindow::exposeEvent(QExposeEvent*)
{
    if (isExposed())
    {
        init();
        render();
    }
}

void SfmlWindow::resizeEvent(QResizeEvent* event)
{
    if (!m_initialized) return;
}

void SfmlWindow::render()
{
    if (!isExposed() || !m_initialized) return;

    while (const std::optional<sf::Event> event = m_renderer->pollEvent())
    {
        
    }

    m_renderer->clear(sf::Color(50, 50, 50));

    m_renderer->display();
}