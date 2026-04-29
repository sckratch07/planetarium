#include <map_preview.hpp>

#include <QVBoxLayout>

MapPreview::MapPreview(QWidget* parent) : QWidget(parent)
{
    QWidget* container = QWidget::createWindowContainer(&m_sfml_window, this);
    container->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(container, 0);
}

void MapPreview::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}