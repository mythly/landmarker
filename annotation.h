#ifndef ANNOTATION_H
#define ANNOTATION_H

#include "common.h"

class Annotation
{
public:    
    enum Type { Manual, Auto, Unclear, Empty };

    static void init(int w, int h);
    static Point2d center_image;
    static double focal_length, sphere_radius;
    static Point3d model3d[M], eye_middle, sphere_center;

    static Type toType(QString st);
    static QString toString(Type t);
    static QColor toColor(Type t);
    const static QString maps[];

public:
    Annotation(QJsonObject obj);
    ~Annotation();
    QJsonObject toObject();

    void paint(QPainter &p, qreal scale);

    void modify_type();
    void modify_landmark(int idx, QPointF p);
    void update();

    int index;
    QString image;
    Type type;
    qreal pose_roll, pose_yaw, pose_pitch;
    QRectF rect;
    qreal ellipse_x, ellipse_y, ellipse_major_radius, ellipse_minor_radius, ellipse_angle;
    QRectF camera;
    QPointF landmark[M];    

private:
    void posit(QPointF landmark[]);
    Point3d transform(Point3d p);
    Point2d project(Point3d p);
    void get_pose();
    void get_rect();
    void get_ellipse();
    void get_camera();
    void calculate_face(Point3d center, double eye_dist, Point2d &center2d, double &scale, Point2d &up);

    Matx33d rotate;
    Matx31d translate;
};

#endif // ANNOTATION_H
