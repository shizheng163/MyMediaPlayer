#ifndef VIDEOGLWIDGET_H
#define VIDEOGLWFileRawDataPtrinclude <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <memory>
#include <mutex>
#include "fileutil.h"
class VideoGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    QOBJECT_H
public:
    typedef fileutil::FileRawDataPtr PicturePtr;
public:
    VideoGLWidget(QWidget * pParent = 0);
    virtual                 ~VideoGLWidget();
    void                    PictureShow(PicturePtr pPicture);
    void                    DefaultPictureShow();

protected:
    void                    initializeGL();
    void                    resizeGL(int w, int h);
    void                    paintGL();

private:
    //��ͼ���첽������ ��ֹ��ͼ�����У�ͼƬ����ΪNULL��������Ҫ����
    std::mutex              m_mutexForShowImage;
    QImage                  *m_pCurrentShowImage;
    QPoint                  m_pointShow;
};

#endif // VIDEOGLWIDGET_H
