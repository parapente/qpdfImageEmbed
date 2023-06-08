#ifndef PDFPROCESSOR_H
#define PDFPROCESSOR_H

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

    public:
        PDFProcessor(/* args */);
        ~PDFProcessor();

        bool open(const std::string filename);
        void rotate(int degrees);
        void setPosition(int side);
        void addImage(const std::string filename);
        void save(const std::string filename);
};

#endif // PDFPROCESSOR_H