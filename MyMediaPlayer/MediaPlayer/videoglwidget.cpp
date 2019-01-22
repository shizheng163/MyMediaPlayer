/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#include "videoglwidget.h"
#include <QPainter>
#include <QDebug>
#include <ctime>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include "logutil.h"
//opengl 渲染部分参考自Csdn博客:https://blog.csdn.net/su_vast/article/details/52214642
using namespace fileutil;
#define DEFAULT_ICO_VIEW_WIDTH 48
#define DEFAULT_ICO_VIEW_HEIGHT 48
#define DEFAULT_ICO_PIX_HEIGHT 512
#define DEFAULT_ICO_PIX_WIDTH 512
VideoGLWidget::VideoGLWidget(QWidget *pParent)
    :QOpenGLWidget(pParent)
{
    textureUniformY = 0;
    textureUniformU = 0;
    textureUniformV = 0;
    id_y = 0;
    id_u = 0;
    id_v = 0;
    m_pYuvPictPtr = NULL;
    m_pVSHader = NULL;
    m_pFSHader = NULL;
    m_pShaderProgram = NULL;
    m_pTextureY = NULL;
    m_pTextureU = NULL;
    m_pTextureV = NULL;
    //默认显示渲染位置为默认视频图标位置
    m_drawRect = QRect((width() - DEFAULT_ICO_VIEW_WIDTH)/2, height()/2, DEFAULT_ICO_VIEW_WIDTH, DEFAULT_ICO_VIEW_HEIGHT);
    this->DefaultPictureShow();
}

VideoGLWidget::~VideoGLWidget()
{
    if(m_pVSHader)
    {
        delete m_pVSHader;
        m_pVSHader = NULL;
    }
    if(m_pFSHader)
    {
        delete m_pFSHader;
        m_pFSHader = NULL;
    }
    if(m_pShaderProgram)
    {
        delete m_pShaderProgram;
        m_pShaderProgram = NULL;
    }
    if(m_pTextureY)
    {
        delete m_pTextureY;
        m_pTextureY = NULL;
    }
    if(m_pTextureU)
    {
        delete m_pTextureU;
        m_pTextureU = NULL;
    }
    if(m_pTextureV)
    {
        delete m_pTextureV;
        m_pTextureV = NULL;
    }
}

void VideoGLWidget::PictureShow(fileutil::RawDataPtr pPicture, int frameWidth, int frameHeight)
{
    std::unique_lock<std::mutex> locker(m_mutexForShowYuvData);
    m_pYuvPictPtr = pPicture;
    m_nPictureHeight = frameHeight;
    m_nPictureWidth = frameWidth;
    m_drawRect = QRect(0, 0, width(), height());
    this->update();
}

void VideoGLWidget::DefaultPictureShow()
{
    std::unique_lock<std::mutex> locker(m_mutexForShowYuvData);
    if(!m_pDefaultPict)
        m_pDefaultPict = fileutil::ReadFileRawData("../../images/video.yuv");
    m_pYuvPictPtr = m_pDefaultPict;
    m_nPictureHeight = DEFAULT_ICO_PIX_WIDTH;
    m_nPictureWidth = DEFAULT_ICO_PIX_HEIGHT;
    m_drawRect = QRect((width() - DEFAULT_ICO_VIEW_WIDTH)/2, (height() - DEFAULT_ICO_VIEW_HEIGHT) /2, DEFAULT_ICO_VIEW_WIDTH, DEFAULT_ICO_VIEW_HEIGHT);
    this->update();
}

void VideoGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    //现代opengl渲染管线依赖着色器来处理传入的数据
    //着色器：就是使用openGL着色语言(OpenGL Shading Language, GLSL)编写的一个小函数,
    //GLSL是构成所有OpenGL着色器的语言,具体的GLSL语言的语法需要读者查找相关资料
    //初始化顶点着色器 对象
    m_pVSHader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc ="\
            attribute vec4 vertexIn; \
    attribute vec2 textureIn; \
    varying vec2 textureOut;  \
    void main(void)           \
    {                         \
        gl_Position = vertexIn; \
        textureOut = textureIn; \
    }";
    m_pVSHader->compileSourceCode(vsrc);
    //初始化片段着色器 功能gpu中yuv转换成rgb
    m_pFSHader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    //片段着色器源码
    const char *fsrc = "varying vec2 textureOut; \
            uniform sampler2D tex_y; \
    uniform sampler2D tex_u; \
    uniform sampler2D tex_v; \
    void main(void) \
    { \
        vec3 yuv; \
        vec3 rgb; \
        yuv.x = texture2D(tex_y, textureOut).r; \
        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
        rgb = mat3( 1,       1,         1, \
                    0,       -0.39465,  2.03211, \
                    1.13983, -0.58060,  0) * yuv; \
        gl_FragColor = vec4(rgb, 1); \
    }";
    m_pFSHader->compileSourceCode(fsrc);
