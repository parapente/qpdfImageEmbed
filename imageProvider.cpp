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

    alphaData = new unsigned char[width*height];
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            alphaData[y*width+x] = (unsigned char) qAlpha(img.pixel(x,y));
            //std::cout << (int) alphaData[i];
        }
    alphaBuf = new Buffer(alphaData, width*height);
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
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
                {
                    pixel[0] = (unsigned char) qRed(img.pixel(x,y));
                    pixel[1] = (unsigned char) qGreen(img.pixel(x,y));
                    pixel[2] = (unsigned char) qBlue(img.pixel(x,y));
                    //std::cout << (int) pixel[0] << "," << (int) pixel[1] << "," << (int) pixel[2] << " - ";
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

Buffer *ImageProvider::getAlpha()
{
    return alphaBuf;
}