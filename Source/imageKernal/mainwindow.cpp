#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    statusBar()->hide();
    setWindowTitle("Kernal Filters");
    eventIgnore = false;
    showKernal = true;
    qsa = new QScrollArea(this);
    QVBoxLayout *layout = new QVBoxLayout(qsa);
    qsa->setWidgetResizable(true);
    label = new QLabel(qsa);
    label->setLayout(layout);
    setCentralWidget(qsa);
    menubar = new QMenuBar(this);
    QMenu *menu = menubar->addMenu("File");
    QAction *action = menu->addAction("Import Image");
    connect(action, &QAction::triggered, this, [=]() { this->menuEvent(importImage); });
    action = menu->addAction("Import Kernal");
    connect(action, &QAction::triggered, this, [=]() { this->menuEvent(importKernal); });
    action = menu->addAction("Toggle Filter");
    connect(action, &QAction::triggered, this, [=]() { this->menuEvent(toggleFilter); });
    action = menu->addAction("Set Zoom");
    connect(action, &QAction::triggered, this, [=]() { this->menuEvent(setZoom); });
    qsa->setWidget(label);
    ui->menubar->setCornerWidget(menubar, Qt::TopLeftCorner);
    resize(qs);
    kernal = new float*[1];
    kernal[0] = new float[1];
    kernalSize = 1;
    kernal[0][0] = 1;
    needGreyscale = false;
    zoom = 1.0;
}

MainWindow::~MainWindow()
{
    hide();
    delete label;
    delete qsa;
    delete menubar;
    for (int i = 0; i < kernalSize; ++i)
        delete [] kernal[i];
    delete [] kernal;
    delete ui;
}

void MainWindow::menuEvent(menuAction action) {
    if (eventIgnore)
        return;
    QString fileName;
    bool ok;
    float f;
    switch (action) {
    // Process menu click events.
    case setZoom:
        if (!image.isNull()) {
            f = QInputDialog::getDouble(this, "Set Zoom", "Set Zoom", zoom, 0.2, 4.0, 1, &ok);
            if (ok)
                zoom = f;
        }
        break;
    case toggleFilter:
        showKernal = !showKernal;
        break;
    case importImage:
        fileName = QFileDialog::getOpenFileName(this, tr("Import Image"), "/", tr("Image Files (*.png *.jpg *.bmp)"));
        if (fileName != "") {
            image = QImage(fileName).convertToFormat(QImage::Format_ARGB32_Premultiplied);
            showKernal = true;
            process();
        }
        break;
    case importKernal:
        fileName = QFileDialog::getOpenFileName(this, tr("Import Kernal"), "/", tr("Text Files (*.txt)"));
        // Attempt to import kernal. Check for file and kernal validity.
        if (fileName != "") {
            int tKernalSize;
            float **tKernal;
            fstream file;
            file.open(fileName.toStdString(), ios::in);
            if (file.is_open()) {
                string fromFile;
                if (getline(file, fromFile)) {
                    try {
                        tKernalSize = stoi(fromFile);
                        tKernalSize -= 1 - (tKernalSize % 2);
                        needGreyscale = fromFile.find("G") != string::npos;
                    }
                    catch (...) {
                        break;
                    }
                }
                else
                    break;
                if (tKernalSize < 1)
                    break;
                tKernal = new float*[tKernalSize];
                for (int i = 0; i < tKernalSize; ++i)
                    tKernal[i] = new float[tKernalSize];
                int lines = 0;
                while(lines < tKernalSize && getline(file, fromFile)) {
                    int cnt = 0;
                    while (cnt < tKernalSize && fromFile != "") {
                        size_t index = fromFile.find(" ");
                        if (index == string::npos)
                            index = fromFile.length();
                        try {
                            tKernal[lines][cnt] = stof(fromFile.substr(0, index));
                        }
                        catch (...) {
                            lines = -1;
                            break;
                        }
                        ++cnt;
                        fromFile = index + 1 >= fromFile.length() ? "" : fromFile.substr(index + 1);
                    }
                    if (cnt < tKernalSize) {
                        lines = -1;
                    }
                    if (lines == -1)
                        break;
                    ++lines;
                }
                file.close();
                if (lines == tKernalSize) {
                    for (int i = 0; i < kernalSize; ++i)
                        delete [] kernal[i];
                    delete kernal;
                    kernalSize = tKernalSize;
                    kernal = new float*[kernalSize];
                    for (int i = 0; i < kernalSize; ++i)
                        kernal[i] = new float[kernalSize];
                    for (int i = 0; i < kernalSize; ++i)
                        for (int j = 0; j < kernalSize; ++j)
                            kernal[i][j] = tKernal[i][j];
                }
                for (int i = 0; i < tKernalSize; ++i)
                    delete [] tKernal[i];
                delete [] tKernal;
            }
            showKernal = true;
            process();
        }
        break;
    }
    if (!image.isNull())
        label->setPixmap(QPixmap::fromImage((showKernal ? processed : image).scaledToWidth(static_cast<int>(static_cast<float>(image.width()) * zoom))));
    repaint();
}

