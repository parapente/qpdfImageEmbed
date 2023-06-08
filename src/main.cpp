#include <cmath>
#include <qpdf/QPDF.hh>
#include <qrencode.h>

#include "config.h"
#include "imageProvider.h"
#include "logger.h"
#include "options.h"
#include "rect.h"

Logger logger;

int main(int argc, char *argv[]) {
    QPDF pdf;
    int rotate = 0;
    Rect pageRect;
    std::unordered_map<std::string, std::variant<std::string, int>> cliOption;

    cliOption = readCLIOptions(argc, argv);

    pdf.processFile(std::get<std::string>(cliOption["inputPDF"]).c_str());
    logger << "PDF version: " << pdf.getPDFVersion() << "\n";

    std::vector<QPDFObjectHandle> pages;
    pages = pdf.getAllPages();
    logger << "Pages: " << pages.size() << "\n";

    QPDFObjectHandle firstPage = pages.at(0);

    logger << "Has Contents: " << firstPage.hasKey("/Contents") << "\n";
    logger << "Has MediaBox: " << firstPage.hasKey("/MediaBox") << "\n";
    QPDFObjectHandle mediabox;
    if (!firstPage.hasKey(
            "/MediaBox")) { // No MediaBox? Use default values for letter size
        mediabox = QPDFObjectHandle::newArray();
        mediabox.appendItem(QPDFObjectHandle::newInteger(0));
        mediabox.appendItem(QPDFObjectHandle::newInteger(0));
        mediabox.appendItem(QPDFObjectHandle::newInteger(612));
        mediabox.appendItem(QPDFObjectHandle::newInteger(792));
        pageRect.setCoords(0, 0, 612, 792);
    } else {
        mediabox = firstPage.getKey("/MediaBox");
        pageRect.setCoords(mediabox.getArrayItem(0).getNumericValue(),
                           mediabox.getArrayItem(1).getNumericValue(),
                           mediabox.getArrayItem(2).getNumericValue(),
                           mediabox.getArrayItem(3).getNumericValue());
    }
    logger << "--> MediaBox: ";
    for (int i = 0; i < mediabox.getArrayNItems(); i++) {
        logger << mediabox.getArrayItem(i).getNumericValue() << " ";
    }
    logger << "\nHas CropBox: " << firstPage.hasKey("/CropBox") << "\n";
    if (firstPage.hasKey("/CropBox")) {
        QPDFObjectHandle cropbox = firstPage.getKey("/CropBox");
        logger << "--> CropBox: ";
        for (int i = 0; i < cropbox.getArrayNItems(); i++) {
            logger << cropbox.getArrayItem(i).getNumericValue() << " ";
        }
        pageRect.setCoords(cropbox.getArrayItem(0).getNumericValue(),
                           cropbox.getArrayItem(1).getNumericValue(),
                           cropbox.getArrayItem(2).getNumericValue(),
                           cropbox.getArrayItem(3).getNumericValue());
    }
    logger << "\nHas Rotate: " << firstPage.hasKey("/Rotate") << "\n";
    if (firstPage.hasKey("/Rotate")) {
        QPDFObjectHandle rotateObj = firstPage.getKey("/Rotate");
        rotate = rotateObj.getNumericValue();
        logger << "--> Rotate: " << rotate << "\n";
    }
    // Override rotation
    if (cliOption.contains("rotate")) {
        rotate = std::get<int>(cliOption["rotate"]);
    }
    logger << "Has Resources: " << firstPage.hasKey("/Resources") << "\n";

    QPDFObjectHandle resources = firstPage.getKey("/Resources");
    logger << "Has Resources->XObject: " << resources.hasKey("/XObject")
           << "\n";

    QPDFObjectHandle xobject;
    if (!resources.hasKey("/XObject")) {
        xobject = QPDFObjectHandle::newDictionary();
        resources.replaceKey("/XObject", xobject);
    } else
        xobject = resources.getKey("/XObject");

    // Image object
    QPDFObjectHandle image = QPDFObjectHandle::newStream(&pdf);
    ImageProvider *p = new ImageProvider(
        std::get<std::string>(cliOption["imageFile"]).c_str());
    std::string imageString = std::string("<<"
                                          " /Type /XObject"
                                          " /Subtype /Image"
                                          " /ColorSpace /DeviceRGB"
                                          " /BitsPerComponent 8"
                                          " /Width ") +
                              std::to_string(p->getWidth()) + " /Height " +
                              std::to_string(p->getHeight()) + ">>";
    logger << "Image string: " << imageString << "\n";
    image.replaceDict(QPDFObjectHandle::parse(imageString));
    // Provide the stream data.
    std::shared_ptr<QPDFObjectHandle::StreamDataProvider> provider(p);
    image.replaceStreamData(provider, QPDFObjectHandle::newNull(),
                            QPDFObjectHandle::newNull());
    xobject.replaceKey("/ImEPStamp55", image);

    // Transparency object
    QPDFObjectHandle transparency = QPDFObjectHandle::newStream(&pdf);
    std::string transparencyImageString = std::string("<<"
                                                      " /Type /XObject"
                                                      " /Subtype /Image"
                                                      " /ColorSpace /DeviceGray"
                                                      " /BitsPerComponent 8"
                                                      " /Decode [0.0 1.0]"
                                                      " /Width ") +
                                          std::to_string(p->getWidth()) +
                                          " /Height " +
                                          std::to_string(p->getHeight()) + ">>";
    logger << "Transparency image string: " << transparencyImageString << "\n";
    transparency.replaceDict(QPDFObjectHandle::parse(transparencyImageString));
    // Provide the stream data.
    std::shared_ptr<Buffer> transparencyProvider(p->getAlpha());
    logger << "Buffer size: " << transparencyProvider.get()->getSize() << "\n";
    transparency.replaceStreamData(transparencyProvider,
                                   QPDFObjectHandle::newNull(),
                                   QPDFObjectHandle::newNull());
    xobject.replaceKey("/ImEPStamp55tr", transparency);
    std::string smaskString =
        std::string(std::to_string(transparency.getObjectID()) + " " +
                    std::to_string(transparency.getGeneration()) + " R");
    pdf.updateAllPagesCache();
    image.getDict().replaceKey("/SMask", transparency);

    // To prevent our image appearing in unexpected places we save the initial
    // state at the beginning of the page and restore it at the end before
    // adding our image. Thanks go to David van Driessche @StackOverflow for
    // this elegant solution
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, "q\n"), true);
    std::string streamString;
    int topMargin = 0;
    int sideMargin = 20;
    int imgHeight, imgWidth;
    float pageHeight, pageWidth;
    pageHeight = pageRect.height();
    pageWidth = pageRect.width();
    imgHeight = p->getHeight();
    imgWidth = p->getWidth();
    streamString = std::string("Q q ");
    // If the page is rotated
    if (rotate != 0) {
        int cq, sq;
        int tx, ty;
        cq = cos((double)rotate * M_PI / 180);
        sq = sin((double)rotate * M_PI / 180);
        if (rotate == 90) {
            tx = pageRect.y();
            ty = -pageRect.x() - pageWidth;
        } else if (rotate == 180) {
            tx = -pageRect.x() - pageWidth;
            ty = -pageRect.y() - pageHeight;
        } else {
            tx = -pageRect.y() - pageHeight;
            ty = pageRect.x();
        }
        streamString += std::to_string(cq) + " " + std::to_string(sq) + " " +
                        std::to_string(-sq) + " " + std::to_string(cq) +
                        " 0 0 cm ";
        streamString += std::string("1 0 0 1 ") + std::to_string(tx) + " " +
                        std::to_string(ty) + " cm ";
    } else {
        streamString += std::string("1 0 0 1 ") + std::to_string(pageRect.x()) +
                        " " + std::to_string(pageRect.y()) + " cm ";
    }

    // Set the appropriate image scaling according to page rotation
    streamString += std::to_string(0.57 * imgWidth) + " 0 0 " +
                    std::to_string(0.57 * imgHeight) + " ";

    int imgtx, imgty;
    if (rotate == 90 || rotate == 270) {
        if (std::get<int>(cliOption["side"]) == 0)
            imgtx = (pageHeight - 0.57 * imgWidth) / 2;
        else if (std::get<int>(cliOption["side"]) == 1)
            imgtx = mediabox.getArrayItem(1).getNumericValue() + sideMargin;
        else
            imgtx = pageHeight - 0.57 * imgWidth - sideMargin;
        imgty = pageWidth - imgHeight - topMargin;
    } else {
        if (std::get<int>(cliOption["side"]) == 0)
            imgtx = (pageWidth - 0.57 * imgWidth) / 2;
        else if (std::get<int>(cliOption["side"]) == 1)
            imgtx = mediabox.getArrayItem(0).getNumericValue() + sideMargin;
        else
            imgtx = pageWidth - 0.57 * imgWidth - sideMargin;
        imgty = pageHeight - imgHeight - topMargin;
    }
    streamString += std::to_string(imgtx) + " " + std::to_string(imgty);
    streamString += " cm /ImEPStamp55 Do Q\n";
    logger << "Stream str: " << streamString << "\n";
    firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, streamString),
                              false);

    QPDFWriter w(pdf, std::get<std::string>(cliOption["outputPDF"]).c_str());
    w.write();

    return 0;
}
