#include "estvid.h"

using namespace std;
using namespace cv;

int displayDemo(VideoCapture inVid, VideoCapture outVid, int maxframes, int delay) {
    Mat frame1, frame2, canvas;
    int k = 1;

    inVid >> frame1;
    outVid >> frame2;

    while (k < maxframes - 1) {
        canvas = Mat::zeros(frame1.rows, frame1.cols * 2 + 10, frame1.type());

        if (frame1.data == NULL || frame2.data == NULL) {
            break;
        }

        frame1.copyTo(canvas(Range::all(), Range(0, frame2.cols)));
        frame2.copyTo(canvas(Range::all(), Range(frame2.cols + 10, frame2.cols * 2 + 10)));

        // Si la ventana es demasiado grande para caber en la pantalla, la escalamos a la mitad. Debería ser suficiente.
        if (canvas.cols > 1480) {
            resize(canvas, canvas, Size(int(canvas.cols / 1.25), int(canvas.rows / 1.25)));
        }

        imshow("Before and after", canvas);
        waitKey(delay);

        inVid >> frame1;
        outVid >> frame2;

        k++;
    }

    canvas.release();
    return 0;
}