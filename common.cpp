#include "common.h"

qreal qScale(QSizeF from, QSizeF to)
{
    qreal sw = to.width() / from.width();
    qreal sh = to.height() / from.height();
    return sw * sh < 1 ? min(sw, sh) : max(sw, sh);
}

qreal L2(QPointF p)
{
    return qSqrt(p.x() * p.x() + p.y() * p.y());
}

qreal L2(QLineF l, QPointF p)
{
    if (QPointF::dotProduct(p - l.p1(), l.p2() - l.p1()) < 0)
        return L2(p - l.p1());
    if (QPointF::dotProduct(p - l.p2(), l.p1() - l.p2()) < 0)
        return L2(p - l.p2());
    QPointF a(p - l.p1()), b(l.p2() - l.p1());
    return qAbs(a.x() * b.y() - a.y() * b.x()) / L2(b);
}

QString toString(QPoint p)
{
    return QString("(%1, %2)").arg(p.x()).arg(p.y());
}

QString toString(QPointF p)
{
    return toString(p.toPoint());
}

QString toString(QRect r)
{
    return QString("%1x%2").arg(r.width()).arg(r.height()) + " from " + toString(r.topLeft());
}

QString toString(QRectF r)
{
    return toString(r.toRect());
}

Mat toGray(QImage img)
{
    Mat ret(img.height(), img.width(), CV_8UC1);
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            ret.at<unsigned char>(y, x) = (unsigned char)qBound(0, qGray(img.pixel(x, y)), 255);
    return ret;
}

Mat toBGR(QImage img)
{
    Mat ret(img.height(), img.width(), CV_8UC3);
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) {
            QRgb c = img.pixel(x, y);
            ret.at<unsigned char>(y, 3 * x) = (unsigned char)(c & 0xff);
            ret.at<unsigned char>(y, 3 * x + 1) = (unsigned char)((c >> 8) & 0xff);
            ret.at<unsigned char>(y, 3 * x + 2) = (unsigned char)((c >> 16) & 0xff);
        }    
    return ret;
}
