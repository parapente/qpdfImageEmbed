#include "imageProvider.h"

ImageProvider::ImageProvider(int width, int height) :
    width(width),
    height(height)
{
}

ImageProvider::~ImageProvider()
{
}

void
ImageProvider::provideStreamData(int objid, int generation,
                                 Pipeline* pipeline)
{
    for (int i = 0; i < width * height; ++i)
    {
        pipeline->write(QUtil::unsigned_char_pointer("\xff\x7f\x00"), 3);
    }
    pipeline->finish();
}
