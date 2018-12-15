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
    //��ͼ���첽������ ��ֹ��ͼ�����У�ͼƬ����ΪNULL��������Ҫ����
    std::mutex                  m_mutexForShowYuvData;
    bool                        m_bIsShowVideoIcon; //�Ƿ�Ҫ��ʾ��ƵĬ��ͼ��
    fileutil::PictureFilePtr    m_pDefaultPict;//Ĭ��ͼƬ
    fileutil::PictureFilePtr    m_pYuvPictPtr;//��ǰ��Ҫ��ʾ��Yuv����

    //��ʾOpengl��Ⱦ��ͼ��λ��
    QRect                       m_drawRect;

    //��Ⱦyuvͼ�����
    GLuint                      textureUniformY; //y��������λ��
    GLuint                      textureUniformU; //u��������λ��
    GLuint                      textureUniformV; //v��������λ��
    GLuint                      id_y; //y�������ID
    GLuint                      id_u; //u�������ID
    GLuint                      id_v; //v�������ID
    QOpenGLTexture*             m_pTextureY;  //y�������
    QOpenGLTexture*             m_pTextureU;  //u�������
    QOpenGLTexture*             m_pTextureV;  //v�������
    QOpenGLShader *             m_pVSHader;  //������ɫ���������
    QOpenGLShader *             m_pFSHader;  //Ƭ����ɫ������
    QOpenGLShaderProgram        *m_pShaderProgram; //��ɫ����������
};

#endif // VIDEOGLWIDGET_H
