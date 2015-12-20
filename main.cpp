#include <iostream>
#include <qpdf/QPDF.hh>
#include <QtCore/QString>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include "imageProvider.h"

using namespace std;
using namespace boost::program_options;

int main(int argc, char *argv[])
{
    QPDF pdf;
    string inpdf, outpdf, imgfile;

    // parse options using boost::program_options
    options_description desc("Allowed options:");
    desc.add_options()
        ("help,h", "Produce this help message")
        ("input-file,i", value<string>(),"Input file")
        ("output-file,o", value<string>(), "Output file")
        ("stamp,s", value<string>(), "Image to embed")
    ;
    
    positional_options_description posdes;
    posdes.add("input-file", 1);
    posdes.add("stamp", 1);
    posdes.add("output-file", 1);
    
    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).positional(posdes).run(), vm);
    notify(vm);
    
    if ((argc !=4) || vm.empty() || vm.count("help")) {
        cout << desc << endl;
        return 1;
    }
    
    if (vm.count("input-file")) {
        inpdf = vm["input-file"].as<string>();
    }

    if (vm.count("output-file")) {
        outpdf = vm["output-file"].as<string>();
    }

    if (vm.count("stamp")) {
        imgfile = vm["stamp"].as<string>();
    }

    pdf.processFile(inpdf.c_str());
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
    ImageProvider* p = new ImageProvider(imgfile.c_str());
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
    
    // To prevent our image appearing in unexpected places we save the initial state at the beginning of the page
    // and restore it at the end before adding our image.
    // Thanks go to David van Driessche @StackOverflow for this elegant solution
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, "q\n"), true);
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, "Q q 286 0 0 99 100 50 cm /ImEPStamp55 Do Q\n"), false);
    //QPDFObjectHandle contents = firstPage.getKey("/Contents");
    //unsigned const char *stream = contents.getStreamData().getPointer()->getBuffer();
    
    //cout << stream << endl;
    
    QPDFWriter w(pdf, outpdf.c_str());
    w.write();

    return 0;
}
