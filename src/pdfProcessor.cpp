#include "pdfProcessor.h"
#include "imageProvider.h"
#include "logger.h"
#include <qpdf/QPDFMatrix.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <random>

PDFProcessor::PDFProcessor(/* args */) {}

PDFProcessor::~PDFProcessor() {}

bool PDFProcessor::open(const std::string filename) {
    try {
        m_pdf.processFile(filename.c_str());
    } catch (std::runtime_error &e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        std::cerr << "Does " << filename << " exist?" << std::endl;
        exit(4);
    }
    logger << "PDF version: " << m_pdf.getPDFVersion() << "\n";

    std::vector<QPDFObjectHandle> pages;
    pages = m_pdf.getAllPages();
    logger << "Pages: " << pages.size() << "\n";

    m_firstPage = pages.at(0);

    logger << "Has Contents: " << m_firstPage.hasKey("/Contents") << "\n";
    logger << "Has MediaBox: " << m_firstPage.hasKey("/MediaBox") << "\n";

    if (!m_firstPage.hasKey(
            "/MediaBox")) { // No MediaBox? Use default values for letter size
        m_mediabox = QPDFObjectHandle::newArray();
        m_mediabox.appendItem(QPDFObjectHandle::newInteger(0));
        m_mediabox.appendItem(QPDFObjectHandle::newInteger(0));
        m_mediabox.appendItem(QPDFObjectHandle::newInteger(612));
        m_mediabox.appendItem(QPDFObjectHandle::newInteger(792));
        m_pageRect.setCoords(0, 0, 612, 792);
    } else {
        m_mediabox = m_firstPage.getKey("/MediaBox");
        m_pageRect.setCoords(m_mediabox.getArrayItem(0).getNumericValue(),
                             m_mediabox.getArrayItem(1).getNumericValue(),
                             m_mediabox.getArrayItem(2).getNumericValue(),
                             m_mediabox.getArrayItem(3).getNumericValue());
    }
    logger << "--> MediaBox: ";
    for (int i = 0; i < m_mediabox.getArrayNItems(); i++) {
        logger << m_mediabox.getArrayItem(i).getNumericValue() << " ";
    }
    logger << "\nHas CropBox: " << m_firstPage.hasKey("/CropBox") << "\n";
    if (m_firstPage.hasKey("/CropBox")) {
        QPDFObjectHandle cropbox = m_firstPage.getKey("/CropBox");
        logger << "--> CropBox: ";
        for (int i = 0; i < cropbox.getArrayNItems(); i++) {
            logger << cropbox.getArrayItem(i).getNumericValue() << " ";
        }
        m_pageRect.setCoords(cropbox.getArrayItem(0).getNumericValue(),
                             cropbox.getArrayItem(1).getNumericValue(),
                             cropbox.getArrayItem(2).getNumericValue(),
                             cropbox.getArrayItem(3).getNumericValue());
    }
    logger << "\nHas Rotate: " << m_firstPage.hasKey("/Rotate") << "\n";
    if (m_firstPage.hasKey("/Rotate")) {
        QPDFObjectHandle rotateObj = m_firstPage.getKey("/Rotate");
        m_rotate = rotateObj.getNumericValue();
        logger << "--> Rotate: " << m_rotate << "\n";
    }

    logger << "Has Resources: " << m_firstPage.hasKey("/Resources") << "\n";

    QPDFObjectHandle resources = m_firstPage.getKey("/Resources");
    logger << "Has Resources->XObject: " << resources.hasKey("/XObject")
           << "\n";

    if (!resources.hasKey("/XObject")) {
        m_xobject = QPDFObjectHandle::newDictionary();
        resources.replaceKey("/XObject", m_xobject);
    } else
        m_xobject = resources.getKey("/XObject");

    return true;
}

void PDFProcessor::rotate(int degrees) {
    // Override rotation
    m_rotate = degrees;
}

void PDFProcessor::setPosition(int side) {
    m_side = side;
    logger << "Side: " << side << "\n";
}

std::string PDFProcessor::rand_str(int length) {
    std::string chars{"abcdefghijklmnopqrstuvwxyz1234567890"};
    std::random_device rd;
    std::mt19937 generator(rd());

    std::string output;
    output.reserve(length);

    while (length > 0) {
        auto randNumb = generator();
        while (randNumb > 36 && length--) {
            output.push_back(chars[randNumb % 36]);
            randNumb /= 36;
        }
    }

    return output;
}

std::string PDFProcessor::createNewImageName(std::string prefix) {
    std::string newName;
    bool nameFound = true;

    do {
        newName = prefix + rand_str(6);
        QPDFObjectHandle check = m_xobject.getKey("/" + newName);
        if (check.isNull()) {
            nameFound = false;
        }
    } while (nameFound);

    return newName;
}

