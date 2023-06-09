#include "imageProvider.h"
#include "logger.h"

ImageProvider::ImageProvider(int width, int height)
    : width(width), height(height) {}

ImageProvider::ImageProvider(const std::string filename) : filename(filename) {
    img.read(filename);
    processImage();
}

ImageProvider::ImageProvider(const QRcode *qr) {
    Magick::Image image(Magick::Geometry(qr->width * 2, qr->width * 2),
                        Magick::Color("none"));

    Magick::Color black("black");
    image.fillColor(black);
    image.strokeColor(black);

    int cellSize = 1; // Size of each QR cell

    for (int row = 0; row < qr->width; row++) {
        for (int col = 0; col < qr->width; col++) {
            if ((qr->data[row * qr->width + col] & 1) == 1) {
                int x = col * cellSize * 2; // Calculate the x-coordinate
                int y = row * cellSize * 2; // Calculate the y-coordinate

                Magick::DrawableRectangle rect(x, y, x + cellSize,
                                               y + cellSize);
                image.draw(rect);
            }
        }
    }

    img = image;
    processImage();
}

ImageProvider::~ImageProvider() {}

void ImageProvider::provideStreamData(int objid, int generation,
                                      Pipeline *pipeline) {
    if (!filename.empty()) {
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

void ImageProvider::processImage() {
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
