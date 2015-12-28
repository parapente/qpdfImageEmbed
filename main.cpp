#include <iostream>
#include <cmath>
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
    int side=0, rotate=0;

    // parse options using boost::program_options
    options_description desc("Allowed options:");
    desc.add_options()
        ("help,h", "Produce this help message")
        ("input-file,i", value<string>(),"Input file")
        ("output-file,o", value<string>(), "Output file")
        ("stamp,s", value<string>(), "Image to embed")
        ("side", value<int>(), "Side of the document: 0 center (default), 1 left, 2 right")
    ;
    
    positional_options_description posdes;
    posdes.add("input-file", 1);
    posdes.add("stamp", 1);
    posdes.add("output-file", 1);
    posdes.add("side", 1);
    
    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).positional(posdes).run(), vm);
    notify(vm);
    
    if ((argc <4) || vm.empty() || vm.count("help")) {
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

    if (vm.count("side")) {
        side = vm["side"].as<int>();
    }

    cout << side <<endl;
    pdf.processFile(inpdf.c_str());
    cout << pdf.getPDFVersion() << endl;

    vector<QPDFObjectHandle> pages;
    pages = pdf.getAllPages();
    cout << "Pages:" << pages.size() << endl;
    
    QPDFObjectHandle firstPage = pages.at(0);
    
    cout << "Has Contents:" << firstPage.hasKey("/Contents") << endl;
    cout << "Has MediaBox:" << firstPage.hasKey("/MediaBox") << endl;
    QPDFObjectHandle mediabox;
    if (!firstPage.hasKey("/MediaBox")) {
        mediabox = QPDFObjectHandle::newArray();
        mediabox.appendItem(QPDFObjectHandle::newInteger(0));
        mediabox.appendItem(QPDFObjectHandle::newInteger(0));
        mediabox.appendItem(QPDFObjectHandle::newInteger(612));
        mediabox.appendItem(QPDFObjectHandle::newInteger(792));
    }
    else
        mediabox = firstPage.getKey("/MediaBox");
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
        QPDFObjectHandle rotateObj = firstPage.getKey("/Rotate");
        rotate = rotateObj.getNumericValue();
        cout << "--> Rotate :" << rotate << endl;
        
    }
    cout << "Has Resouces:" << firstPage.hasKey("/Resources") << endl;
    
    QPDFObjectHandle resources = firstPage.getKey("/Resources");
    cout << "Has Resouces->XObject:" << resources.hasKey("/XObject") << endl;
    
    QPDFObjectHandle xobject;
    if (!resources.hasKey("/XObject")) {
        xobject = QPDFObjectHandle::newDictionary();
        resources.replaceKey("/XObject", xobject);
    }
    else 
        xobject = resources.getKey("/XObject");
    
    // Image object
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
    
    // Transparency object
    QPDFObjectHandle transparency = QPDFObjectHandle::newStream(&pdf);
    QString trimgstr = QString("<<"
                          " /Type /XObject"
                          " /Subtype /Image"
                          " /ColorSpace /DeviceGray"
                          " /BitsPerComponent 8"
                          " /Decode [0.0 1.0]"
                          " /Width ") + QString::number(p->getWidth()) +
                          " /Height " + QString::number(p->getHeight()) +
                          ">>";
    cout << trimgstr.toLocal8Bit().constData() << endl;
    transparency.replaceDict(QPDFObjectHandle::parse(trimgstr.toLocal8Bit().constData()));
    // Provide the stream data.
    PointerHolder<Buffer> trprovider(p->getAlpha());
    cout << "Buffer size: " << trprovider.getPointer()->getSize() << endl;
    transparency.replaceStreamData(trprovider,
                            QPDFObjectHandle::newNull(),
                            QPDFObjectHandle::newNull());
    xobject.replaceKey("/ImEPStamp55tr", transparency);
    QString smaskstr = QString(QString::number(transparency.getObjectID()) + " " + QString::number(transparency.getGeneration()) + " R");
    pdf.updateAllPagesCache();
    cout << "/SMask " << smaskstr.toStdString() << endl;
    cout << "/SMask " << transparency.getObjGen().getObj() << endl;
    image.getDict().replaceKey("/SMask", QPDFObjectHandle::newOperator(smaskstr.toStdString()));
    
    // To prevent our image appearing in unexpected places we save the initial state at the beginning of the page
    // and restore it at the end before adding our image.
    // Thanks go to David van Driessche @StackOverflow for this elegant solution
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, "q\n"), true);
    QString streamstr;
    int topMargin = 20;
    int sideMargin = 20;
    int imgHeight, imgWidth;
    float pageHeight, pageWidth;
    pageHeight = mediabox.getArrayItem(3).getNumericValue() - mediabox.getArrayItem(1).getNumericValue();
    pageWidth = mediabox.getArrayItem(2).getNumericValue() - mediabox.getArrayItem(0).getNumericValue();
    imgHeight = p->getHeight();
    imgWidth = p->getWidth();
    streamstr = QString("Q q ");
    // If the page is rotated
    if (rotate != 0) {
        double cq, sq;
        int tx, ty;
        cq = cos((double)rotate * M_PI / 180);
        sq = sin((double)rotate * M_PI / 180);
        if (rotate == 90) {
            tx = 0;
            ty = -pageWidth;
        }
        else if (rotate == 180) {
            tx = -pageWidth;
            ty = -pageHeight;
        }
        else {
            tx = -pageHeight;
            ty = -pageWidth;
        }
        streamstr += QString::number(cq, 'f', 3) + " " + QString::number(sq, 'f', 3) + " -" + QString::number(sq, 'f', 3) + " " + QString::number(cq, 'f', 3) + " 0 0 cm ";
        streamstr += QString("1 0 0 1 ") + QString::number(tx) + " " + QString::number(ty) + " cm ";
    }

    // Set the apropriate image scaling according to page rotation
    streamstr += QString::number(imgWidth) + " 0 0 " + QString::number(imgHeight) + " ";

    int imgtx, imgty;
    if (rotate == 90 || rotate == 270) {
        if (side == 0 )
            imgtx = (pageHeight - imgWidth)/2;
        else if (side == 1)
            imgtx = mediabox.getArrayItem(1).getNumericValue() + sideMargin;
        else
            imgtx = pageHeight - imgWidth - sideMargin;
        imgty = pageWidth - imgHeight - topMargin;
    }
    else {
        if (side == 0 )
            imgtx = (pageWidth - imgWidth)/2;
        else if (side == 1)
            imgtx = mediabox.getArrayItem(0).getNumericValue() + sideMargin;
        else
            imgtx = pageWidth - imgWidth - sideMargin;
        imgty = pageHeight - imgHeight - topMargin;
    }
    streamstr += QString::number(imgtx) + " " + QString::number(imgty);
    streamstr += " cm /ImEPStamp55 Do Q\n";
    cout << "Stream str: " << streamstr.toStdString() << endl;
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, streamstr.toStdString()), false);

    cout << "/SMask " << transparency.getObjGen().getObj() << endl;
    
    QPDFWriter w(pdf, outpdf.c_str());
    w.write();
    cout << "/SMask " << transparency.getObjGen().getObj() << endl;

    return 0;
}
