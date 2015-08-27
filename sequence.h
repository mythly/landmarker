#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "common.h"
#include "annotation.h"
#include "generator.h"

class Sequence
{
public:
    Sequence(QString path, QString dir);
    ~Sequence();

    bool opened();
    bool save();

    QString path();
    QString name();
    qreal rate();
    int width();
    int height();
    QSize size();
    int nFrames();

    Annotation &annotation();
    QRectF &camera();
    const QImage &image(int idx = -1);

    void set(int value = -1, bool gen = true);
    int get();

private:
    bool flag_open;
    QString _name;
    qreal _rate;
    int n, _N, _W, _H;
    Generator *_g;

    QJsonDocument doc;
    QJsonObject obj;
    QJsonArray arr;
    QString _path, _dir;
    QVector<Annotation*> _a;
    QVector<QImage*> _image;
    QQueue<int> _q;
    QRectF _c;
};

#endif // SEQUENCE_H
