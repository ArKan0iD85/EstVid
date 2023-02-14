#include "estvid.h"

using namespace std;
using namespace cv;

int displayDemo(VideoCapture inVid, VideoCapture outVid, VideoWriter demoVid, int maxframes, int delay) {
    Mat frame1, frame2, canvas;
    int k = 0;
    int progress = 0;

    cout << "\n\nCreating comparison video..." << endl;
    while (k < maxframes - 1) {
        inVid >> frame1;
        outVid >> frame2;
        
        canvas = Mat::zeros(frame1.rows, frame1.cols * 2, frame1.type());

        if (frame1.data == NULL || frame2.data == NULL) {
            break;
        }

        frame1.copyTo(canvas(Range::all(), Range(0, frame1.cols)));
        frame2.copyTo(canvas(Range::all(), Range(frame1.cols, frame1.cols * 2)));

        demoVid.write(canvas);

        progress = (k * 100 / maxframes);
        cout << "\r" << setw(-10) << printBarProg(progress) << " " << progress + 1 << "% completed." << flush;
        k++;
    }

    demoVid.release();
    canvas.release();
    return 0;
}