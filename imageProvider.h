#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QUtil.hh>
#include <qpdf/QPDFWriter.hh>

class ImageProvider: public QPDFObjectHandle::StreamDataProvider
{
  public:
    ImageProvider(int width, int height);
    virtual ~ImageProvider();
    virtual void provideStreamData(int objid, int generation, Pipeline* pipeline);

  private:
    int width;
    int height;
};

