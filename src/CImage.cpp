#include "CImage.h"
#include "Palette.h"
#include "Popup.h"
#include "QtFileFunctions.h"
#include "RC/RC.h"
#include <math.h>
#include <QMouseEvent>
#include <QPrinter>
#include <QSvgGenerator>


using namespace RC;


namespace CML {
  CImage::CImage()
    : width (1)
    , height (1) {

    setContentsMargins(0,0,0,0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DrawAll();
  }


  CImage::~CImage() {
  }


  void CImage::DrawAll() {
    width = QLabel::width();
    height = QLabel::height();

    mypixmap = QPixmap(width, height);

    DrawGeneral(&mypixmap);

    setPixmap(mypixmap);
  }


  void CImage::DrawGeneral(Ptr<QPaintDevice> device) {
    painter.begin(device.Raw());

    painter.fillRect(0, 0, width, height,
                     QColor::fromRgba(palette.GetBG_ARGB()));

    painter.setRenderHints(QPainter::Antialiasing |
                           QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform);

    DrawBackground();
    DrawOnTop();

    painter.end();
  }


  void CImage::ReDraw_Handler() {
    DrawAll();
  }


  void CImage::SetPalette_Handler(const PaletteType& type) {
    palette.SetPalette(type);
  }


  void CImage::DrawBackground() {
  }


  void CImage::DrawOnTop() {
  }


  void CImage::SetPen(u32 color, f32 width) {
    QColor qcolor = QColor::fromRgba(color);
    QPen qpen(qcolor);
    qpen.setWidthF(qreal(width));
    painter.setPen(qpen);
  }


  void CImage::ImagePos(QMouseEvent *event, int &x, int &y) {
    x = event->x();// - (QLabel::width()-width)/2;  // aligned left
    y = event->y() - (QLabel::height()-height)/2;  // center aligned
  }


  void CImage::mousePressEvent(QMouseEvent * event) {
    int x, y;
    ImagePos(event, x, y);
    switch(event->button()) {
      case Qt::LeftButton:  MouseLeftClicked(x, y);
                            break;
      case Qt::RightButton:  MouseRightClicked(x, y);
                             break;
      default:  break;
    }
    QLabel::mousePressEvent(event);
  }


  void CImage::mouseMoveEvent(QMouseEvent * event) {
    // Note:  buttons is bitwise or, so it passes if these are the only
    // conditions active at the time.
    int x, y;
    ImagePos(event, x, y);
    switch(event->buttons()) {
      case Qt::LeftButton:  MouseLeftDragged(x, y);
                            break;
      case Qt::RightButton:  MouseRightDragged(x, y);
                             break;
      default:  break;
    }
    QLabel::mouseMoveEvent(event);
  }


  void CImage::CornerText(const RStr& str) {
    QFont font = painter.font();
    font.setPixelSize(17);
    painter.setFont(font);
    SetPen(palette.GetFG_ARGB(0.9f));
    painter.drawText(0, 2, width-5, height-5, Qt::AlignTop | Qt::AlignRight, str.ToQString());
  }


  bool CImage::SavePNG_Handler(FileWrite& fwr) {
    return SaveImage(fwr, "PNG");
  }


  // Note:  Closes the FileWrite before saving to the filename.
  bool CImage::SavePDF_Handler(FileWrite& fwr) {
    // TODO: JPB: (feature) Add this back in
    //RStr filename = fwr.GetFilename();
    //fwr.Close();
    //
    //QPrinter printer;
    //printer.setOutputFormat(QPrinter::PdfFormat);
    //printer.setOutputFileName(filename.ToQString());
    //printer.setPaperSize(QSizeF(width, height), QPrinter::Point);
    //printer.setResolution(72);
    //printer.setFullPage(true);

    //DrawGeneral(&printer);

    return true;
  }


  // Note:  Closes the FileWrite before saving to the filename.
  bool CImage::SaveSVG_Handler(FileWrite& fwr) {
    RStr filename = fwr.GetFilename();
    fwr.Close();
    
    QSvgGenerator generator;
    generator.setFileName(filename.ToQString());
    generator.setResolution(90);
    generator.setSize(QSize(width, height));
    generator.setViewBox(QRect(0, 0, width, height));

    DrawGeneral(&generator);

    return true;
  }


  bool CImage::SaveImage(FileWrite& fwr, const RStr& format) {
    try {
      MyQIODevice myqio(fwr);
      mypixmap.save(&myqio, format.c_str());
    }
    catch (ErrorMsg &e) {
      ErrorWin(fwr.GetFilename() + " - " + e.GetError());
      return false;
    }
    return true;
  }
}

