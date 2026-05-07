#include <utils/sprite_sheet_cutting.hpp>

#include <vector>
#include <queue>
#include <iostream>

Element cut_sprite_sheet(QImage& image, int x, int y)
{
    const int width = image.width();
    const int height = image.height();

    constexpr int alpha_minimum = 10;

    int min_x = x;
    int min_y = y;
    int max_x = x;
    int max_y = y;

    std::vector<uint8_t> visited(width * height, 0);

    std::queue<std::pair<int, int>> queue;
    std::vector<std::pair<int, int>> pixels_keep;
    queue.push({x, y});

    while (!queue.empty())
    {
        auto [px, py] = queue.front();
        queue.pop();

        if (px < 0 || py < 0 || px >= width || py >= height) continue;
        int index = py * width + px;

        if (visited[index]) continue;

        visited[index] = 1;

        const QRgb* line = reinterpret_cast<const QRgb*>(image.scanLine(py));
        if (qAlpha(line[px]) <= alpha_minimum) continue;

        pixels_keep.push_back({px, py});

        min_x = std::min(min_x, px);
        min_y = std::min(min_y, py);
        max_x = std::max(max_x, px);
        max_y = std::max(max_y, py);

        queue.push({px + 1, py});
        queue.push({px - 1, py});
        queue.push({px, py + 1});
        queue.push({px, py - 1});
    }

    return Element(
        min_x,
        min_y,
        max_x - min_x + 1,
        max_y - min_y + 1,
        std::move(pixels_keep)
    );
}

sf::Image qimage_to_sfimage(const QImage& qimage, Element element)
{
    sf::Image sfimage;
    sfimage.resize({(unsigned int)element.width, (unsigned int)element.height});
    
    for(auto [x, y] : element.pixels)
    {
        QColor c = qimage.pixelColor(x, y);
        unsigned int pos_x = x - element.x;
        unsigned int pos_y = y - element.y;
        
        if (pos_x < 0 || pos_x >= sfimage.getSize().x) continue;
        if (pos_y < 0 || pos_y >= sfimage.getSize().y) continue;
        
        sfimage.setPixel({pos_x, pos_y}, sf::Color((uint8_t)c.red(), (uint8_t)c.green(), (uint8_t)c.blue(), (uint8_t)c.alpha()));
    }

    return sfimage;
}

QPixmap sfimage_to_qpixmap(const sf::Image& image)
{
    sf::Vector2u size = image.getSize();
    const uint8_t* pixels = image.getPixelsPtr();

    QImage qimg(pixels, size.x, size.y, QImage::Format_RGBA8888);

    return QPixmap::fromImage(qimg);
}
