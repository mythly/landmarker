#include "annotation.h"

void Annotation::init(int w, int h)
{
    center_image.x = w * 0.5;
    center_image.y = h * 0.5;
    focal_length = max(w, h);

    model3d[0] = Point3d(-0.3183259666, -0.3255176246, -0.3641529083);
    model3d[1] = Point3d(0.3183259666, -0.3255176246, -0.3641529083);
    model3d[2] = Point3d(0, 0, 0);
    model3d[3] = Point3d(-0.2724561393, 0.3472396433, -0.3292782605);
    model3d[4] = Point3d(0.2724561393, 0.3472396433, -0.3292782605);
    eye_middle = (model3d[0] + model3d[1]) / 2;
    sphere_center = Point3d(0, -0.2, -0.767);
    sphere_radius = 0.3183259666;
}

Point2d Annotation::center_image;
double Annotation::focal_length, Annotation::sphere_radius;
Point3d Annotation::model3d[M], Annotation::eye_middle, Annotation::sphere_center;

Annotation::Type Annotation::toType(QString st)
{
    if (st.isEmpty())
        return Empty;
    for (int i = 0; !maps[i].isEmpty(); ++i)
        if (st == maps[i])
            return Type(i);
    return Empty;
}

QString Annotation::toString(Annotation::Type t)
{
    return maps[t];
}

QColor Annotation::toColor(Annotation::Type t)
{
    if (t == Manual)
        return Qt::blue;
    if (t == Auto)
        return Qt::red;
    return QColor(127, 127, 127, 127);
}

const QString Annotation::maps[] = {
    "Manual",    
    "Auto",
    "Unclear",
    "Empty",
    ""
};

Annotation::Annotation(QJsonObject obj)
{    
    index = obj["index"].toInt();
    image = obj["image"].toString();
    type = toType(obj["type"].toString());

    {
        QJsonObject t = obj["landmark"].toObject();
        for (int i = 0; i < M; ++i) {
            QJsonObject p = t[names[i]].toObject();
            landmark[i].setX(p["x"].toDouble());
            landmark[i].setY(p["y"].toDouble());
        }
    }

    {
        QJsonObject t = obj["pose"].toObject();
        pose_roll = t["roll"].toDouble();
        pose_yaw = t["yaw"].toDouble();
        pose_pitch = t["pitch"].toDouble();
    }

    {
        QJsonObject t = obj["rect"].toObject();
        qreal x = t["x"].toDouble();
        qreal y = t["y"].toDouble();
        qreal w = t["width"].toDouble();
        qreal h = t["height"].toDouble();
        if (w > h + eps) {
            x += (w - h) / 2;
            w = h;
        }
        if (h > w + eps) {
            y += (h - w) / 2;
            h = w;
        }
        rect = QRectF(x, y, w, h);
    }

    {
        QJsonObject t = obj["ellipse"].toObject();
        ellipse_x = t["x"].toDouble();
        ellipse_y = t["y"].toDouble();
        ellipse_major_radius = t["major_radius"].toDouble();
        ellipse_minor_radius = t["minor_radius"].toDouble();
        ellipse_angle = t["angle"].toDouble();
    }

    update();
}

Annotation::~Annotation()
{

}

QJsonObject Annotation::toObject()
{
    QJsonObject obj;

    obj["index"] = index;
    obj["image"] = image;
    obj["type"] = toString(type);

    {
        QJsonObject t;
        for (int i = 0; i < M; ++i) {
            QJsonObject p;
            p["x"] = landmark[i].x();
            p["y"] = landmark[i].y();
            t[names[i]] = p;
        }
        obj["landmark"] = t;
    }

    {
        QJsonObject t;
        t["roll"] = pose_roll;
        t["yaw"] = pose_yaw;
        t["pitch"] = pose_pitch;
        obj["pose"] = t;
    }

    {
        QJsonObject t;
        t["x"] = rect.x();
        t["y"] = rect.y();
        t["width"] = rect.width();
        t["height"] = rect.height();
        obj["rect"] = t;
    }

    {
        QJsonObject t;
        t["x"] = ellipse_x;
        t["y"] = ellipse_y;
        t["major_radius"] = ellipse_major_radius;
        t["minor_radius"] = ellipse_minor_radius;
        t["angle"] = ellipse_angle;
        obj["ellipse"] = t;
    }

    return obj;
}

void Annotation::paint(QPainter &p, qreal scale)
{
    {
        qreal W = scale;
        p.setPen(QPen(Qt::green, W, Qt::DashLine));
        p.setBrush(QBrush());
        p.drawRect(rect);
    }

    {
        qreal W = scale;
        p.setPen(QPen(Qt::green, W, Qt::DashLine));
        p.setBrush(QBrush());
        p.save();
        p.translate(ellipse_x, ellipse_y);
        p.rotate(qRadiansToDegrees(ellipse_angle));
        p.drawEllipse(QPointF(0, 0), ellipse_major_radius, ellipse_minor_radius);
        p.restore();
    }

    {
        qreal rx[] = {5, 5, 2, 3, 3, 4};
        qreal ry[] = {2, 2, 5, 3, 3, 4};
        for (int i = 0; i <= M; ++i) {
            rx[i] *= scale;
            ry[i] *= scale;
        }

        p.setPen(QPen(toColor(type)));
        p.setBrush(QBrush(toColor(type)));
        for (int i = 0; i < M; ++i)
            p.drawEllipse(landmark[i], rx[i], ry[i]);
    }
}

