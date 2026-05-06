#ifndef SPRITE_SHEET_CUTTING_HPP
#define SPRITE_SHEET_CUTTING_HPP

#include <QImage>
#include <QPixmap>

#include <SFML/Graphics/Image.hpp>

#include <utility>

struct Element
{
    int x, y;
    int width, height;
    std::vector<std::pair<int, int>> pixels;
};

Element cut_sprite_sheet(QImage& image, int x, int y);

sf::Image qimage_to_sfimage(const QImage& qimage, Element element);

QPixmap sfimage_to_qpixmap(const sf::Image& image);

#endif