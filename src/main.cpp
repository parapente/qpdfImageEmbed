#include <QtCore/QDebug>
#include <cmath>
#include <qpdf/QPDF.hh>
#include <qrencode.h>

#include "config.h"
#include "imageProvider.h"
#include "options.h"
#include "rect.h"

using namespace std;

int main(int argc, char *argv[]) {
  QPDF pdf;
  int rotate = 0;
  Rect pageRect;
  QHash<QString, QVariant> cliOption;

  cliOption = readCLIOptions(argc, argv);

  qDebug() << cliOption["side"].toString();
  pdf.processFile(cliOption["inputPDF"].toString().toLocal8Bit());
  qDebug() << QString::fromStdString(pdf.getPDFVersion());

  vector<QPDFObjectHandle> pages;
  pages = pdf.getAllPages();
  qDebug() << "Pages:" << pages.size();

  QPDFObjectHandle firstPage = pages.at(0);

  qDebug() << "Has Contents:" << firstPage.hasKey("/Contents");
  qDebug() << "Has MediaBox:" << firstPage.hasKey("/MediaBox");
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
  qDebug() << "--> MediaBox : ";
  for (int i = 0; i < mediabox.getArrayNItems(); i++) {
    qDebug() << mediabox.getArrayItem(i).getNumericValue() << " ";
  }
  qDebug() << "Has CropBox:" << firstPage.hasKey("/CropBox");
  if (firstPage.hasKey("/CropBox")) {
    QPDFObjectHandle cropbox = firstPage.getKey("/CropBox");
    qDebug() << "--> CropBox : ";
    for (int i = 0; i < cropbox.getArrayNItems(); i++) {
      qDebug() << cropbox.getArrayItem(i).getNumericValue() << " ";
    }
    pageRect.setCoords(cropbox.getArrayItem(0).getNumericValue(),
                       cropbox.getArrayItem(1).getNumericValue(),
                       cropbox.getArrayItem(2).getNumericValue(),
                       cropbox.getArrayItem(3).getNumericValue());
  }
  qDebug() << "Has Rotate:" << firstPage.hasKey("/Rotate");
  if (firstPage.hasKey("/Rotate")) {
    QPDFObjectHandle rotateObj = firstPage.getKey("/Rotate");
    rotate = rotateObj.getNumericValue();
    qDebug() << "--> Rotate :" << rotate;
  }
  // Override rotation
  if (cliOption.contains("rotate")) {
    rotate = cliOption["rotate"].toInt();
  }
  qDebug() << "Has Resources:" << firstPage.hasKey("/Resources");

  QPDFObjectHandle resources = firstPage.getKey("/Resources");
  qDebug() << "Has Resources->XObject:" << resources.hasKey("/XObject");

  QPDFObjectHandle xobject;
  if (!resources.hasKey("/XObject")) {
    xobject = QPDFObjectHandle::newDictionary();
    resources.replaceKey("/XObject", xobject);
  } else
    xobject = resources.getKey("/XObject");

  // Image object
  QPDFObjectHandle image = QPDFObjectHandle::newStream(&pdf);
  ImageProvider *p =
      new ImageProvider(cliOption["imageFile"].toString().toLocal8Bit());
  QString imageString = QString("<<"
                                " /Type /XObject"
                                " /Subtype /Image"
                                " /ColorSpace /DeviceRGB"
                                " /BitsPerComponent 8"
                                " /Width ") +
                        QString::number(p->getWidth()) + " /Height " +
                        QString::number(p->getHeight()) + ">>";
  qDebug() << imageString.toLocal8Bit().constData();
  image.replaceDict(
      QPDFObjectHandle::parse(imageString.toLocal8Bit().constData()));
  // Provide the stream data.
  PointerHolder<QPDFObjectHandle::StreamDataProvider> provider(p);
  image.replaceStreamData(provider, QPDFObjectHandle::newNull(),
                          QPDFObjectHandle::newNull());
  xobject.replaceKey("/ImEPStamp55", image);

  // Transparency object
  QPDFObjectHandle transparency = QPDFObjectHandle::newStream(&pdf);
  QString transparencyImageString = QString("<<"
                                            " /Type /XObject"
                                            " /Subtype /Image"
                                            " /ColorSpace /DeviceGray"
                                            " /BitsPerComponent 8"
                                            " /Decode [0.0 1.0]"
                                            " /Width ") +
                                    QString::number(p->getWidth()) +
                                    " /Height " +
                                    QString::number(p->getHeight()) + ">>";
  qDebug() << transparencyImageString.toLocal8Bit().constData();
  transparency.replaceDict(QPDFObjectHandle::parse(
      transparencyImageString.toLocal8Bit().constData()));
  // Provide the stream data.
  PointerHolder<Buffer> transparencyProvider(p->getAlpha());
  qDebug() << "Buffer size: " << transparencyProvider.getPointer()->getSize();
  transparency.replaceStreamData(transparencyProvider,
                                 QPDFObjectHandle::newNull(),
                                 QPDFObjectHandle::newNull());
  xobject.replaceKey("/ImEPStamp55tr", transparency);
  QString smaskString =
      QString(QString::number(transparency.getObjectID()) + " " +
              QString::number(transparency.getGeneration()) + " R");
  pdf.updateAllPagesCache();
  image.getDict().replaceKey("/SMask", transparency);

  // To prevent our image appearing in unexpected places we save the initial
  // state at the beginning of the page and restore it at the end before adding
  // our image. Thanks go to David van Driessche @StackOverflow for this elegant
  // solution
  firstPage.addPageContents(QPDFObjectHandle::newStream(&pdf, "q\n"), true);
  QString streamString;
  int topMargin = 0;
  int sideMargin = 20;
  int imgHeight, imgWidth;
  float pageHeight, pageWidth;
  pageHeight = pageRect.height();
  pageWidth = pageRect.width();
  imgHeight = p->getHeight();
  imgWidth = p->getWidth();
  streamString = QString("Q q ");
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
    streamString += QString::number(cq) + " " + QString::number(sq) + " " +
                    QString::number(-sq) + " " + QString::number(cq) +
                    " 0 0 cm ";
    streamString += QString("1 0 0 1 ") + QString::number(tx) + " " +
                    QString::number(ty) + " cm ";
  } else {
    streamString += QString("1 0 0 1 ") + QString::number(pageRect.x()) + " " +
                    QString::number(pageRect.y()) + " cm ";
  }

  // Set the appropriate image scaling according to page rotation
  streamString += QString::number(0.57 * imgWidth) + " 0 0 " +
                  QString::number(0.57 * imgHeight) + " ";

  int imgtx, imgty;
  if (rotate == 90 || rotate == 270) {
    if (cliOption["side"] == 0)
      imgtx = (pageHeight - 0.57 * imgWidth) / 2;
    else if (cliOption["side"] == 1)
      imgtx = mediabox.getArrayItem(1).getNumericValue() + sideMargin;
    else
      imgtx = pageHeight - 0.57 * imgWidth - sideMargin;
    imgty = pageWidth - imgHeight - topMargin;
  } else {
    if (cliOption["side"] == 0)
      imgtx = (pageWidth - 0.57 * imgWidth) / 2;
    else if (cliOption["side"] == 1)
      imgtx = mediabox.getArrayItem(0).getNumericValue() + sideMargin;
    else
      imgtx = pageWidth - 0.57 * imgWidth - sideMargin;
    imgty = pageHeight - imgHeight - topMargin;
  }
  streamString += QString::number(imgtx) + " " + QString::number(imgty);
  streamString += " cm /ImEPStamp55 Do Q\n";
  qDebug() << "Stream str: " << streamString;
  firstPage.addPageContents(
      QPDFObjectHandle::newStream(&pdf, streamString.toStdString()), false);

  QPDFWriter w(pdf, cliOption["outputPDF"].toString().toLocal8Bit());
  w.write();

  return 0;
}
