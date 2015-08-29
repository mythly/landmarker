#include "mainwindow.h"
#include "ui_mainwindow.h"

GlobalScreen::GlobalScreen(Sequence *seq, QWidget *parent):
    QFrame(parent), s(seq), flag_drag(Empty), playing(false)
{
    setFrameStyle(WinPanel | Sunken);
    setLineWidth(0);
    setFocusPolicy(Qt::NoFocus);

    connect(this, SIGNAL(changed()), parent, SLOT(refresh()));
}

GlobalScreen::~GlobalScreen()
{

}

QString GlobalScreen::hint()
{        
    return "camera : " + toString(s->camera());
}

void GlobalScreen::play()
{
    flag_drag = Empty;
    playing = true;
}

void GlobalScreen::stop()
{
    playing = false;
}

void GlobalScreen::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    const QRectF &c = s->camera();
    qreal scale = qScale(size(), s->size());

    QPainter painter(this);
    painter.setWindow(0, 0, s->width(), s->height());

    painter.drawImage(0, 0, s->image());

    painter.setPen(QPen(Qt::white, 2 * scale));
    painter.drawRect(c);
}

void GlobalScreen::wheelEvent(QWheelEvent *e)
{
    if (playing)
        return;

    QRectF &c = s->camera();
    qreal f = qPow(ScaleFactor, e->angleDelta().y() / qreal(120));
    qreal x = c.x() - c.width() * (f - 1) / 2;
    qreal y = c.y() - c.height() * (f - 1) / 2;
    qreal w = c.width() * f, h = c.height() * f;
    if (f < 1 && w < 16 && h < 16)
        return;
    if (f > 1 && w > s->width() && h > s->height())
        return;

    c = QRectF(x, y, w, h);
    emit changed();
}

void GlobalScreen::mousePressEvent(QMouseEvent *e)
{    
    if (playing)
        return;

    QPointF p = e->localPos();
    QPointF gp = toGlobal(p);    
    QRectF &c = s->camera();

    if (c.contains(gp)) {
        flag_drag = Global;
        start_pos = gp;
        emit changed();
    }else {
        c.moveCenter(gp);        
        emit changed();
    }
}

void GlobalScreen::mouseMoveEvent(QMouseEvent *e)
{    
    if (playing)
        return;

    QPointF p = e->localPos();
    QPointF gp = toGlobal(p);
    QRectF &c = s->camera();

    if (flag_drag == Empty || !QRectF(QPointF(), size()).contains(p))
        return;
    c.translate(gp - start_pos);    
    start_pos = gp;
    emit changed();
}

void GlobalScreen::mouseReleaseEvent(QMouseEvent *)
{
    if (playing)
        return;
    flag_drag = Empty;
}

QPointF GlobalScreen::toGlobal(QPointF p)
{
    return p * qScale(size(), s->size());
}

FaceScreen::FaceScreen(Sequence *seq, QWidget *parent):
    QFrame(parent), s(seq), flag_drag(Empty), playing(false)
{
    setFrameStyle(WinPanel | Sunken);
    setLineWidth(0);
    setFocusPolicy(Qt::NoFocus);

    connect(this, SIGNAL(changed()), parent, SLOT(refresh()));
}

FaceScreen::~FaceScreen()
{

}

QString FaceScreen::hint()
{
    return hints;
}

void FaceScreen::play()
{
    flag_drag = Empty;
    playing = true;
}

void FaceScreen::stop()
{
    playing = false;
}

void FaceScreen::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    Annotation &a = s->annotation();
    QRectF &c = s->camera();

    QPainter painter(this);
    painter.drawImage(QRectF(QPointF(), size()), s->image(), c);
    painter.setWindow(c.toRect());

    a.paint(painter, qScale(size(), c.size()));
}

void FaceScreen::wheelEvent(QWheelEvent *e)
{
    if (playing)
        return;

    QRectF &c = s->camera();
    qreal f = qPow(ScaleFactor, e->angleDelta().y() / qreal(120));
    qreal x = c.x() - c.width() * (f - 1) / 2;
    qreal y = c.y() - c.height() * (f - 1) / 2;
    qreal w = c.width() * f, h = c.height() * f;
    if (f < 1 && w < 16 && h < 16)
        return;
    if (f > 1 && w > s->width() && h > s->height())
        return;

    c = QRectF(x, y, w, h);
    hints = "Camera : " + toString(s->camera());
    emit changed();
}

void FaceScreen::mousePressEvent(QMouseEvent *e)
{
    Annotation &a = s->annotation();
    if (playing)
        return;
    if (e->button() == Qt::RightButton) {
        a.modify_type();
        if (a.type == Annotation::Auto)
            s->generate();
        hints = "Type : " + Annotation::toString(a.type);
        emit changed();
    }else {
        QPointF p = e->localPos();
        QPointF gp = toGlobal(p);
        const QRectF &c = s->camera();
        qreal scale = qScale(size(), c.size());
        flag_drag = target(a, gp, 8 * scale);
        if (flag_drag != Empty) {
            start_pos = gp;
            hints = names[flag_drag] + " : " + toString(gp);
        }else {
            int besti = -1;
            qreal bestD = qInf();
            for (int i = 0; i < M; ++i) {
                qreal D = L2(a.landmark[i] - gp);
                if (D < bestD) {
                    besti = i;
                    bestD = D;
                }
            }
            a.modify_landmark(besti, gp);
            hints = names[besti] + " : " + toString(gp);
        }
        emit changed();
    }
}

