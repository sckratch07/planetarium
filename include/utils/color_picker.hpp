#ifndef COLOR_PICKER_HPP
#define COLOR_PICKER_HPP

#include <QWidget>

class ColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QWidget* parent = nullptr);

    void set_hue(double h);
    QColor color() const;

signals:
    void color_changed(const QColor&);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    void update_from_pos(const QPoint& pos);

private:
    QImage m_cached;
    bool m_dirty = true;

    double m_hue;
    double m_sat;
    double m_val;
};


class HueSlider : public QWidget
{
    Q_OBJECT
public:
    explicit HueSlider(QWidget* parent = nullptr);

    double hue() const;

signals:
    void hue_changed(double);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    void update_hue(const QPoint& pos);

private:
    QImage m_cached;
    bool m_dirty = true;

    double m_hue;
};


class ColorPickerWidget : public QWidget
{
    Q_OBJECT
public:
    ColorPickerWidget(QWidget* parent = nullptr);

signals:
    void color_changed(const QColor&);

private:
    ColorPicker* m_picker;
    HueSlider* m_hue;
};

#endif