#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1
#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4
    //创建着色器程序容器
    m_pShaderProgram = new QOpenGLShaderProgram;
    //将片段着色器添加到程序容器
    m_pShaderProgram->addShader(m_pFSHader);
    //将顶点着色器添加到程序容器
    m_pShaderProgram->addShader(m_pVSHader);
    //绑定属性vertexIn到指定位置ATTRIB_VERTEX,该属性在顶点着色源码其中有声明
    m_pShaderProgram->bindAttributeLocation("vertexIn", ATTRIB_VERTEX);
    //绑定属性textureIn到指定位置ATTRIB_TEXTURE,该属性在顶点着色源码其中有声明
    m_pShaderProgram->bindAttributeLocation("textureIn", ATTRIB_TEXTURE);
    //链接所有所有添入到的着色器程序
    m_pShaderProgram->link();
    //激活所有链接
    m_pShaderProgram->bind();
    //读取着色器中的数据变量tex_y, tex_u, tex_v的位置,这些变量的声明可以在
    //片段着色器源码中可以看到
    textureUniformY = m_pShaderProgram->uniformLocation("tex_y");
    textureUniformU =  m_pShaderProgram->uniformLocation("tex_u");
    textureUniformV =  m_pShaderProgram->uniformLocation("tex_v");
    // 顶点矩阵
    static const GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f,
    };
    //纹理矩阵
    static const GLfloat textureVertices[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };
    //设置属性ATTRIB_VERTEX的顶点矩阵值以及格式
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    //设置属性ATTRIB_TEXTURE的纹理矩阵值以及格式
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    //启用ATTRIB_VERTEX属性的数据,默认是关闭的
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    //启用ATTRIB_TEXTURE属性的数据,默认是关闭的
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    //分别创建y,u,v纹理对象
    m_pTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureY->create();
    m_pTextureU->create();
    m_pTextureV->create();
    //获取返回y分量的纹理索引值
    id_y = m_pTextureY->textureId();
    //获取返回u分量的纹理索引值
    id_u = m_pTextureU->textureId();
    //获取返回v分量的纹理索引值
    id_v = m_pTextureV->textureId();
    //    glClearColor(0.3,0.3,0.3,0.0);//设置背景色
}

void VideoGLWidget::resizeGL(int w, int h)
{
    if(h == 0)// 防止被零除
    {
        h = 1;// 将高设为1
    }
    glViewport(0, 0, w, h);
}

void VideoGLWidget::paintGL()
{
    std::unique_lock<std::mutex> locker(m_mutexForShowYuvData);
    if(m_pYuvPictPtr)
    {
        glViewport(m_drawRect.x(), m_drawRect.y(), m_drawRect.width(), m_drawRect.height());
        //加载y数据纹理
        //激活纹理单元GL_TEXTURE0
        glActiveTexture(GL_TEXTURE0);
        //使用来自y数据生成纹理
        glBindTexture(GL_TEXTURE_2D, id_y);
        //使用内存中m_pBufYuv420p数据创建真正的y数据纹理
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nPictureWidth, m_nPictureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, m_pYuvPictPtr->m_pData);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //加载u数据纹理
        glActiveTexture(GL_TEXTURE1);//激活纹理单元GL_TEXTURE1
        glBindTexture(GL_TEXTURE_2D, id_u);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nPictureWidth/2, m_nPictureHeight/2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pYuvPictPtr->m_pData+m_nPictureWidth*m_nPictureHeight);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //加载v数据纹理
        glActiveTexture(GL_TEXTURE2);//激活纹理单元GL_TEXTURE2
        glBindTexture(GL_TEXTURE_2D, id_v);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nPictureWidth/2, m_nPictureHeight/2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pYuvPictPtr->m_pData+m_nPictureWidth*m_nPictureHeight*5/4);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //指定y纹理要使用新值 只能用0,1,2等表示纹理单元的索引，这是opengl不人性化的地方
        //0对应纹理单元GL_TEXTURE0 1对应纹理单元GL_TEXTURE1 2对应纹理的单元
        glUniform1i(textureUniformY, 0);
        //指定u纹理要使用新值
        glUniform1i(textureUniformU, 1);
        //指定v纹理要使用新值
        glUniform1i(textureUniformV, 2);
        /*
         * OpenGL要求所有的纹理都是4字节对齐的，即纹理的大小永远是4字节的倍数。
         * 通常这并不会出现什么问题，因为大部分纹理的宽度都为4的倍数并/或每像素使用4个字节。
         * 当图片宽高不是4的倍数时会出现的情况。通过将纹理解压对齐参数设为1，这样才能确保不会有对齐问题。
         */
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        //使用顶点数组方式绘制图形
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
