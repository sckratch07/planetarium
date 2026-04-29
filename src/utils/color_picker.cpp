#include <utils/color_picker.hpp>

#include <QPainter>
#include <QMouseEvent>
#include <QBoxLayout>

ColorPicker::ColorPicker(QWidget* parent) :
    QWidget(parent), m_hue(0), m_sat(1), m_val(1)
{
    setMinimumSize(200, 200);
}

void ColorPicker::set_hue(double h)
{
    m_hue = h;
    m_dirty = true;
    update();
}

QColor ColorPicker::color() const
{
    return QColor::fromHsvF(m_hue, m_sat, m_val);
}

void ColorPicker::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    if (m_dirty || m_cached.size() != size())
    {
        m_cached = QImage(width(), height(), QImage::Format_RGB32);

        for (int y = 0; y < m_cached.height(); ++y)
        {
            for (int x = 0; x < m_cached.width(); ++x)
            {
                double s = (double)x / m_cached.width();
                double v = 1.0 - (double)y / m_cached.height();
                QColor c = QColor::fromHsvF(m_hue, s, v);
                QRgb* line = reinterpret_cast<QRgb*>(m_cached.scanLine(y));
                line[x] = c.rgb();
            }
        }

        m_dirty = false;
    }

    p.drawImage(rect(), m_cached);

    int px = m_sat * width();
    int py = (1 - m_val) * height();

    p.setPen(Qt::black);
    p.drawEllipse(QPoint(px, py), 5, 5);
    p.setPen(Qt::white);
    p.drawEllipse(QPoint(px, py), 6, 6);
}

void ColorPicker::mousePressEvent(QMouseEvent* e)
{
    update_from_pos(e->pos());
}

void ColorPicker::mouseMoveEvent(QMouseEvent* e)
{
    update_from_pos(e->pos());
}

void ColorPicker::resizeEvent(QResizeEvent*)
{
    m_dirty = true;
}

void ColorPicker::update_from_pos(const QPoint& pos)
{
    m_sat = std::clamp((double)pos.x() / width(), 0.0, 1.0);
    m_val = std::clamp(1.0 - (double)pos.y() / height(), 0.0, 1.0);

    emit color_changed(color());
    update();
}

HueSlider::HueSlider(QWidget* parent) :
    QWidget(parent), m_hue(0)
{
    setFixedWidth(20);
}

double HueSlider::hue() const
{
    return m_hue;
}

void HueSlider::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    if (m_dirty || m_cached.size() != size())
    {
        m_cached = QImage(width(), height(), QImage::Format_RGB32);

        for (int y = 0; y < m_cached.height(); ++y)
        {
            double h = (double)y / m_cached.height();
            QColor c = QColor::fromHsvF(h, 1, 1);
            QRgb* line = reinterpret_cast<QRgb*>(m_cached.scanLine(y));
            for (int x = 0; x < m_cached.width(); ++x)
                line[x] = c.rgb();
        }

        m_dirty = false;
    }

    p.fillRect(rect(), m_cached);

    int y = m_hue * height();
    p.setPen(Qt::white);
    p.drawLine(0, y, width(), y);
}

void HueSlider::mousePressEvent(QMouseEvent* e)
{
    update_hue(e->pos());
}

void HueSlider::mouseMoveEvent(QMouseEvent* e)
{
    update_hue(e->pos());
}

void HueSlider::resizeEvent(QResizeEvent*)
{
    m_dirty = true;
}

void HueSlider::update_hue(const QPoint& pos)
{
    m_hue = std::clamp((double)pos.y() / height(), 0.0, 1.0);
    emit hue_changed(m_hue);
    update();
}


ColorPickerWidget::ColorPickerWidget(QWidget* parent) :
    QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);

    m_picker = new ColorPicker(this);
    m_hue = new HueSlider(this);

    layout->addWidget(m_picker);
    layout->addWidget(m_hue);

    connect(m_hue, &HueSlider::hue_changed, this, [this](double h)
        {
            m_picker->set_hue(h);
            emit color_changed(m_picker->color());
        }
    );

    connect(m_picker, &ColorPicker::color_changed, this, &ColorPickerWidget::color_changed);
}