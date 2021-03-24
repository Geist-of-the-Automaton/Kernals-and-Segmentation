#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QScrollArea>
#include <QMenuBar>
#include <QMenu>
#include <QLabel>
#include <QPixmap>
#include <QFileDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QProgressDialog>
#include <exception>

using std::exception;

#include <fstream>
#include <string>

using std::stof;
using std::string;
using std::fstream;
using std::ios;
using std::min;
using std::max;

#include <iostream>
using std::cout;
using std::endl;

using std::to_string;

const QSize qs (700, 700);

enum menuAction {importImage, showHisto, toggleSeg, setZoom};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void menuEvent(menuAction action);

private:
    void process();

    Ui::MainWindow *ui;
    float zoom;
    QMenuBar *menubar;
    QScrollArea *qsa;
    QLabel *label, histoDisp;
    QImage image, processed;
    QPixmap histo;
    bool showSeg, eventIgnore;
    int histoData[255];
};
#endif // MAINWINDOW_H
