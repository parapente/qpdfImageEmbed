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
    cout << "Has MediaBox:" << firstPage.hasKey("/MediaBox") << endl;
    QPDFObjectHandle mediabox = firstPage.getKey("/MediaBox");
    cout << "--> MediaBox : ";
    for (int i=0; i<mediabox.getArrayNItems(); i++) {
        cout << mediabox.getArrayItem(i).getNumericValue() << " ";
    }
    cout << endl;
    cout << "Has CropBox:" << firstPage.hasKey("/CropBox") << endl;
    if (firstPage.hasKey("/CrobBox")) {
        QPDFObjectHandle cropbox = firstPage.getKey("/CropBox");
        cout << "--> CropBox : ";
        for (int i=0; i<cropbox.getArrayNItems(); i++) {
            cout << cropbox.getArrayItem(i).getNumericValue() << " ";
        }
        cout << endl;
    }
    cout << "Has Rotate:" << firstPage.hasKey("/Rotate") << endl;
    if (firstPage.hasKey("/Rotate")) {
        QPDFObjectHandle rotate = firstPage.getKey("/Rotate");
        cout << "--> Rotate :" << rotate.getNumericValue();
    }
    cout << "Has Resouces:" << firstPage.hasKey("/Resources") << endl;
    
    QPDFObjectHandle resources = firstPage.getKey("/Resources");
    cout << "Has Resouces->XObject:" << resources.hasKey("/XObject") << endl;
    
    QPDFObjectHandle xobject = resources.getKey("/XObject");
    QPDFObjectHandle image = QPDFObjectHandle::newStream(&pdf);
    ImageProvider* p = new ImageProvider("protocol-imageb.png");
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
    
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, "q 286 0 0 99 100 50 cm /ImEPStamp55 Do Q\n"), false);
    //QPDFObjectHandle contents = firstPage.getKey("/Contents");
    //unsigned const char *stream = contents.getStreamData().getPointer()->getBuffer();
    
    //cout << stream << endl;
    
    QPDFWriter w(pdf, "a.pdf");
    w.write();

    return 0;
}
