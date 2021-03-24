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
#include <QVBoxLayout>
#include <QKeyEvent>
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

enum menuAction {importImage, importKernal, toggleFilter, setZoom};

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
    int clamp(float val);

    Ui::MainWindow *ui;
    float **kernal, zoom;
    QMenuBar *menubar;
    QScrollArea *qsa;
    QLabel *label;
    int kernalSize;
    QImage image, processed;
    bool showKernal, needGreyscale, eventIgnore;
};
#endif // MAINWINDOW_H
