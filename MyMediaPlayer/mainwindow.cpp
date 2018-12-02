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
using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->m_pSelectPictBtn, &QPushButton::clicked, this, &SelectDir);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SelectDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "H://");
    if(dir.isEmpty())
    {
        ui->m_pPictuerLabel->setText("未选择文件夹");
    }
    else
    {
        ui->m_pPictuerLabel->setText("Show Picture ing....");
        ShowPictures(dir);
    }
}

void MainWindow::ShowPictures(QString dir)
{
    std::vector<std::string> pictures = FindPicturesFromDir(dir.toStdString());
    if(pictures.empty())
    {
        ui->m_pPictuerLabel->setText("no pictures can load.");
        return;
    }
    std::thread([&](std::vector<std::string> pictures){
        QImage *pimg = new QImage;
        for(string pictName : pictures)
        {
            if(!pimg->load(pictName.c_str()))
            {
                qDebug() << "error: picture" << pictName.c_str() << "could not right load.";
                continue;
            }
            ui->m_pPictuerLabel->setPixmap(QPixmap::fromImage(pimg->scaled(ui->m_pPictuerLabel->size())));
            qDebug() << "showing picture " << pictName.c_str();
            Sleep(40);
        }
        delete pimg;
        QApplication::quit();
    }, pictures).detach();
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
                cout << "process file:" << fileinfo.name << endl;
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
