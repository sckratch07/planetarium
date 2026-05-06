#include <utils/sprite_sheet_cutting.hpp>

#include <vector>
#include <queue>
#include <iostream>

Element cut_sprite_sheet(QImage &image, int x, int y)
{    
    // Image Size
    int image_width = image.size().width();
    int image_height = image.size().height();

    // Bounds
    int min_x = x;
    int min_y = y;
    int max_x = x;
    int max_y = y;
    
    const int alpha_minimum = 10;
    
    // Visited Pixels
    std::vector<std::vector<bool>> visited(image_width, std::vector<bool>(image_height, false));

    // All asset pixel
    std::vector<std::pair<int, int>> pixels;
    std::vector<std::pair<int, int>> pixels_keep;
    pixels.push_back({x, y});

    // Flood Fill
    while(!pixels.empty()){
        // Get first pixel of the queue
        auto pixel = pixels.front();
        pixels.erase(pixels.begin());

        // Coords of the pixel
        auto px = pixel.first;
        auto py = pixel.second;

        // Check if pixel is out of image
        if(px < 0 || py < 0 || px > image_width || py > image_height) continue;
        // Check if pixel have been visited before
        if(visited[px][py]) continue;
        // Check if the alpha value of the pixel is under the minimum alpha value accepted
        if(qAlpha(image.pixel({px, py})) <= alpha_minimum) continue;

        // Set visited value to true
        visited[px][py] = true;
        
        pixels_keep.push_back(pixel);

        // Set the new bounds
        min_x = std::min(min_x, px);
        min_y = std::min(min_y, py);
        max_x = std::max(max_x, px);
        max_y = std::max(max_y, py);

        // Add next pixel to check
        pixels.push_back({px+1, py});
        pixels.push_back({px-1, py});
        pixels.push_back({px, py+1});
        pixels.push_back({px, py-1});

    }
    return Element(min_x, min_y, max_x - min_x, max_y - min_y, std::vector<std::pair<int, int>>(pixels_keep.begin(), pixels_keep.end()));
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