void FaceScreen::mouseMoveEvent(QMouseEvent *e)
{
    Annotation &a = s->annotation();
    if (playing || a.type == Annotation::Unclear)
        return;

    QPointF p = e->localPos();
    QPointF gp = toGlobal(p);    

    if (flag_drag == Empty || !QRectF(QPointF(), size()).contains(p))
        return;
    switch (flag_drag) {
    case EyeLeft:
    case EyeRight:
    case Nose:
    case MouseLeft:
    case MouseRight:
        a.modify_landmark(flag_drag, gp);
        hints = names[flag_drag] + " : " + toString(gp);
        break;
    }
    start_pos = gp;
    emit changed();
}

void FaceScreen::mouseReleaseEvent(QMouseEvent *)
{    
    flag_drag = Empty;
}

QPointF FaceScreen::toGlobal(QPointF p)
{    
    const QRectF &c = s->camera();
    qreal x = p.x() / width() * c.width() + c.x();
    qreal y = p.y() / height() * c.height() + c.y();
    return QPointF(x, y);
}

FaceScreen::FlagDrag FaceScreen::target(const Annotation &a, QPointF gp, qreal limit)
{                    
    const QPointF* p = a.landmark;
    for (int i = 0; i < M; ++i)
        if (L2(p[i] - gp) <= limit)
            return FlagDrag(i);
    return Empty;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    gs(NULL), fs(NULL),
    seq(NULL),
    state(Empty),
    timer_frame(new QTimer())
{    
    ui->setupUi(this);
    setWindowTitle(Title);
    ui->slider->setEnabled(false);

    separator[0] = ui->menu_File->insertSeparator(ui->actionExit);
    separator[1] = ui->menu_File->insertSeparator(ui->actionExit);
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recent.push_back(new QAction(this));
        connect(recent[i], SIGNAL(triggered()), this, SLOT(open_recent()));
    }
    for (int i = 0; i < recent.size(); ++i)
        ui->menu_File->insertAction(separator[1], recent[i]);
    update_recent();

    get_folder();
    ui->statusBar->showMessage("Current Image Folder : " + image_folder, LongTime);

    connect(timer_frame, SIGNAL(timeout()), this, SLOT(frame_come()));
    installEventFilter(this);
}

MainWindow::~MainWindow()
{
    //lazy
}

void MainWindow::handleKeyEvent(QKeyEvent *e)
{    
    if (state == Empty)
        return;    
    int delta = 0;
    switch (e->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        delta = -ui->slider->pageStep();
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        delta = -ui->slider->singleStep();
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        delta = ui->slider->pageStep();
        break;
    case Qt::Key_Space:
        if (state == Play) {
            QEvent e(StopVideo);
            handleVideoEvent(&e);
        }else
            delta = ui->slider->singleStep();
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        delta = ui->slider->singleStep();
        break;
    }
    if (delta != 0) {
        int value = ui->slider->value() + delta;
        ui->slider->setValue(value);
    }
}

void MainWindow::handlePaintEvent(QPaintEvent *e)
{                
    QMainWindow::paintEvent(e);
    if (state != Empty) {
        gs->update();
        fs->update();
    }
}

void MainWindow::handleSequenceEvent(QEvent *e)
{
    if (state == Play) {
        QEvent e(StopVideo);
        handleVideoEvent(&e);
    }

    if (e->type() == OpenSequence) {
        QString path = recent_path;
        recent_path.clear();
        if (path.isEmpty())
            path = QFileDialog::getOpenFileName(this, "Open", "", " sequence file (*.json)");
        if (path.isEmpty())
            return;
        path = QFileInfo(path).absoluteFilePath();

        ui->statusBar->showMessage("Loading");
        Sequence* new_seq = new Sequence(path, image_folder);
        if (!new_seq->opened()) {
            ui->statusBar->showMessage("Failed open " + path, LongTime);
            delete new_seq;
            return;
        }
        if (seq != NULL)
            delete seq;
        QGridLayout *g = ui->screen;
        if (state == Empty) {
            g->removeWidget(ui->frame);
            delete ui->frame;
        }else {
            g->removeWidget(gs);
            g->removeWidget(fs);            
            delete gs;
            delete fs;            
        }        

        seq = new_seq;
        state = Normal;
        gs = new GlobalScreen(seq, this);
        fs = new FaceScreen(seq, this);

        qreal scale = qScale(seq->size(), QSize(640, 640));
        if (scale > 1)
            scale = 1;
        gs->setFixedSize(seq->size() * scale);
        fs->setFixedSize(640, 640);
        g->addWidget(fs, 0, 0, 1, 1, Qt::AlignCenter);
        g->addWidget(gs, 0, 1, 1, 1, Qt::AlignCenter);

        ui->slider->setEnabled(true);
        ui->slider->setMinimum(1);
        ui->slider->setMaximum(seq->nFrames());
        ui->slider->setSingleStep(1);
        ui->slider->setPageStep(qRound(seq->rate()));
        QString text = QString::number(1) + "/" + QString::number(seq->nFrames());
        ui->progress->setText(text);
        ui->slider->setValue(1);

        QSettings settings(Author, Title);
        QStringList files = settings.value("recent").toStringList();
        files.removeAll(path);
        files.prepend(path);
        while (files.size() > MaxRecentFiles)
            files.removeLast();
        settings.setValue("recent", files);
        update_recent();

        setWindowTitle(seq->name() + " - " + Title);
        ui->type->setText(Annotation::toString(seq->annotation().type));
        ui->statusBar->showMessage("Success open " + seq->path(), LongTime);
        update();
    }else {
        if (state == Empty)
            ui->statusBar->showMessage("No sequence opened", LongTime);
        else
            if (seq->save())
                ui->statusBar->showMessage("Success save to " + seq->path(), LongTime);
            else
                ui->statusBar->showMessage("Failed save to " + seq->path(), LongTime);
    }
}

