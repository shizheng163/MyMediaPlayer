#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <string>
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
    void                        SelectDir();
    void                        ShowPictures(QString dir);
    std::vector<std::string>    FindPicturesFromDir(std::string dir);

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
