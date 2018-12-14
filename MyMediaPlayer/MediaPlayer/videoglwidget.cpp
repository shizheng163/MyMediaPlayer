#include "videoglwidget.h"
#include <QPainter>
#include <QDebug>
#include <ctime>

using namespace fileutil;
VideoGLWidget::VideoGLWidget(QWidget *pParent)
    :QOpenGLWidget(pParent)
    ,m_pCurrentShowImage(NULL)
{
    this->DefaultPictureShow();
}

VideoGLWidget::~VideoGLWidget()
{
    std::unique_lock<std::mutex> locker(m_mutexForShowImage);
    if(m_pCurrentShowImage)
    {
        delete m_pCurrentShowImage;
        m_pCurrentShowImage = NULL;
    }
}

void VideoGLWidget::PictureShow(PicturePtr pPicture)
{
    std::unique_lock<std::mutex> locker(m_mutexForShowImage);
    if(m_pCurrentShowImage)
    {
        delete m_pCurrentShowImage;
        m_pCurrentShowImage = NULL;
    }
    QImage img;
    if(!img.loadFromData(pPicture->m_pData, pPicture->m_uLen, "jpeg"))
    {
        qDebug() << "load picture error:" << pPicture->m_filename.c_str();
        return;
    }
    m_pCurrentShowImage = new QImage(img.scaled(this->size()));
    m_pointShow.setX(0);
    m_pointShow.setY(0);
    this->update();
}

void VideoGLWidget::DefaultPictureShow()
{
    std::unique_lock<std::mutex> locker(m_mutexForShowImage);
    if(m_pCurrentShowImage)
        delete m_pCurrentShowImage;
    m_pCurrentShowImage = new QImage;
    if(m_pCurrentShowImage->load(QString::fromUtf8("./images/video.ico")))
    {
        m_pointShow.setX((this->width() - m_pCurrentShowImage->width()) / 2);
        m_pointShow.setY(this->height() / 2);
        this->update();
    }
    else
    {
        delete m_pCurrentShowImage;
        m_pCurrentShowImage = NULL;
    }
}

void VideoGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
}

void VideoGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void VideoGLWidget::paintGL()
{
    clock_t start = clock();
    std::unique_lock<std::mutex> locker(m_mutexForShowImage);
    if(m_pCurrentShowImage)
    {
        QPainter painter;
        painter.begin(this);
        painter.drawImage(m_pointShow, *m_pCurrentShowImage);
        painter.end();
    }
    clock_t end = clock();
    qDebug() << "drawImage time = " << (end - start);
}
