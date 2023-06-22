#ifndef PDFPROCESSOR_H
#define PDFPROCESSOR_H

#include "imageProvider.h"
#include "rect.h"
#include <qpdf/QPDF.hh>
#include <string>

class PDFProcessor {
    private:
        QPDF m_pdf;
        Rect m_pageRect;
        int m_rotate = 0;
        int m_side = 0;
        QPDFObjectHandle m_xobject;
        QPDFObjectHandle m_firstPage;
        QPDFObjectHandle m_mediabox;

        std::string rand_str(int length);
        std::string createNewImageName(std::string prefix);
        void createImageStream(ImageProvider *p, std::string name);

    public:
        PDFProcessor(/* args */);
        ~PDFProcessor();

        bool open(const std::string filename);
        void rotate(int degrees);
        void setPosition(int side);
        void addImage(ImageProvider *p, float scale, float topMargin,
                      float sideMargin, std::string link = "");
        void addExtraText(std::string text, float x, float y, float font_size,
                          std::string basefont, std::string style);
        void save(const std::string filename);
};

#endif // PDFPROCESSOR_H