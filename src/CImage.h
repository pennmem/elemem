#ifndef CIMAGE_H
#define CIMAGE_H

#include "Palette.h"
#include "RC/RC.h"
#include "RCqt/Worker.h"
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPainter>

using namespace std;
using namespace RC;


namespace CML {
  class Frame;

  class CImage : public QLabel, public RCqt::Worker {
    public:

    CImage();
    virtual ~CImage();

    protected:
    void ReDraw_Handler();
    void SetPalette_Handler(const PaletteType& type);

    public:
    RCqt::TaskCaller<> ReDraw = TaskHandler(CImage::ReDraw_Handler);
    RCqt::TaskCaller<const PaletteType> SetPalette =
      TaskHandler(CImage::SetPalette_Handler);

    virtual void CornerText(const RStr& str);

    QImage ToQImage();

    RCqt::TaskGetter<bool, FileWrite> SavePNG =
      TaskHandler(CImage::SavePNG_Handler);
    RCqt::TaskGetter<bool, FileWrite> SavePDF =
      TaskHandler(CImage::SavePDF_Handler);
    RCqt::TaskGetter<bool, FileWrite> SaveSVG =
      TaskHandler(CImage::SaveSVG_Handler);

    protected:


    void DrawAll();
    void DrawGeneral(RC::Ptr<QPaintDevice> device);

    virtual void DrawBackground();
    virtual void DrawOnTop();

    void SetPen(u32 color, f32 width=1);

    virtual void MouseLeftClicked(int /* x */, int /* y */) { }
    virtual void MouseLeftDragged(int /* x */, int /* y */) { }
    virtual void MouseRightClicked(int /* x */, int /* y */) { }
    virtual void MouseRightDragged(int /* x */, int /* y */) { }

    void ImagePos(QMouseEvent *event, int &x, int &y);

    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

    bool SavePNG_Handler(FileWrite& fwr);
    bool SavePDF_Handler(FileWrite& fwr);
    bool SaveSVG_Handler(FileWrite& fwr);

    bool SaveImage(FileWrite& fwr, const RStr& format);

    Palette palette;

    int width;
    int height;

    QPainter painter;

    QPixmap mypixmap;
  };
}

#endif // CIMAGE_H

