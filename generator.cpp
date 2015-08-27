#include "generator.h"

#define DEFAULT_ALIGNMENT_LANDMARKS 21

Generator::Generator(int W, int H)
{
    image_size = Size(W, H);

    pt = DEFAULT_ALIGNMENT_LANDMARKS;
    hDetect = mcv_facesdk_create_frontal_detector_instance_from_resource(2);
    hAlign = mcv_facesdk_create_LRAlignmentor_instance_from_resource(pt);
    if (!hDetect || !hAlign)
        fprintf(stderr, "fail to init handle\n");
    ret = MCV_OK;
    pface = NULL;
}

Generator::~Generator()
{
    mcv_facesdk_destroy_LRAlignmentor_instance(hAlign);
    mcv_facesdk_destroy_frontal_instance(hDetect);
}

void Generator::generate(Point2f landmark1[])
{
    Point2f mean_pose[] = {
        Point2f(11.0812f, 10.7494f),
        Point2f(26.9188f, 10.7494f),
        Point2f(19.0000f, 20.4058f),
        Point2f(12.1041f, 27.9159f),
        Point2f(25.8959f, 27.9159f)
    };
    float scale = float(min(image_size.width, image_size.height)) / 80.0f;
    for (int i = 0; i < 5; ++i) {
        landmark1[i].x = (mean_pose[i].x - mean_pose[2].x) * scale + image_size.width * 0.5f;
        landmark1[i].y = (mean_pose[i].y - mean_pose[2].y) * scale + image_size.height * 0.5f;
    }
}

void Generator::generate(Mat gray1, Point2f landmark1[])
{
    gray1.copyTo(frame_grey);

    ret = mcv_facesdk_frontal_detector(
        hDetect, frame_grey.data, frame_grey.cols, frame_grey.rows,
        frame_grey.cols, &pface, &count);
    if (ret != MCV_OK){
        fprintf(stderr, "frontal detector error : %d", ret);
        return ;
    }

    if(count >= 1) {
        result = new mcv_pointf_t[pt];
        ret = mcv_facesdk_LRAlign(hAlign, frame_grey.data, frame_grey.cols, frame_grey.rows,
            frame_grey.cols, pface[0].Rect, static_cast<int>(pface[0].Pose), pt, result);
        if (ret != MCV_OK){
            fprintf(stderr, "LRAlign error : %d", ret);
            return ;
        }
        for (int j= 0; j < 5; j++)
            landmark1[j] = Point2f(result[j + DEFAULT_ALIGNMENT_LANDMARKS - 5].x, result[j + DEFAULT_ALIGNMENT_LANDMARKS - 5].y);
        delete result;
    }
    else
        generate(landmark1);
    mcv_facesdk_release_frontal_result(pface, count);

}
void Generator::generate(Mat gray1, Point2f landmark1[], Mat gray0, Point2f landmark0[], Rect2f rect0)
{
    gray1.copyTo(frame_grey);

    ret = mcv_facesdk_frontal_detector(
        hDetect, frame_grey.data, frame_grey.cols, frame_grey.rows,
        frame_grey.cols, &pface, &count);
    if (ret != MCV_OK){
        fprintf(stderr, "frontal detector error : %d", ret);
        return ;
    }

    result = new mcv_pointf_t[pt];
    if(count == 1) {
        ret = mcv_facesdk_LRAlign(hAlign, frame_grey.data, frame_grey.cols, frame_grey.rows,
        frame_grey.cols, pface[0].Rect, static_cast<int>(pface[0].Pose), pt, result);
    }
    else
        if(count > 1) {
            float best_score = 0;
            int best_index = 0;
            for(int j = 0; j < count; j++)
            {
                Rect2f tmp_rect;
                tmp_rect.x = pface[j].Rect.left;
                tmp_rect.y = pface[j].Rect.top;
                tmp_rect.width = pface[j].Rect.right - pface[j].Rect.left;
                tmp_rect.height = pface[j].Rect.bottom - pface[j].Rect.top;

                float score = overlap(tmp_rect, rect0);
                if(score > best_score)
                {
                    best_score = score;
                    best_index = j;
                }
            }

            ret = mcv_facesdk_LRAlign(hAlign, frame_grey.data, frame_grey.cols, frame_grey.rows,
            frame_grey.cols, pface[best_index].Rect, static_cast<int>(pface[best_index].Pose), pt, result);
        }
        else {
            mcv_rect_t face;
            face.left = int(rect0.x + 0.5f);
            face.top = int(rect0.y + 0.5f);
            face.right = int(rect0.x + rect0.width + 0.5f);
            face.bottom = int(rect0.y + rect0.height + 0.5f);

            ret = mcv_facesdk_LRAlign(hAlign, frame_grey.data, frame_grey.cols, frame_grey.rows,
            frame_grey.cols, face, 1, pt, result);
        }

    if (ret != MCV_OK)
    {
        fprintf(stderr, "LRAlign error : %d", ret);
        return ;
    }

    for (int j= 0; j < 5; j++)
        landmark1[j] = Point2f(result[j + DEFAULT_ALIGNMENT_LANDMARKS - 5].x, result[j + DEFAULT_ALIGNMENT_LANDMARKS - 5].y);
    delete result;

    mcv_facesdk_release_frontal_result(pface, count);
}

float Generator::overlap(Rect2f a, Rect2f b)
{
    if (a.width <= 0.0f || a.height <= 0.0f || a.area() <= 0.0f)
        return 0.0f;
    if (b.width <= 0.0f || b.height <= 0.0f || b.area() <= 0.0f)
        return 0.0f;
    float s = (a & b).area();
    return s / (a.area() + b.area() - s);
}
