#include <map_preview.hpp>
#include <camera.hpp>

#include <QSettings>
#include <QVBoxLayout>

MapPreview::MapPreview(QWidget* parent) : QWidget(parent)
{
    QSettings settings("Game Academy", "EarthPower");

    settings.beginGroup("map_preview");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();

    QWidget* container = QWidget::createWindowContainer(&m_sfml_window, this);
    container->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(container, 0);

    auto* camera = new Camera(m_sfml_window.renderer(), this);
    connect(&m_sfml_window, &SfmlWindow::event, camera, &Camera::event);
}

MapPreview::~MapPreview()
{
    QSettings settings("Game Academy", "EarthPower");

    settings.beginGroup("map_preview");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

SfmlWindow& MapPreview::sfml_window()
{
    return m_sfml_window;
}