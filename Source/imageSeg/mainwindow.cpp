#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    for (int i = 0; i < 255; ++i)
        histoData[i] = 0;
    statusBar()->hide();
    eventIgnore = false;
    qsa = new QScrollArea(this);
    QGridLayout *layout = new QGridLayout(qsa);
    qsa->setWidgetResizable(true);
    label = new QLabel(qsa);
    label->setLayout(layout);
    setCentralWidget(qsa);
    menubar = new QMenuBar(this);
    QMenu *menu = menubar->addMenu("File");
    QAction *action = menu->addAction("Import Image");
    connect(action, &QAction::triggered, this, [=]() { this->menuEvent(importImage); });
    action = menu->addAction("Show Histogram");
    connect(action, &QAction::triggered, this, [=]() { this->menuEvent(showHisto); });
    action = menu->addAction("Toggle Threshold");
    connect(action, &QAction::triggered, this, [=]() { this->menuEvent(toggleSeg); });
    action = menu->addAction("Set Zoom");
    connect(action, &QAction::triggered, this, [=]() { this->menuEvent(setZoom); });
    qsa->setWidget(label);
    ui->menubar->setCornerWidget(menubar, Qt::TopLeftCorner);
    resize(qs);
    zoom = 1.0;
    histoDisp.setScaledContents(true);
    histoDisp.setToolTip("Intensity Histogram");
}

MainWindow::~MainWindow()
{
    hide();
    delete label;
    delete qsa;
    delete menubar;
    delete ui;
}

void MainWindow::menuEvent(menuAction action) {
    if (eventIgnore)
        return;
    QString fileName;
    bool ok;
    float f;
    // Handle menu clicks.
    switch (action) {
    case setZoom:
        if (image.isNull()) {
            f = QInputDialog::getDouble(this, "Set Zoom", "Set Zoom", zoom, 0.2, 4.0, 1, &ok);
            if (ok)
                zoom = f;
        }
        break;
    case toggleSeg:
        showSeg = !showSeg;
        break;
    case importImage:
        histoDisp.hide();
        fileName = QFileDialog::getOpenFileName(this, tr("Import"), "/", tr("Image Files (*.png *.jpg *.bmp)"));
        if (fileName != "") {
            image = QImage(fileName).convertToFormat(QImage::Format_ARGB32_Premultiplied);
            showSeg = true;
            process();
        }
        break;
    case showHisto:
        if (!image.isNull())
            histoDisp.show();
        break;
    }
    if (!image.isNull())
        label->setPixmap(QPixmap::fromImage((showSeg ? processed : image).scaledToWidth(static_cast<int>(static_cast<float>(image.width()) * zoom))));
    repaint();
}

void MainWindow::process() {
    // Process imported image to get binary image.
    if (image.isNull())
        return;
    eventIgnore = true;
    QProgressDialog qpd("Processing Image", "Close", 0, 3, this, Qt::WindowType::FramelessWindowHint);
    qpd.setCancelButton(nullptr);
    setEnabled(false);
    qpd.setValue(0);
    qpd.show();
    QCoreApplication::processEvents();
    for (int i = 0; i < 255; ++i)
        histoData[i] = 0;
    processed = image;
    long threshold = 0;
    // Make the image greyscale if it isn't already.
    for (int i = 0; i < processed.width(); ++i)
        for (int j = 0; j < processed.height(); ++j) {
            QColor qc = processed.pixelColor(i, j);
            int c = (qc.red() + qc.green() + qc.blue()) / 3;
            processed.setPixelColor(i, j, QColor(c, c, c));
            threshold += c;
            ++histoData[c];
        }
    threshold /= (processed.width() * processed.height());
    long mean1, mean2, cnt1, cnt2;
    // Converge means to find proper threshold.
    while (true) {
        qpd.setMaximum(qpd.maximum() + 1);
        qpd.setValue(qpd.value() + 1);
        QCoreApplication::processEvents();
        mean1 = mean2 = cnt1 = cnt2 = 0;
        for (int i = 0; i <= threshold; ++i) {
            mean1 += histoData[i] * i;
            cnt1 += histoData[i];
        }
        cnt1 == 0 ? mean1 = threshold : mean1 /= cnt1;
        for (int i = threshold + 1; i < 255; ++i) {
            mean2 += histoData[i] * i;
            cnt2 += histoData[i];
        }
        cnt2 == 0 ? mean2 = threshold : mean2 /= cnt2;
        int newThreshold = static_cast<int>(mean1 + mean2) / 2;
        if (newThreshold == threshold)
            break;
        threshold = newThreshold;
    }
    qpd.setValue(qpd.value() + 1);
    QCoreApplication::processEvents();
    // Convert Image to black and white.
    for (int i = 0; i < processed.width(); ++i)
        for (int j = 0; j < processed.height(); ++j)
            processed.setPixel(i, j, processed.pixelColor(i, j).red() > threshold ? 0xFFFFFFFF : 0xFF000000);
    qpd.setValue(qpd.value() + 1);
    QCoreApplication::processEvents();
    // Construct histogram and update.
    int minI = INT_MAX, maxI = 0, y;
    for (int i = 0; i < 255; ++i) {
        minI = min(histoData[i], minI);
        maxI = max(histoData[i], maxI);
    }
    ++minI;
    ++maxI;
    y = maxI / minI;
    QImage qi(255, y + 1, QImage::Format_ARGB32_Premultiplied);
    qi.fill(0xFF000000);
    cout << minI << " " << maxI << endl;
    for (int i = 0; i < 255; ++i)
        for (int j = y; j > y - histoData[i] / minI && j >= 0; --j) {
            int c = 127 + i / 2;
            qi.setPixelColor(i, j, QColor(c, c, c));
        }
    histo.convertFromImage(qi);
    histoDisp.setPixmap(histo);
    setEnabled(true);
    eventIgnore = false;
}

