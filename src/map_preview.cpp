#include <map_preview.hpp>
#include <camera.hpp>

#include <QVBoxLayout>

MapPreview::MapPreview(QWidget* parent) : QWidget(parent)
{
    QWidget* container = QWidget::createWindowContainer(&m_sfml_window, this);
    container->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(container);

    auto* camera = new Camera(m_sfml_window.renderer(), this);
    connect(&m_sfml_window, &SfmlWindow::event, camera, &Camera::event);
}

MapPreview::~MapPreview()
{
}

SfmlWindow& MapPreview::sfml_window()
{
    return m_sfml_window;
}