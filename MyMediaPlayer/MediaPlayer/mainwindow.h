#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <string>
#include <memory>
#include "videoglwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void                        SignalBtnEnable(bool);

private:
    void                        SlotBtnEnable(bool enable);
    void                        ResetControls();
    void                        SelectDir();
    void                        ShowPictures(QString dir);
    void                        ShowPicturesInThread(std::vector<std::string> pictureVector);
    std::vector<std::string>    FindPicturesFromDir(std::string dir);

    Ui::MainWindow *ui;

    VideoGLWidget              *m_pVideoGLWidget;
};

#endif // MAINWINDOW_H