void PDFProcessor::createImageStream(ImageProvider *p, std::string name) {
    // Image object
    QPDFObjectHandle image = QPDFObjectHandle::newStream(&m_pdf);
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
    m_xobject.replaceKey("/" + name, image);

    // Transparency object
    QPDFObjectHandle transparency = QPDFObjectHandle::newStream(&m_pdf);
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
    m_xobject.replaceKey("/" + name + "tr", transparency);
    std::string smaskString =
        std::string(std::to_string(transparency.getObjectID()) + " " +
                    std::to_string(transparency.getGeneration()) + " R");
    m_pdf.updateAllPagesCache();
    image.getDict().replaceKey("/SMask", transparency);
}

void PDFProcessor::addImage(ImageProvider *p, float scale, float topMargin,
                            float sideMargin, std::string link,
                            Point *exactPosition) {

    std::string imageName = createNewImageName("ImEPStampR");
    createImageStream(p, imageName);

    // To prevent our image appearing in unexpected places we save the initial
    // state at the beginning of the page and restore it at the end before
    // adding our image. Thanks go to David van Driessche @StackOverflow for
    // this elegant solution
    const std::string saveState = std::string("Q q ");

    m_firstPage.addPageContents(QPDFObjectHandle::newStream(&m_pdf, "q\n"),
                                true);

    std::string streamString;
    int imgHeight, imgWidth;
    float pageHeight, pageWidth;

    if (m_rotate == 90 || m_rotate == 270) {
        pageHeight = m_pageRect.width();
        pageWidth = m_pageRect.height();
    } else {
        pageHeight = m_pageRect.height();
        pageWidth = m_pageRect.width();
    }
    imgHeight = p->getHeight();
    imgWidth = p->getWidth();

    QPDFMatrix pageRotation, pageTranslation, pageTranslation2,
        imgTransformation;

    // If the page is rotated
    if (m_rotate != 0) {
        pageRotation.rotatex90(m_rotate);

        streamString += pageRotation.unparse() + " cm ";

        if (m_rotate == 90) {
            pageTranslation.translate(0, -pageHeight);
        } else if (m_rotate == 180) {
            pageTranslation.translate(-pageWidth, -pageHeight);
        } else if (m_rotate == 270) {
            pageTranslation.translate(-pageWidth, 0);
        }

        streamString += pageTranslation.unparse() + " cm ";
    }

    // Translation
    double imgTranslateX, imgTranslateY;

    if (exactPosition == nullptr) {
        if (m_side == 0) { // Center
            imgTranslateX = m_pageRect.x() + (pageWidth - scale * imgWidth) / 2;
        } else if (m_side == 1) // Left
            imgTranslateX = m_pageRect.x() + sideMargin;
        else // Right
            imgTranslateX = pageWidth - scale * imgWidth - sideMargin;
        imgTranslateY = pageHeight - scale * imgHeight - topMargin;
    } else {
        imgTranslateX = exactPosition->x();
        imgTranslateY = exactPosition->y();
    }

    pageTranslation2.translate(imgTranslateX, imgTranslateY);
    streamString += pageTranslation2.unparse() + " cm ";

    // Scale
    imgTransformation.scale(scale * imgWidth, scale * imgHeight);
    streamString += imgTransformation.unparse() + " cm ";

    logger << "pageHeight: " << pageHeight << "\n";
    logger << "pageWidth: " << pageWidth << "\n";

    // Check if we have a link annotation
    if (link.empty()) {
        streamString += "/" + imageName + " Do Q\n";
    } else {
        streamString += "/" + imageName + " Do Q\n";

        // Calculate the annotation position
        // (it doesn't follow the transformations applied above)
        double x1, y1, x2, y2;
        if (m_rotate == 0) {
            x1 = imgTranslateX;
            x2 = x1 + scale * imgWidth;
            y1 = imgTranslateY;
            y2 = y1 + scale * imgHeight;
        } else if (m_rotate == 90) {
            x1 = pageHeight - imgTranslateY - scale * imgHeight;
            x2 = x1 + scale * imgHeight;
            y1 = imgTranslateX;
            y2 = y1 + scale * imgWidth;
        } else if (m_rotate == 180) {
            x1 = pageWidth - imgTranslateX - scale * imgWidth;
            x2 = x1 + scale * imgWidth;
            y1 = pageHeight - imgTranslateY - scale * imgHeight;
            y2 = y1 + scale * imgHeight;
        } else { // 270
            x1 = imgTranslateY;
            x2 = x1 + scale * imgHeight;
            y1 = pageWidth - imgTranslateX - scale * imgWidth;
            y2 = y1 + scale * imgWidth;
        }

        QPDFObjectHandle annots = m_firstPage.getKeyIfDict("/Annots");

        QPDFObjectHandle newAnnotation = QPDFObjectHandle::newDictionary();
        newAnnotation.replaceKey("/Type", QPDFObjectHandle::newName("/Annot"));
        newAnnotation.replaceKey("/Subtype",
                                 QPDFObjectHandle::newName("/Link"));

        logger << "Rect: " << x1 << " " << y1 << " " << x2 << " " << y2 << "\n";
        newAnnotation.replaceKey(
            "/Rect", QPDFObjectHandle::newFromRectangle(
                         QPDFObjectHandle::Rectangle(x1, y1, x2, y2)));

        QPDFObjectHandle anchor = QPDFObjectHandle::newDictionary();
        anchor.replaceKey("/Type", QPDFObjectHandle::newName("/Action"));
        anchor.replaceKey("/S", QPDFObjectHandle::newName("/URI"));
        anchor.replaceKey("/URI", QPDFObjectHandle::newString(link));
        newAnnotation.replaceKey("/A", anchor);

        if (annots.isArray()) {
            logger << "Appending to annots array\n";
            annots.appendItem(newAnnotation);
        } else {
            logger << "Adding annots array\n";
            QPDFObjectHandle newAnnots = QPDFObjectHandle::newArray();
            newAnnots.appendItem(newAnnotation);
            m_firstPage.replaceKey("/Annots", newAnnots);
        }
    }

    logger << "Stream str: " << saveState + streamString << "\n";

    m_firstPage.addPageContents(
        QPDFObjectHandle::newStream(&m_pdf, saveState + streamString), false);
}