void MainWindow::process() {
    if (image.isNull())
        return;
    eventIgnore = true;
    QProgressDialog qpd("Processing Image", "Close", 0, image.width() * (needGreyscale ? 3 : 1), this, Qt::WindowType::FramelessWindowHint);
    qpd.setCancelButton(nullptr);
    setEnabled(false);
    qpd.show();
    QCoreApplication::processEvents();
    processed = QImage(image.size(), QImage::Format_ARGB32_Premultiplied);
    QImage temp = image;
    // Convert the image to greyscale.
    if (needGreyscale) {
        for (int i = 0; i < temp.width(); ++i) {
            qpd.setValue(qpd.value() + 1);
            for (int j = 0; j < temp.height(); ++j) {
                QColor qc = temp.pixelColor(i, j);
                int grey = (qc.red() + qc.green() + qc.blue()) / 3;
                temp.setPixelColor(i, j, QColor(grey, grey, grey));
            }
        }
    }
    int boost = 0;
    // Apply kernal to the image.
    for (int i = 0; i < temp.width(); ++i) {
        qpd.setValue(qpd.value() + 1);
        QCoreApplication::processEvents();
        for (int j = 0; j < temp.height(); ++j) {
            int offset = kernalSize / 2;
            int xstart = max(i - offset, 0), ystart = max(j - offset, 0), xend = min(i + offset, temp.width() - 1), yend = min(j + offset, temp.height() - 1);
            float r = 0.0, g = 0.0, b = 0.0;
            for (int x = xstart; x <= xend; ++x) {
                int dx = x - xstart;
                for (int y = ystart; y <= yend; ++y) {
                    int dy = y - ystart;
                    QColor qc = image.pixelColor(x, y);
                    r += kernal[dx][dy] * static_cast<float>(qc.red());
                    g += kernal[dx][dy] * static_cast<float>(qc.green());
                    b += kernal[dx][dy] * static_cast<float>(qc.blue());
                }
            }
            if (needGreyscale)
                boost = max(boost, max(clamp(r), max(clamp(g), clamp(b))));
            processed.setPixelColor(i, j, QColor(clamp(r), clamp(g), clamp(b)));
        }
    }
    // Boost contrast with greyscale output so that the result is more visible / distinguishable.
    if (needGreyscale) {
        float boostF = 255.0 / static_cast<float>(boost);
        for (int i = 0; i < processed.width(); ++i) {
            qpd.setValue(qpd.value() + 1);
            for (int j = 0; j < processed.height(); ++j) {
                QColor qc = processed.pixelColor(i, j);
                int color = static_cast<int>(static_cast<float>(qc.red()) * boostF);
                processed.setPixelColor(i, j, QColor(color, color, color));
            }
        }
    }
    setEnabled(true);
    eventIgnore = false;
}

int MainWindow::clamp(float val) {
    return static_cast<int>(val > 255.0 ? 255.0 : (val < 0.0 ? 0.0 : val));
}

