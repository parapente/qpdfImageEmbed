#include <iostream>
#include <cmath>
#include <qpdf/QPDF.hh>
#include <QtCore/QString>
#include <QtCore/QDebug>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include "imageProvider.h"

using namespace std;
using namespace boost::program_options;

int main(int argc, char *argv[])
{
    QPDF pdf;
    string inpdf, outpdf, imgfile;
    int side=0, rotate=0, assumerotate=-1;
    QRectF pageRect;

    // parse options using boost::program_options
    options_description desc("Allowed options:");
    desc.add_options()
        ("help,h", "Produce this help message")
        ("input-file,i", value<string>(),"Input file")
        ("stamp,s", value<string>(), "Image to embed")
        ("output-file,o", value<string>(), "Output file")
        ("side", value<int>(), "Side of the document: 0 center (default), 1 left, 2 right")
        ("rotate", value<int>(), "Assume page is rotated by 0/90/180/270 degrees")
    ;
    
    positional_options_description posdes;
    posdes.add("input-file", 1);
    posdes.add("stamp", 1);
    posdes.add("output-file", 1);
    posdes.add("side", 1);
    posdes.add("rotate", 1);
    
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
    if (vm.count("rotate")) {
        assumerotate = vm["rotate"].as<int>();
        if ((assumerotate !=0) && (assumerotate !=90) &&
            (assumerotate !=180) && (assumerotate !=270)) {
            cout << "Valid rotate values are 0,90,180 or 270 only." << endl;
            return 2;
        }
    }

    qDebug() << side;
    pdf.processFile(inpdf.c_str());
    qDebug() << QString::fromStdString(pdf.getPDFVersion());

    vector<QPDFObjectHandle> pages;
    pages = pdf.getAllPages();
    qDebug() << "Pages:" << pages.size();
    
    QPDFObjectHandle firstPage = pages.at(0);
    
    qDebug() << "Has Contents:" << firstPage.hasKey("/Contents");
    qDebug() << "Has MediaBox:" << firstPage.hasKey("/MediaBox");
    QPDFObjectHandle mediabox;
    if (!firstPage.hasKey("/MediaBox")) { // No MediaBox? Use default values for letter size
        mediabox = QPDFObjectHandle::newArray();
        mediabox.appendItem(QPDFObjectHandle::newInteger(0));
        mediabox.appendItem(QPDFObjectHandle::newInteger(0));
        mediabox.appendItem(QPDFObjectHandle::newInteger(612));
        mediabox.appendItem(QPDFObjectHandle::newInteger(792));
        pageRect.setCoords(0, 0, 612, 792);
    }
    else {
        mediabox = firstPage.getKey("/MediaBox");
        pageRect.setCoords(mediabox.getArrayItem(0).getNumericValue(), mediabox.getArrayItem(1).getNumericValue(),
                           mediabox.getArrayItem(2).getNumericValue(), mediabox.getArrayItem(3).getNumericValue());
    }
    qDebug() << "--> MediaBox : ";
    for (int i=0; i<mediabox.getArrayNItems(); i++) {
        qDebug() << mediabox.getArrayItem(i).getNumericValue() << " ";
    }
    qDebug() << "Has CropBox:" << firstPage.hasKey("/CropBox");
    if (firstPage.hasKey("/CropBox")) {
        QPDFObjectHandle cropbox = firstPage.getKey("/CropBox");
        qDebug() << "--> CropBox : ";
        for (int i=0; i<cropbox.getArrayNItems(); i++) {
            qDebug() << cropbox.getArrayItem(i).getNumericValue() << " ";
        }
        pageRect.setCoords(cropbox.getArrayItem(0).getNumericValue(), cropbox.getArrayItem(1).getNumericValue(),
                           cropbox.getArrayItem(2).getNumericValue(), cropbox.getArrayItem(3).getNumericValue());
    }
    qDebug() << "Has Rotate:" << firstPage.hasKey("/Rotate");
    if (firstPage.hasKey("/Rotate")) {
        QPDFObjectHandle rotateObj = firstPage.getKey("/Rotate");
        rotate = rotateObj.getNumericValue();
        qDebug() << "--> Rotate :" << rotate;
        
    }
    // Override rotation
    if (assumerotate!=-1) {
        rotate = assumerotate;
    }
    qDebug() << "Has Resouces:" << firstPage.hasKey("/Resources");
    
    QPDFObjectHandle resources = firstPage.getKey("/Resources");
    qDebug() << "Has Resouces->XObject:" << resources.hasKey("/XObject");
    
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
    qDebug() << imgstr.toLocal8Bit().constData();
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
    qDebug() << trimgstr.toLocal8Bit().constData();
    transparency.replaceDict(QPDFObjectHandle::parse(trimgstr.toLocal8Bit().constData()));
    // Provide the stream data.
    PointerHolder<Buffer> trprovider(p->getAlpha());
    qDebug() << "Buffer size: " << trprovider.getPointer()->getSize();
    transparency.replaceStreamData(trprovider,
                            QPDFObjectHandle::newNull(),
                            QPDFObjectHandle::newNull());
    xobject.replaceKey("/ImEPStamp55tr", transparency);
    QString smaskstr = QString(QString::number(transparency.getObjectID()) + " " + QString::number(transparency.getGeneration()) + " R");
    pdf.updateAllPagesCache();
    image.getDict().replaceKey("/SMask", transparency);
    
    // To prevent our image appearing in unexpected places we save the initial state at the beginning of the page
    // and restore it at the end before adding our image.
    // Thanks go to David van Driessche @StackOverflow for this elegant solution
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, "q\n"), true);
    QString streamstr;
    int topMargin = 20;
    int sideMargin = 20;
    int imgHeight, imgWidth;
    float pageHeight, pageWidth;
    pageHeight = pageRect.height();
    pageWidth = pageRect.width();
    imgHeight = p->getHeight();
    imgWidth = p->getWidth();
    streamstr = QString("Q q ");
    // If the page is rotated
    if (rotate != 0) {
        int cq, sq;
        int tx, ty;
        cq = cos((double)rotate * M_PI / 180);
        sq = sin((double)rotate * M_PI / 180);
        if (rotate == 90) {
            tx = pageRect.y();
            ty = -pageRect.x()-pageWidth;
        }
        else if (rotate == 180) {
            tx = -pageRect.x()-pageWidth;
            ty = -pageRect.y()-pageHeight;
        }
        else {
            tx = -pageRect.y()-pageHeight;
            ty = pageRect.x();
        }
        streamstr += QString::number(cq) + " " + QString::number(sq) + " " + QString::number(-sq) + " " + QString::number(cq) + " 0 0 cm ";
        streamstr += QString("1 0 0 1 ") + QString::number(tx) + " " + QString::number(ty) + " cm ";
    }
    else {
        streamstr += QString("1 0 0 1 ") + QString::number(pageRect.x()) + " " + QString::number(pageRect.y()) + " cm ";
    }

    // Set the apropriate image scaling according to page rotation
    streamstr += QString::number(0.75*imgWidth) + " 0 0 " + QString::number(0.75*imgHeight) + " ";

    int imgtx, imgty;
    if (rotate == 90 || rotate == 270) {
        if (side == 0 )
            imgtx = (pageHeight - 0.75*imgWidth)/2;
        else if (side == 1)
            imgtx = mediabox.getArrayItem(1).getNumericValue() + sideMargin;
        else
            imgtx = pageHeight - 0.75*imgWidth - sideMargin;
        imgty = pageWidth - imgHeight - topMargin;
    }
    else {
        if (side == 0 )
            imgtx = (pageWidth - 0.75*imgWidth)/2;
        else if (side == 1)
            imgtx = mediabox.getArrayItem(0).getNumericValue() + sideMargin;
        else
            imgtx = pageWidth - 0.75*imgWidth - sideMargin;
        imgty = pageHeight - imgHeight - topMargin;
    }
    streamstr += QString::number(imgtx) + " " + QString::number(imgty);
    streamstr += " cm /ImEPStamp55 Do Q\n";
    qDebug() << "Stream str: " << streamstr;
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, streamstr.toStdString()), false);

    QPDFWriter w(pdf, outpdf.c_str());
    w.write();

    return 0;
}
