#ifndef COMMON_H
#define COMMON_H

//Qt
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QMainWindow>
#include <QApplication>

//opencv
#include <opencv2/world.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;

//for Annotation
const int M = 5;
const QString names[M] = {
    "eye_left",
    "eye_right",
    "nose",
    "mouse_left",
    "mouse_right"
};
const double eps = 1e-16;

//for GUI
const QString Author = "MoTao", Title = "Landmarker";
const int MaxRecentFiles = 5, LongTime = 3000, ShortTime = 1000;
const qreal ScaleFactor = 1.1;
const int CacheWidth = 4, CacheSize = 256;

//kits
qreal qScale(QSizeF from, QSizeF to);
qreal L2(QPointF p);
qreal L2(QLineF l, QPointF p);
QString toString(QPoint p);
QString toString(QPointF p);
QString toString(QRect r);
QString toString(QRectF r);
Mat toGray(QImage img);
Mat toBGR(QImage img);

#endif // COMMON_H
