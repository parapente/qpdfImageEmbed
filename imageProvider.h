#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QUtil.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/Buffer.hh>
#include <QtGui/QImage>

class ImageProvider: public QPDFObjectHandle::StreamDataProvider
{
  public:
    ImageProvider(int width, int height);
    ImageProvider(const char *filename);
    virtual ~ImageProvider();
    virtual void provideStreamData(int objid, int generation, Pipeline* pipeline);
    int getWidth();
    int getHeight();
    Buffer *getAlpha();

  private:
    int width;
    int height;
    const char *filename;
    QImage img;
    Buffer *alphaBuf;
    unsigned char *alphaData;
};

