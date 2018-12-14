#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <string>
#include <io.h>
#include <iostream>
#include <thread>
#include <windows.h>
#include <QDebug>
#include <queue>
#include <condition_variable>
#include <mutex>
#include "logutil.h"
using namespace std;
MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,m_pVideoGLWidget(NULL)
{
    //布局
    ui->setupUi(this);
    this->setFixedSize(this->size());
    this->setStyleSheet("QMainWindow{background-color:#C9C9C9;}");

    unsigned mainWidth(this->width()), mainHeight(this->height());
    m_pVideoGLWidget = new VideoGLWidget(this);
    m_pVideoGLWidget->move(0, 0);
    m_pVideoGLWidget->resize(mainWidth, mainHeight * 0.85);
    m_pVideoGLWidget->setStyleSheet("background-color:#000000");

    ui->m_pLabProcessBar->move(0, this->height() * 0.85);
    ui->m_pLabProcessBar->resize(m_pVideoGLWidget->width(), this->height()*0.05);
    ui->m_pLabProcessBar->setStyleSheet("background-color:#66CDAA");

    //设置按钮透明
    ui->m_pBtnOpenFile->setFlat(true);
    ui->m_pBtnPlayOrPause->setFlat(true);
    ui->m_pBtnStop->setFlat(true);
    ui->m_pBtnSpeedSlow->setFlat(true);
    ui->m_pBtnSpeedUp->setFlat(true);

    this->ResetControls();

    connect(ui->m_pBtnOpenFile, &QPushButton::clicked, this, &SelectDir);
    connect(this, &MainWindow::SignalBtnEnable, this, &MainWindow::SlotBtnEnable);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pVideoGLWidget;
}

void MainWindow::SlotBtnEnable(bool enable)
{
    ui->m_pBtnOpenFile->setEnabled(enable);
}

void MainWindow::ResetControls()
{
    emit SignalBtnEnable(true);
    ui->m_pLabProcessBar->setText(tr("当前无文件播放"));
    m_pVideoGLWidget->DefaultPictureShow();
}

void MainWindow::SelectDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "H://");
    if(dir.isEmpty())
    {
        this->ResetControls();
    }
    else
    {
        ShowPictures(dir);
    }
}

void MainWindow::ShowPictures(QString dir)
{
    std::vector<std::string> pictures = FindPicturesFromDir(dir.toStdString());
    if(pictures.empty())
    {
        ui->m_pLabProcessBar->setText("未找到可显示的图片");
        return;
    }
    std::thread(&MainWindow::ShowPicturesInThread, this, pictures).detach();
}

void MainWindow::ShowPicturesInThread(std::vector<string> pictureVector)
{
    std::mutex mutexForMemoryPictures;
    std::queue<VideoGLWidget::PicturePtr> memoryPictures;
    std::condition_variable conditionVar;
    unsigned totolPictureNum = pictureVector.size();
    unsigned curPictureNum = 0;
    emit SignalBtnEnable(false);
    std::thread([&](std::vector<std::string> pictures){
        for(string pictureName : pictures)
        {
            VideoGLWidget::PicturePtr pPicture = fileutil::ReadFileRawData(pictureName);
            if(!pPicture)
                continue;
            unique_lock<mutex> locker(mutexForMemoryPictures);
            memoryPictures.push(pPicture);
            conditionVar.notify_one();
        }
    }, pictureVector).detach();

    std::thread([&](){
        unsigned i = 0;
        while(i < totolPictureNum)
        {
            Sleep(40);
            i++;
            logutil::MyLog(logutil::info, "cur should show picture num:%d\n", i);
        }

    }).detach();

    while(curPictureNum < totolPictureNum)
    {
        unique_lock<mutex> locker(mutexForMemoryPictures);
        if(memoryPictures.empty())
            conditionVar.wait(locker);
        VideoGLWidget::PicturePtr ptr = memoryPictures.front();
        memoryPictures.pop();
        curPictureNum++;
        locker.unlock();

        m_pVideoGLWidget->PictureShow(ptr);
        string textName(ptr->m_filename);
        textName.append("\t\t");
        textName.append(to_string(curPictureNum));
        textName.append("/");
        textName.append(to_string(totolPictureNum));
        ui->m_pLabProcessBar->setText(textName.c_str());
    }
    this->ResetControls();
}

std::vector<std::string> MainWindow::FindPicturesFromDir(std::string dir)
{
    string srcPath = dir;
    dir.append("/*.*");
    dir = QString::fromStdString(dir).replace("/", "\\\\").toStdString();
    std::vector<std::string> pictures;
    struct _finddata_t fileinfo;

    intptr_t fileIdx = _findfirst(dir.c_str(), &fileinfo);
    logutil::MyLog(logutil::info, "search dir:%s\n", dir.c_str());
    if(fileIdx != -1)
    {
        do
        {
            if(!(fileinfo.attrib & _A_SUBDIR))
            {
                string filename(fileinfo.name);
                size_t pos = filename.find_last_of('.');
                if(pos != std::string::npos)
                {
                    string extension = filename.substr(pos+1);
                    std::transform(extension.begin(), extension.end(), extension.begin(), std::ptr_fun<int, int>(tolower));
                    if(extension == "img" || extension == "png" || extension == "jpeg" || extension == "jpg")
                    {
                        qDebug() << "get picture:" << fileinfo.name;
                        pictures.push_back(srcPath + "/" + filename);
                    }
                }
            }
        }while(_findnext(fileIdx, &fileinfo) == 0);
    }
    else
        logutil::MyLog(logutil::info, "could not find file from:%s\n", dir.c_str());
    _findclose(fileIdx);
    return pictures;
}
