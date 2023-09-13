#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <Magick++/Image.h>
#include <qpdf/Buffer.hh>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <qrencode.h>

class ImageProvider : public QPDFObjectHandle::StreamDataProvider {
    public:
        ImageProvider(int width, int height);
        ImageProvider(const std::string filename);
        ImageProvider(const QRcode *qr);
        virtual ~ImageProvider();
        virtual void provideStreamData(int objid, int generation,
                                       Pipeline *pipeline);
        int getWidth();
        int getHeight();
        Buffer *getAlpha();

    private:
        int width;
        int height;
        std::string filename;
        Magick::Image img;
        Buffer *alphaBuf;
        unsigned char *alphaData;
        unsigned char *rgbData;
        const QRcode *qr;

        void processImage();
};

#endif // IMAGEPROVIDER_H