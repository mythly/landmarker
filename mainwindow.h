#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"
#include "sequence.h"
#include "annotation.h"

namespace Ui {
class MainWindow;
}

const QEvent::Type
    OpenSequence = QEvent::Type(QEvent::User + 1),
    SaveSequence = QEvent::Type(QEvent::User + 2),    
    PlayVideo = QEvent::Type(QEvent::User + 3),
    StopVideo = QEvent::Type(QEvent::User + 4);

class GlobalScreen : public QFrame
{
    Q_OBJECT

public:
    GlobalScreen(Sequence *seq, QWidget *parent);
    ~GlobalScreen();

    QString hint();
    void play();
    void stop();

signals:
    void changed();

protected:
    void paintEvent(QPaintEvent *e);
    void wheelEvent(QWheelEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);

private:
    enum FlagDrag { Empty, Global };
    QPointF toGlobal(QPointF p);

private:
    Sequence *s;
    FlagDrag flag_drag;
    QPointF start_pos;
    bool playing;
};

class FaceScreen : public QFrame
{
    Q_OBJECT

public:
    FaceScreen(Sequence *seq, QWidget *parent);
    ~FaceScreen();

    QString hint();
    void play();
    void stop();

signals:
    void changed();

protected:
    void paintEvent(QPaintEvent *e);
    void wheelEvent(QWheelEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);

private:
    enum FlagDrag {
        EyeLeft, EyeRight, Nose, MouseLeft, MouseRight, Empty
    };
    QPointF toGlobal(QPointF p);
    FlagDrag target(const Annotation& a, QPointF gp, qreal limit);        

private:
    Sequence *s;
    FlagDrag flag_drag;
    QPointF start_pos;
    bool playing;
    QString hints;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    enum State { Empty, Normal, Play };

    void handleKeyEvent(QKeyEvent *e);
    void handlePaintEvent(QPaintEvent *e);
    void handleSequenceEvent(QEvent *e);
    void handleVideoEvent(QEvent *e);

    void update_recent();
    void get_folder();

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private slots:
    void on_action_Open_triggered();
    void on_action_Save_triggered();    
    void on_actionExit_triggered();    
    void on_playButton_clicked();
    void on_slider_valueChanged(int value);

    void open_recent();
    void frame_come();
    void refresh();

private:
    Ui::MainWindow *ui;
    GlobalScreen *gs;
    FaceScreen *fs;
    Sequence *seq;
    QString image_folder;

    QAction* separator[2];
    QVector<QAction*> recent;
    QString recent_path;

    State state;
    QTimer *timer_frame;
};

#endif // MAINWINDOW_H
