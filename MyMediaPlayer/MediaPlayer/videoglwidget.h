#ifndef VIDEOGLWIDGET_H
#define VIDEOGLWFileRawDataPtrinclude <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <memory>
#include <mutex>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include "fileutil.h"
class VideoGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    QOBJECT_H
public:
    VideoGLWidget(QWidget * pParent = 0);
    virtual                 ~VideoGLWidget();
    void                    PictureShow(fileutil::PictureFilePtr pPicture);
    void                    DefaultPictureShow();

protected:
    void                    initializeGL();
    void                    resizeGL(int w, int h);
    void                    paintGL();

private:
    //绘图是异步动作， 防止绘图过程中，图片被置为NULL，所以需要加锁
    std::mutex                  m_mutexForShowYuvData;
    bool                        m_bIsShowVideoIcon; //是否要显示视频默认图标
    fileutil::PictureFilePtr    m_pDefaultPict;//默认图片
    fileutil::PictureFilePtr    m_pYuvPictPtr;//当前需要显示的Yuv数据

    //显示Opengl渲染后图像位置
    QRect                       m_drawRect;

    //渲染yuv图像变量
    GLuint                      textureUniformY; //y纹理数据位置
    GLuint                      textureUniformU; //u纹理数据位置
    GLuint                      textureUniformV; //v纹理数据位置
    GLuint                      id_y; //y纹理对象ID
    GLuint                      id_u; //u纹理对象ID
    GLuint                      id_v; //v纹理对象ID
    QOpenGLTexture*             m_pTextureY;  //y纹理对象
    QOpenGLTexture*             m_pTextureU;  //u纹理对象
    QOpenGLTexture*             m_pTextureV;  //v纹理对象
    QOpenGLShader *             m_pVSHader;  //顶点着色器程序对象
    QOpenGLShader *             m_pFSHader;  //片段着色器对象
    QOpenGLShaderProgram        *m_pShaderProgram; //着色器程序容器
};

#endif // VIDEOGLWIDGET_H
