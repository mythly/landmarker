#ifndef GENERATOR_H
#define GENERATOR_H

#include <cstdio>

//opencv
#include <opencv2/world.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;

//sensetime SDK
#include "mcv_facesdk.h"

class Generator
{
public:
    Generator(int W, int H);
    ~Generator();

    void generate(Point2f landmark1[]);
    void generate(Rect2f rect1, Point2f landmark1[]);
    void generate(Mat gray1, Point2f landmark1[]);
    void generate(Mat gray1, Point2f landmark1[], Mat gray0, Point2f landmark0[], Rect2f rect0);

private:
    float overlap(Rect2f a, Rect2f b);

private:
    Size image_size;

    unsigned int pt;

    mcv_handle_t hDetect;
    mcv_handle_t hAlign;

    mcv_result_t ret;
    PMCV_FACERECT pface;
    unsigned int count;
    mcv_pointf_t *result;

    Mat frame_grey;
};

#endif // GENERATOR_H
