#include "imageProvider.h"

ImageProvider::ImageProvider(int width, int height) :
    width(width),
    height(height)
{
}

ImageProvider::ImageProvider(const char *filename) :
    filename(filename)
{
    img = new QImage(filename);
    width = img->width();
    height = img->height();
    
    std::cout << "Image width: " << width << std::endl;
    std::cout << "Image height: " << height << std::endl;
}

ImageProvider::~ImageProvider()
{
}

void
ImageProvider::provideStreamData(int objid, int generation,
                                 Pipeline* pipeline)
{
    if (!filename) {
        for (int i = 0; i < width * height; ++i)
        {
            pipeline->write(QUtil::unsigned_char_pointer("\xff\x7f\x00"), 3);
        }
    }
    else {
        pipeline->write(img->bits(),width*height*3);
    }
    pipeline->finish();
}

int ImageProvider::getHeight()
{
    return height;
}

int ImageProvider::getWidth()
{
    return width;
}
