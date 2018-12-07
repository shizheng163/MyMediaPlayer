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
using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //布局
    ui->setupUi(this);
    this->setFixedSize(this->size());
    this->setStyleSheet("QMainWindow{background-color:#C9C9C9;}");

    unsigned mainWidth(this->width()), mainHeight(this->height());
    ui->m_pLabVideoImage->move(0, 0);
    ui->m_pLabVideoImage->resize(mainWidth, mainHeight * 0.9);
    ui->m_pLabVideoImage->setStyleSheet("background-color:#000000");

    ui->m_pLabProcessBar->move(0, this->height() * 0.85);
    ui->m_pLabProcessBar->resize(ui->m_pLabVideoImage->width(), this->height()*0.05);
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
}

void MainWindow::SlotBtnEnable(bool enable)
{
    ui->m_pBtnOpenFile->setEnabled(enable);
}

void MainWindow::ResetControls()
{
    emit SignalBtnEnable(true);
    ui->m_pLabProcessBar->setText(tr("当前无文件播放"));
    ui->m_pLabVideoImage->setPixmap(QPixmap(QString::fromUtf8("imags/video.ico")));
}

void MainWindow::SelectDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "H://");
    if(dir.isEmpty())
    {
        ui->m_pLabVideoImage->setText("未选择文件夹");
        this->ResetControls();
    }
    else
    {
        ui->m_pLabVideoImage->setText("Show Picture ing....");
        ShowPictures(dir);
    }
}

void MainWindow::ShowPictures(QString dir)
{
    std::vector<std::string> pictures = FindPicturesFromDir(dir.toStdString());
    if(pictures.empty())
    {
        ui->m_pLabVideoImage->setText("no pictures can load.");
        return;
    }
    std::thread(&MainWindow::ShowPicturesInThread, this, pictures).detach();
}

void MainWindow::ShowPicturesInThread(std::vector<string> pictureVector)
{
    std::mutex mutexForMemoryPictures;
    std::queue<PicturePtr> memoryPictures;
    std::condition_variable conditionVar;
    unsigned totolPictureNum = pictureVector.size();
    unsigned curPictureNum = 0;
    emit SignalBtnEnable(false);
    std::thread([&](std::vector<std::string> pictures){
        for(string pictureName : pictures)
        {
            FILE * fPict = fopen(pictureName.c_str(), "rb");
            fseek(fPict, 0, SEEK_END);
            PicturePtr pPicture(new Picture);
            pPicture->m_len = ftell(fPict);
            pPicture->m_pData = new uint8_t[pPicture->m_len];
            pPicture->m_strPictureName = pictureName;
            fseek(fPict, 0, SEEK_SET);
            fread(pPicture->m_pData, sizeof(uint8_t), pPicture->m_len, fPict);
            fclose(fPict);
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
            qDebug() << "cur should show picture num:" << i;
        }

    }).detach();

    QImage *pimg = new QImage;
    while(curPictureNum < totolPictureNum)
    {
        unique_lock<mutex> locker(mutexForMemoryPictures);
        if(memoryPictures.empty())
            conditionVar.wait(locker);
        PicturePtr ptr = memoryPictures.front();
        memoryPictures.pop();
        locker.unlock();
        curPictureNum++;
        if(!pimg->loadFromData(ptr->m_pData, ptr->m_len, "jpg"))
        {
            qDebug() << "error: load memory picture" << ptr->m_strPictureName.c_str();
        }
        ui->m_pLabVideoImage->setPixmap(QPixmap::fromImage(pimg->scaled(ui->m_pLabVideoImage->size())));
        string textName(ptr->m_strPictureName);
        textName.append("\t\t");
        textName.append(to_string(curPictureNum));
        textName.append("/");
        textName.append(to_string(totolPictureNum));
        ui->m_pLabProcessBar->setText(textName.c_str());
    }
    delete pimg;
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
    qDebug() << "search dir:" << dir.c_str();
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
        qDebug() << "could not find file from" << dir.c_str();
    _findclose(fileIdx);
    return pictures;
}
