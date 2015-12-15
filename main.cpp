#include <iostream>
#include <qpdf/QPDF.hh>
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
    image.replaceDict(QPDFObjectHandle::parse(
                          "<<"
                          " /Type /XObject"
                          " /Subtype /Image"
                          " /ColorSpace /DeviceRGB"
                          " /BitsPerComponent 8"
                          " /Width 100"
                          " /Height 100"
                          ">>"));
    // Provide the stream data.
    ImageProvider* p = new ImageProvider(100, 100);
    PointerHolder<QPDFObjectHandle::StreamDataProvider> provider(p);
    image.replaceStreamData(provider,
                            QPDFObjectHandle::newNull(),
                            QPDFObjectHandle::newNull());
    xobject.replaceKey("/ImEPStamp55", image);
    
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, "q 1 0 0 1 50 289 cm /ImEPStamp55 Do Q\n"), true);
    //QPDFObjectHandle contents = firstPage.getKey("/Contents");
    //unsigned const char *stream = contents.getStreamData().getPointer()->getBuffer();
    
    //cout << stream << endl;
    
    QPDFWriter w(pdf, "a.pdf");
    w.write();

    return 0;
}
