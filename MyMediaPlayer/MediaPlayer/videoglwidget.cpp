#include "videoglwidget.h"
#include <QPainter>
#include <QDebug>
#include <ctime>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include "logutil.h"
//opengl ��Ⱦ���ֲο���Csdn����:https://blog.csdn.net/su_vast/article/details/52214642
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
    //Ĭ����ʾ��Ⱦλ��ΪĬ����Ƶͼ��λ��
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

void VideoGLWidget::PictureShow(PictureFilePtr pPicture)
{
    std::unique_lock<std::mutex> locker(m_mutexForShowYuvData);
    m_pYuvPictPtr = pPicture;
    m_drawRect = QRect(0, 0, width(), height());
    this->update();
}

void VideoGLWidget::DefaultPictureShow()
{
    std::unique_lock<std::mutex> locker(m_mutexForShowYuvData);
    if(!m_pDefaultPict)
    {
        FileRawDataPtr pFileData = fileutil::ReadFileRawData("../../images/video.yuv");
        m_pDefaultPict = PictureFilePtr(new PictureFile(*pFileData.get(), DEFAULT_ICO_PIX_WIDTH, DEFAULT_ICO_PIX_HEIGHT, PictureFile::kFormatYuv));
    }
    m_pYuvPictPtr = m_pDefaultPict;
    m_drawRect = QRect((width() - DEFAULT_ICO_VIEW_WIDTH)/2, (height() - DEFAULT_ICO_VIEW_HEIGHT) /2, DEFAULT_ICO_VIEW_WIDTH, DEFAULT_ICO_VIEW_HEIGHT);
    this->update();
}

void VideoGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    //�ִ�opengl��Ⱦ����������ɫ���������������
    //��ɫ��������ʹ��openGL��ɫ����(OpenGL Shading Language, GLSL)��д��һ��С����,
    //GLSL�ǹ�������OpenGL��ɫ��������,�����GLSL���Ե��﷨��Ҫ���߲����������
    //��ʼ��������ɫ�� ����
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
    //��ʼ��Ƭ����ɫ�� ����gpu��yuvת����rgb
    m_pFSHader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    //Ƭ����ɫ��Դ��
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
    //������ɫ����������
    m_pShaderProgram = new QOpenGLShaderProgram;
    //��Ƭ����ɫ����ӵ���������
    m_pShaderProgram->addShader(m_pFSHader);
    //��������ɫ����ӵ���������
    m_pShaderProgram->addShader(m_pVSHader);
    //������vertexIn��ָ��λ��ATTRIB_VERTEX,�������ڶ�����ɫԴ������������
    m_pShaderProgram->bindAttributeLocation("vertexIn", ATTRIB_VERTEX);
    //������textureIn��ָ��λ��ATTRIB_TEXTURE,�������ڶ�����ɫԴ������������
    m_pShaderProgram->bindAttributeLocation("textureIn", ATTRIB_TEXTURE);
    //���������������뵽����ɫ������
    m_pShaderProgram->link();
    //������������
    m_pShaderProgram->bind();
    //��ȡ��ɫ���е����ݱ���tex_y, tex_u, tex_v��λ��,��Щ����������������
    //Ƭ����ɫ��Դ���п��Կ���
    textureUniformY = m_pShaderProgram->uniformLocation("tex_y");
    textureUniformU =  m_pShaderProgram->uniformLocation("tex_u");
    textureUniformV =  m_pShaderProgram->uniformLocation("tex_v");
    // �������
    static const GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f,
    };
    //�������
    static const GLfloat textureVertices[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };
    //��������ATTRIB_VERTEX�Ķ������ֵ�Լ���ʽ
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    //��������ATTRIB_TEXTURE���������ֵ�Լ���ʽ
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    //����ATTRIB_VERTEX���Ե�����,Ĭ���ǹرյ�
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    //����ATTRIB_TEXTURE���Ե�����,Ĭ���ǹرյ�
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    //�ֱ𴴽�y,u,v�������
    m_pTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureY->create();
    m_pTextureU->create();
    m_pTextureV->create();
    //��ȡ����y��������������ֵ
    id_y = m_pTextureY->textureId();
    //��ȡ����u��������������ֵ
    id_u = m_pTextureU->textureId();
    //��ȡ����v��������������ֵ
    id_v = m_pTextureV->textureId();
    //    glClearColor(0.3,0.3,0.3,0.0);//���ñ���ɫ
}

void VideoGLWidget::resizeGL(int w, int h)
{
    if(h == 0)// ��ֹ�����
    {
        h = 1;// ������Ϊ1
    }
    glViewport(0, 0, w, h);
}

void VideoGLWidget::paintGL()
{
    std::unique_lock<std::mutex> locker(m_mutexForShowYuvData);
    if(m_pYuvPictPtr)
    {
        //        clock_t start = clock();
        uint32_t weight = m_pYuvPictPtr->m_nWeight;
        uint32_t height = m_pYuvPictPtr->m_nHeight;
        glViewport(m_drawRect.x(), m_drawRect.y(), m_drawRect.width(), m_drawRect.height());
        //����y��������
        //��������ԪGL_TEXTURE0
        glActiveTexture(GL_TEXTURE0);
        //ʹ������y������������
        glBindTexture(GL_TEXTURE_2D, id_y);
        //ʹ���ڴ���m_pBufYuv420p���ݴ���������y��������
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, weight, height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pYuvPictPtr->m_pData);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //����u��������
        glActiveTexture(GL_TEXTURE1);//��������ԪGL_TEXTURE1
        glBindTexture(GL_TEXTURE_2D, id_u);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, weight/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pYuvPictPtr->m_pData+weight*height);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //����v��������
        glActiveTexture(GL_TEXTURE2);//��������ԪGL_TEXTURE2
        glBindTexture(GL_TEXTURE_2D, id_v);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, weight/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)m_pYuvPictPtr->m_pData+weight*height*5/4);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //ָ��y����Ҫʹ����ֵ ֻ����0,1,2�ȱ�ʾ����Ԫ������������opengl�����Ի��ĵط�
        //0��Ӧ����ԪGL_TEXTURE0 1��Ӧ����ԪGL_TEXTURE1 2��Ӧ����ĵ�Ԫ
        glUniform1i(textureUniformY, 0);
        //ָ��u����Ҫʹ����ֵ
        glUniform1i(textureUniformU, 1);
        //ָ��v����Ҫʹ����ֵ
        glUniform1i(textureUniformV, 2);
        //ʹ�ö������鷽ʽ����ͼ��
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //        clock_t end = clock();
    }
}