void PDFProcessor::addExtraText(std::string text, float x, float y,
                                float font_size, std::string basefont,
                                std::string style) {
    const std::string basefont_full_name =
        basefont + (style == "" ? "" : "-" + style);
    // Create a new font dictionary
    QPDFObjectHandle newFont = QPDFObjectHandle::parse("<<\n"
                                                       "  /Type /Font\n"
                                                       "  /Subtype /Type1\n"
                                                       "  /BaseFont /" +
                                                       basefont_full_name +
                                                       "\n"
                                                       ">>");

    // Get the fist page's resources dictionary
    QPDFObjectHandle resources = m_firstPage.getKey("/Resources");

    // Check if the resources dictionary already exists
    if (!resources.isDictionary()) {
        // Create a new resources dictionary if it doesn't exist
        resources = m_firstPage.newDictionary();
        m_firstPage.replaceKey("/Resources", resources);
    }

    // Get the fonts dictionary from the resources dictionary
    QPDFObjectHandle fonts = resources.getKey("/Font");

    // Check if the fonts dictionary already exists
    if (!fonts.isDictionary()) {
        // Create a new fonts dictionary if it doesn't exist
        fonts = m_pdf.makeIndirectObject(QPDFObjectHandle::newDictionary());
        resources.replaceKey("/Font", fonts);
    }

    std::string new_font_name = "/F";
    int font_name_increment = 1;
    bool found = false;
    std::string found_name;
    auto font_names = fonts.getKeys();

    // Look for the font
    for (auto font_name : font_names) {
        // Test if /F + i is the name of the font
        if ((new_font_name + std::to_string(font_name_increment)) ==
            font_name) {
            font_name_increment++;
        }

        QPDFObjectHandle f = fonts.getKeyIfDict(font_name);

        if (f.isNull())
            continue;

        QPDFObjectHandle basefont = f.getKey("/BaseFont");
        if (basefont.isNull())
            continue;

        if (basefont.isName() &&
            basefont.getName() == "/" + basefont_full_name) {
            found = true;
            found_name = font_name;
        }
    }

    if (!found) {
        // Add the new font to the fonts dictionary
        fonts.replaceKey(new_font_name + std::to_string(font_name_increment),
                         newFont);
    } else {
        new_font_name = found_name;
    }

    m_firstPage.addPageContents(
        QPDFObjectHandle::newStream(
            &m_pdf,
            "BT " + new_font_name + std::to_string(font_name_increment) + " " +
                std::to_string(font_size) + " Tf 1 0 0 1 " + std::to_string(x) +
                " " + std::to_string(y) + " Tm (" + text + ") Tj ET"),
        false);
    logger << "Adding text...\n";
}

void PDFProcessor::save(const std::string filename) {
    try {
        QPDFWriter w(m_pdf, filename.c_str());
        w.write();
    } catch (std::runtime_error &e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        std::cerr << "Cannot create output file '" << filename << "'"
                  << std::endl;
        exit(5);
    }
}
