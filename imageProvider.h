#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QUtil.hh>
#include <qpdf/QPDFWriter.hh>
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

  private:
    int width;
    int height;
    const char *filename;
    QImage *img;
};

