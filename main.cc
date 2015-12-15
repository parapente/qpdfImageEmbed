#include <iostream>
#include <qpdf/QPDF.hh>

int main(int argc, char *argv[])
{
    std::cout << "Hello world!" << std::endl;
    QPDF p;

    p.processFile(argv[1]);
    std::cout << p.getPDFVersion() << std::endl;

    std::vector<QPDFObjectHandle> pages;
    pages = p.getAllPages();
    std::cout << "Pages:" << pages.size();

    return 0;
}
