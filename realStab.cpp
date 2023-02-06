#include <Windows.h>
#include "estvid.h"

using namespace std;
using namespace cv;

void realStabilizator() {
    VideoCapture cap(0);
    Mat frame1, frame2, gray1, gray2;
    Mat smoothedMat(2, 3, CV_64F);
    bool cont = true;
    int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v');
    int vert_border = 0;

    cout << "\n\nPress ESCAPE key to stop the streaming...\n" << endl;

    cap >> frame1;
    cvtColor(frame1, gray1, COLOR_BGR2GRAY);

    VideoWriter outVideo;
    outVideo.open("realOutput.mp4", fourcc, 30, frame1.size());

    while (cont) {
        cap >> frame2;
        if (frame2.data == NULL) { break; };

        cvtColor(frame2, gray2, COLOR_BGR2GRAY);

        Mat smoothFrame;

        vector <ParamTransformada> prev_to_cur_transform = prev_cur_Transform(cap, prev_to_cur_transform);
        vector <Trayectoria> trajectory = accumulateTransform(prev_to_cur_transform);
        vector <Trayectoria> smoothed_trajectory = smoothTransform(trajectory);

        //smoothFrame = realTStab(frame1, frame2);

        outVideo.write(smoothFrame);

        imshow("Stabilized cam", smoothFrame);
        waitKey(1);

        frame1 = frame2;
        gray2.copyTo(gray1);

        if (GetAsyncKeyState(VK_ESCAPE)) cont = false;
    }

    cap.release();
    destroyAllWindows();

};