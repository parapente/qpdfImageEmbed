#include "imageProvider.h"
#include "logger.h"

ImageProvider::ImageProvider(int width, int height)
    : width(width), height(height) {}

ImageProvider::ImageProvider(const char *filename) : filename(filename) {
    img.read(filename);
    Magick::Geometry geometry = img.size();
    width = geometry.width();
    height = geometry.height();

    logger << "Image width: " << width << "\n";
    logger << "Image height: " << height << "\n";

    alphaData = new unsigned char[width * height];
    img.write(0, 0, width, height, "A", MagickCore::StorageType::CharPixel,
              alphaData);
    alphaBuf = new Buffer(alphaData, width * height);

    rgbData = new unsigned char[width * height * 3];
    img.write(0, 0, width, height, "RGB", MagickCore::StorageType::CharPixel,
              rgbData);
}

ImageProvider::~ImageProvider() {}

void ImageProvider::provideStreamData(int objid, int generation,
                                      Pipeline *pipeline) {
    if (!filename) {
        for (int i = 0; i < width * height; ++i) {
            pipeline->write(QUtil::unsigned_char_pointer("\xff\x7f\x00"), 3);
        }
    } else {
        pipeline->write(rgbData, width * height * 3);
    }
    pipeline->finish();
}

int ImageProvider::getHeight() { return height; }

int ImageProvider::getWidth() { return width; }

Buffer *ImageProvider::getAlpha() { return alphaBuf; }