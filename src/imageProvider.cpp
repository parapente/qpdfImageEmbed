#include "imageProvider.h"
#include <QtCore/QDebug>

ImageProvider::ImageProvider(int width, int height)
    : width(width), height(height) {}

ImageProvider::ImageProvider(const char *filename) : filename(filename) {
  img.read(filename);
  Magick::Geometry geometry = img.size();
  width = geometry.width();
  height = geometry.height();

  qDebug() << "Image width: " << width;
  qDebug() << "Image height: " << height;

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