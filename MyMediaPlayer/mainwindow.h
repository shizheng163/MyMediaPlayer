#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <string>
#include <memory>
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
    struct Picture
    {
        Picture()
        {
            m_pData = NULL;
            m_len = 0;
        }
        ~Picture()
        {
            if(m_pData)
                delete m_pData;
        }
        uint8_t  *  m_pData;
        unsigned    m_len;
        std::string m_strPictureName;

    };
    typedef std::shared_ptr<Picture> PicturePtr;

    void                        SlotBtnEnable(bool enable);
    void                        ResetControls();
    void                        SelectDir();
    void                        ShowPictures(QString dir);
    void                        ShowPicturesInThread(std::vector<std::string> pictureVector);
    std::vector<std::string>    FindPicturesFromDir(std::string dir);

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