void Annotation::modify_type()
{
    if (type != Unclear)
        type = Unclear;
    else
        type = Auto;
}

void Annotation::modify_landmark(int idx, QPointF p)
{
    type = Manual;
    landmark[idx] = p;
    update();
}

void Annotation::update()
{    
    posit(landmark);
    get_pose();
    get_rect();
    get_ellipse();
    get_camera();
}

void Annotation::posit(QPointF landmark[])
{
    Mat A(4, M, CV_64FC1), B(2, M, CV_64FC1);
    for (int i = 0; i < M; ++i) {
        Point3d t = model3d[i] - sphere_center;
        A.at<double>(0, i) = t.x;
        A.at<double>(1, i) = t.y;
        A.at<double>(2, i) = t.z;
        A.at<double>(3, i) = 1;
        B.at<double>(0, i) = (landmark[i].x() - center_image.x) / focal_length;
        B.at<double>(1, i) = (landmark[i].y() - center_image.y) / focal_length;
    }
    Mat tA = A.inv(DECOMP_SVD), tB = B.clone(), RT(3, 4, CV_64FC1);
    RT = 0;
    const static Rect rx(0, 0, 3, 1), ry(0, 1, 3, 1), rz(0, 2, 3, 1);
    for (int i = 0; i < 8; ++i) {
        RT.rowRange(0, 2) = tB * tA;
        double T0 = 1 / sqrt(RT(rx).dot(RT(rx)));
        double T1 = 1 / sqrt(RT(ry).dot(RT(ry)));
        double Tz = sqrt(T0 * T1);
        RT.rowRange(0, 2) *= Tz;
        RT(rx).cross(RT(ry)).copyTo(RT(rz));
        RT.at<double>(2, 3) = Tz;
        Mat w = RT.row(2) * A / Tz;
        Mat _tB = tB.clone();
        tB.row(0) = B.row(0).mul(w);
        tB.row(1) = B.row(1).mul(w);
        Mat d = tB - _tB;
        double delta = focal_length * focal_length * d.dot(d);
        if (i > 0 && delta < 1)
            break;
    }
    rotate = RT.colRange(0, 3);
    translate = RT.col(3);
}

Point3d Annotation::transform(Point3d p)
{
    p = p - sphere_center;
    Matx31d P(p.x, p.y, p.z);
    Matx31d res = rotate * P + translate;
    return Point3d(res(0, 0), res(1, 0), res(2, 0));
}

Point2d Annotation::project(Point3d p)
{
    double x = p.x / p.z * focal_length + center_image.x;
    double y = p.y / p.z * focal_length + center_image.y;
    return Point2d(x, y);
}

void Annotation::get_pose()
{
    Matx33d R = rotate;
    if (abs(1 - abs(R(2, 1))) > 1.0e-7f) {
        pose_roll = atan2(-R(0, 1), R(1, 1));
        pose_yaw = atan2(-R(2, 0), R(2, 2));
        pose_pitch = asin(R(2, 1));
    }
    else {
        pose_roll = atan2(R(1, 0), R(0, 0));
        pose_yaw = 0.0f;
        pose_pitch = R(2, 1) > 0 ? M_PI / 2 : -M_PI / 2;
    }
}

void Annotation::get_rect()
{
    Point2d center, up;
    double scale;
    calculate_face(eye_middle, 50, center, scale, up);

    center -= up * scale * 25;
    qreal w = 100 * scale, h = 100 * scale;
    qreal x = center.x - w / 2;
    qreal y = center.y - h / 2;
    rect = QRectF(x, y, w, h);
}

void Annotation::get_ellipse()
{
    Point2d center, up;
    double scale;
    calculate_face(sphere_center, 50, center, scale, up);

    ellipse_x = center.x;
    ellipse_y = center.y;
    ellipse_major_radius = 145 * scale / 2;
    ellipse_minor_radius = 100 * scale / 2;
    ellipse_angle = atan2(up.y, up.x);
}

void Annotation::get_camera()
{
    Point2d center, up;
    double scale;
    calculate_face(sphere_center, 50, center, scale, up);

    qreal w = 150 * scale, h = 150 * scale;
    qreal x = landmark[2].x() - w / 2;
    qreal y = landmark[2].y() - h / 2;
    camera = QRectF(x, y, w, h);
}

void Annotation::calculate_face(Point3d center, double eye_dist, Point2d &center2d, double &scale, Point2d &up)
{
    Point3d center3d = transform(center);
    center2d = project(center3d);
    double angleX = atan(center3d.x / sqrt(center3d.z * center3d.z + center3d.x * center3d.x));
    double angleY = atan(center3d.y / sqrt(center3d.z * center3d.z + center3d.y * center3d.y));
    double angleB = asin(sphere_radius / norm(center3d));
    double top = tan(angleY - angleB) * focal_length;
    double bottom = tan(angleY + angleB) * focal_length;
    double left = tan(angleX - angleB) * focal_length;
    double right = tan(angleX + angleB) * focal_length;

    scale = max(abs(top - bottom), abs(left - right)) / eye_dist;
    Point3d t3d = center;
    t3d.y -= 0.5;
    Point2d t2d = project(transform(t3d)) - center2d;
    double l = norm(t2d);
    up = (1 / l) * t2d;
}
