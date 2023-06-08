#include "pdfProcessor.h"
#include "imageProvider.h"
#include "logger.h"

PDFProcessor::PDFProcessor(/* args */) {}

PDFProcessor::~PDFProcessor() {}

bool PDFProcessor::open(const std::string filename) {
    m_pdf.processFile(filename.c_str());
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

void PDFProcessor::setPosition(int side) { m_side = side; }

void PDFProcessor::addImage(const std::string filename) {
    // Image object
    QPDFObjectHandle image = QPDFObjectHandle::newStream(&m_pdf);
    ImageProvider *p = new ImageProvider(filename.c_str());
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
    m_xobject.replaceKey("/ImEPStamp55", image);

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
    m_xobject.replaceKey("/ImEPStamp55tr", transparency);
    std::string smaskString =
        std::string(std::to_string(transparency.getObjectID()) + " " +
                    std::to_string(transparency.getGeneration()) + " R");
    m_pdf.updateAllPagesCache();
    image.getDict().replaceKey("/SMask", transparency);

    // To prevent our image appearing in unexpected places we save the initial
    // state at the beginning of the page and restore it at the end before
    // adding our image. Thanks go to David van Driessche @StackOverflow for
    // this elegant solution
    m_firstPage.addPageContents(QPDFObjectHandle::newStream(&m_pdf, "q\n"),
                                true);
    std::string streamString;
    int topMargin = 0;
    int sideMargin = 20;
    int imgHeight, imgWidth;
    float pageHeight, pageWidth;
    pageHeight = m_pageRect.height();
    pageWidth = m_pageRect.width();
    imgHeight = p->getHeight();
    imgWidth = p->getWidth();
    streamString = std::string("Q q ");
    // If the page is rotated
    if (m_rotate != 0) {
        int cq, sq;
        int tx, ty;
        cq = cos((double)m_rotate * M_PI / 180);
        sq = sin((double)m_rotate * M_PI / 180);
        if (m_rotate == 90) {
            tx = m_pageRect.y();
            ty = -m_pageRect.x() - pageWidth;
        } else if (m_rotate == 180) {
            tx = -m_pageRect.x() - pageWidth;
            ty = -m_pageRect.y() - pageHeight;
        } else {
            tx = -m_pageRect.y() - pageHeight;
            ty = m_pageRect.x();
        }
        streamString += std::to_string(cq) + " " + std::to_string(sq) + " " +
                        std::to_string(-sq) + " " + std::to_string(cq) +
                        " 0 0 cm ";
        streamString += std::string("1 0 0 1 ") + std::to_string(tx) + " " +
                        std::to_string(ty) + " cm ";
    } else {
        streamString += std::string("1 0 0 1 ") +
                        std::to_string(m_pageRect.x()) + " " +
                        std::to_string(m_pageRect.y()) + " cm ";
    }

    // Set the appropriate image scaling according to page rotation
    streamString += std::to_string(0.57 * imgWidth) + " 0 0 " +
                    std::to_string(0.57 * imgHeight) + " ";

    int imgTranslateX, imgTranslateY;
    if (m_rotate == 90 || m_rotate == 270) {
        if (m_side == 0)
            imgTranslateX = (pageHeight - 0.57 * imgWidth) / 2;
        else if (m_side == 1)
            imgTranslateX =
                m_mediabox.getArrayItem(1).getNumericValue() + sideMargin;
        else
            imgTranslateX = pageHeight - 0.57 * imgWidth - sideMargin;
        imgTranslateY = pageWidth - imgHeight - topMargin;
    } else {
        if (m_side == 0)
            imgTranslateX = (pageWidth - 0.57 * imgWidth) / 2;
        else if (m_side == 1)
            imgTranslateX =
                m_mediabox.getArrayItem(0).getNumericValue() + sideMargin;
        else
            imgTranslateX = pageWidth - 0.57 * imgWidth - sideMargin;
        imgTranslateY = pageHeight - imgHeight - topMargin;
    }
    streamString +=
        std::to_string(imgTranslateX) + " " + std::to_string(imgTranslateY);
    streamString += " cm /ImEPStamp55 Do Q\n";
    logger << "Stream str: " << streamString << "\n";
    m_firstPage.addPageContents(
        QPDFObjectHandle::newStream(&m_pdf, streamString), false);
}

void PDFProcessor::save(const std::string filename) {
    QPDFWriter w(m_pdf, filename.c_str());
    w.write();
}