void MainWindow::handleVideoEvent(QEvent *e)
{    
    if (state == Empty)
        return;
    if (state == Normal && e->type() == PlayVideo) {
        state = Play;
        timer_frame->start(qRound(1000 / seq->rate()));
        ui->statusBar->showMessage("Start Play");
        gs->play();
        fs->play();
        ui->playButton->setText("Pause");
    }
    if (state == Play && e->type() == StopVideo) {
        state = Normal;
        timer_frame->stop();
        ui->statusBar->showMessage("Finish Play", LongTime);
        gs->stop();
        fs->stop();
        ui->playButton->setText("Play");
    }
    update();
}

void MainWindow::update_recent()
{
    QSettings settings(Author, Title);
    QStringList files = settings.value("recent").toStringList();

    for (int i = 0; i < recent.size(); ++i)
        if (i < files.size()) {
            QString name = QFileInfo(files[i]).fileName();
            QString text = tr("&%1 %2").arg(i + 1).arg(name);
            recent[i]->setText(text);
            recent[i]->setData(files[i]);
            recent[i]->setVisible(true);
        }else
            recent[i]->setVisible(false);

    separator[0]->setVisible(!files.isEmpty());
    separator[1]->setVisible(!files.isEmpty());
}

void MainWindow::get_folder()
{
    QSettings settings(Author, Title);
    image_folder = settings.value("folder").toString();
    if (QDir(image_folder).exists())
        QDir::setCurrent(image_folder);
    image_folder = QFileDialog::getExistingDirectory(this, "Select Image Folder");
    image_folder = QFileInfo(image_folder).absoluteFilePath();
    settings.setValue("folder", image_folder);
}

void MainWindow::on_action_Open_triggered()
{
    QCoreApplication::postEvent(this, new QEvent(OpenSequence));
}

void MainWindow::on_action_Save_triggered()
{
    QCoreApplication::postEvent(this, new QEvent(SaveSequence));
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_playButton_clicked()
{
    if (state == Normal)
        QCoreApplication::postEvent(this, new QEvent(PlayVideo));
    else
        QCoreApplication::postEvent(this, new QEvent(StopVideo));
}

void MainWindow::on_slider_valueChanged(int value)
{
    int max = ui->slider->maximum();
    QString text = QString::number(value) + "/" + QString::number(max);
    ui->progress->setText(text);

    seq->set(value);
    if (state == Normal)
        seq->generate();
    ui->type->setText(Annotation::toString(seq->annotation().type));
    update();
}

void MainWindow::open_recent()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        recent_path = action->data().toString();
        QCoreApplication::postEvent(this, new QEvent(OpenSequence));
    }
}

void MainWindow::frame_come()
{
    int value = ui->slider->value() + ui->slider->singleStep();
    if (value > ui->slider->maximum()) {
        QEvent e(StopVideo);
        handleVideoEvent(&e);
    }else
        ui->slider->setValue(value);
}

void MainWindow::refresh()
{    
    if (state == Normal) {
        if (sender() == gs)
            ui->statusBar->showMessage(gs->hint(), ShortTime);
        if (sender() == fs) {
            ui->statusBar->showMessage(fs->hint(), ShortTime);
            ui->type->setText(Annotation::toString(seq->annotation().type));
        }
    }
    update();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{    
    if (obj != this)
        return false;

    switch (e->type()) {
    case QEvent::KeyPress:
        if (static_cast<QKeyEvent*>(e)->modifiers() == Qt::NoModifier) {
            handleKeyEvent(static_cast<QKeyEvent*>(e));
            return true;
        }
        break;
    case QEvent::Paint:        
        handlePaintEvent(static_cast<QPaintEvent*>(e));
        return true;
        break;
    case OpenSequence:
    case SaveSequence:
        handleSequenceEvent(e);
        return true;
        break;
    case PlayVideo:    
    case StopVideo:        
        handleVideoEvent(e);
        return true;
        break;
    }

    return false;
}
