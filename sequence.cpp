#include "sequence.h"

Sequence::Sequence(QString path, QString dir)
{    
    flag_open = false;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return;
    _path = path;

    doc = QJsonDocument::fromJson(f.readAll());
    obj = doc.object();
    arr = obj["annotation"].toArray();

    QString keys[] = {
        "name",
        "nFrames",
        "rate",        
        "width",
        "height",
        ""
    };
    for (int i = 0; !keys[i].isEmpty(); ++i)
        if (!obj.contains(keys[i]))
            return;
    _name = obj["name"].toString();
    _N =  obj["nFrames"].toInt();
    _rate = obj["rate"].toDouble();
    _W = obj["width"].toInt();
    _H = obj["height"].toInt();
    Annotation::init(_W, _H);
    _g = new Generator(_W, _H);

    _dir = dir + "/" + _name + "/";
    _a.clear();
    _image.clear();
    for (int i = 0; i < _N; ++i) {
        QJsonObject a = arr[i].toObject();
        _a.push_back(new Annotation(a));
        _image.push_back(NULL);
    }

    set(1);
    flag_open = true;
}

Sequence::~Sequence()
{
    delete _g;
    for (int i = 0; i < _a.size(); ++i)
        delete _a[i];
    for (int i = 0; i < _image.size(); ++i)
        if (_image[i] != NULL)
            delete _image[i];
}

bool Sequence::save()
{
    obj["name"] = _name;
    obj["nFrames"] = _N;
    obj["rate"] = _rate;
    obj["width"] = _W;
    obj["height"] = _H;
    for (int i = 0; i < _a.size(); ++i)
        arr[i] = _a[i]->toObject();
    obj["annotation"] = arr;
    doc.setObject(obj);

    QFile f(path());
    if (!f.open(QIODevice::WriteOnly))
        return false;
    f.write(doc.toJson());

    return true;
}

bool Sequence::opened()
{            
    return flag_open;
}

QString Sequence::path()
{
    return _path;
}

QString Sequence::name()
{
    return _name;
}

qreal Sequence::rate()
{
    return _rate;
}

int Sequence::width()
{
    return _W;
}

int Sequence::height()
{
    return _H;
}

QSize Sequence::size()
{
    return QSize(width(), height());
}

int Sequence::nFrames()
{
    return _N;
}

Annotation& Sequence::annotation()
{
    return *_a[n];
}

QRectF &Sequence::camera()
{
    return _c;
}

const QImage& Sequence::image(int idx)
{
    if (idx < 0)
        idx = n;
    if (_image[idx] == NULL) {
        int start = idx - idx % CacheWidth;
        int end = start + CacheWidth;
        for (int i = start; i < end; ++i)
            if (i < _N && _image[i] == NULL) {
                _image[i] = new QImage(_dir + _a[i]->image);
                _q.push_back(i);
            }
        while (_q.size() > CacheSize) {
            int i = _q.front();
            _q.pop_front();
            if (_image[i] != NULL) {
                delete _image[i];
                _image[i] = NULL;
            }
        }
    }
    return *_image[idx];
}

void Sequence::set(int value, bool gen)
{    
    if (value > 0)
        n = value - 1;
    if (gen && (_a[n]->type == Annotation::Empty || _a[n]->type == Annotation::Auto)) {
        if (n > 0 && _a[n - 1]->type == Annotation::Unclear) {
            _a[n]->type = Annotation::Unclear;
            return;
        }
        Mat gray0, gray1;
        Point2f landmark0[M], landmark1[M];
        if (n > 0 && (_a[n - 1]->type == Annotation::Manual || _a[n - 1]->type == Annotation::Auto)) {
            Rect2f rect0(_a[n - 1]->rect.x(), _a[n - 1]->rect.y(), _a[n - 1]->rect.width(), _a[n - 1]->rect.height());
            gray0 = toGray(image(n - 1));
            gray1 = toGray(image(n));
            for (int i = 0; i < M; ++i) {
                landmark0[i].x = _a[n - 1]->landmark[i].x();
                landmark0[i].y = _a[n - 1]->landmark[i].y();
            }
            _g->generate(gray1, landmark1, gray0, landmark0, rect0);
        }else {
            gray1 = toGray(image(n));
            _g->generate(gray1, landmark1);
        }
        _a[n]->type = Annotation::Auto;
        for (int i = 0; i < M; ++i) {
            _a[n]->landmark[i].setX(landmark1[i].x);
            _a[n]->landmark[i].setY(landmark1[i].y);
        }
        _a[n]->update();
    }
    if (_a[n]->type == Annotation::Manual || _a[n]->type == Annotation::Auto)
        _c = _a[n]->camera;
}

int Sequence::get()
{
    return n + 1;
}
