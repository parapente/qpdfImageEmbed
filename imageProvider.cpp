#include "imageProvider.h"

ImageProvider::ImageProvider(int width, int height) :
    width(width),
    height(height)
{
}

ImageProvider::ImageProvider(const char *filename) :
    filename(filename)
{
    img = QImage(filename); //.convertToFormat(QImage::Format_RGB888);
    //img = QImage(filename).convertToFormat(QImage::Format_RGB888);
    std::cout << "Format: " << img.format() << std::endl;
    width = img.width();
    height = img.height();
    
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
        QRgb *p;
        unsigned char pixel[3];
        for (int i = 0; i < width*height; i++)
        {
            p = (QRgb *) (img.bits()+i*4);
            pixel[0] = qRed(*p);
            pixel[1] = qGreen(*p);
            pixel[2] = qBlue(*p);
            pipeline->write(pixel,3);
        }
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
