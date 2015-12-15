#include <iostream>
#include <qpdf/QPDF.hh>
#include <QtCore/QString>
#include "imageProvider.h"

using namespace std;

int main(int argc, char *argv[])
{
    cout << "Hello world!" << endl;
    QPDF pdf;

    pdf.processFile(argv[1]);
    cout << pdf.getPDFVersion() << endl;

    vector<QPDFObjectHandle> pages;
    pages = pdf.getAllPages();
    cout << "Pages:" << pages.size() << endl;
    
    QPDFObjectHandle firstPage = pages.at(0);
    
    cout << "Has Contents:" << firstPage.hasKey("/Contents") << endl;
    cout << "Has Resouces:" << firstPage.hasKey("/Resources") << endl;
    
    QPDFObjectHandle resources = firstPage.getKey("/Resources");
    cout << "Has Resouces->XObject:" << resources.hasKey("/XObject") << endl;
    
    QPDFObjectHandle xobject = resources.getKey("/XObject");
    QPDFObjectHandle image = QPDFObjectHandle::newStream(&pdf);
    ImageProvider* p = new ImageProvider("protocol-image.png");
    QString imgstr = QString("<<"
                          " /Type /XObject"
                          " /Subtype /Image"
                          " /ColorSpace /DeviceRGB"
                          " /BitsPerComponent 8"
                          " /Width ") + QString::number(p->getWidth()) +
                          " /Height " + QString::number(p->getHeight()) +
                          ">>";
    cout << imgstr.toLocal8Bit().constData() << endl;
    image.replaceDict(QPDFObjectHandle::parse(imgstr.toLocal8Bit().constData()));
    // Provide the stream data.
    PointerHolder<QPDFObjectHandle::StreamDataProvider> provider(p);
    image.replaceStreamData(provider,
                            QPDFObjectHandle::newNull(),
                            QPDFObjectHandle::newNull());
    xobject.replaceKey("/ImEPStamp55", image);
    
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, "q 100 0 0 100 50 289 cm /ImEPStamp55 Do Q\n"), true);
    //QPDFObjectHandle contents = firstPage.getKey("/Contents");
    //unsigned const char *stream = contents.getStreamData().getPointer()->getBuffer();
    
    //cout << stream << endl;
    
    QPDFWriter w(pdf, "a.pdf");
    w.write();

    return 0;
}
