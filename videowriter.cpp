#include "estvid.h"

using namespace std;
using namespace cv;

void videoWrite(VideoCapture cap, VideoWriter outVideo, Mat T, vector <ParamTransformada> new_prev_to_cur_transform, int aspectRatio, int wK_delay, int max_frames, int ZOOM_IMAGEN) {
    cap.set(CAP_PROP_POS_FRAMES, 0);
    Mat cur, cur2, canvas;
    int progress = 0;
    int k = 0;

    cout << "\n\nCreating stabilised video..." << endl;
    while (k < max_frames - 1) { // no procesamos el último frame, ya que no tiene una transformada de trayectoria válida
        cap >> cur;

        if (cur.data == NULL) {
            break;
        }

        T.at<double>(0, 0) = cos(new_prev_to_cur_transform[k].da);
        T.at<double>(0, 1) = -sin(new_prev_to_cur_transform[k].da);
        T.at<double>(1, 0) = sin(new_prev_to_cur_transform[k].da);
        T.at<double>(1, 1) = cos(new_prev_to_cur_transform[k].da);

        T.at<double>(0, 2) = new_prev_to_cur_transform[k].dx;
        T.at<double>(1, 2) = new_prev_to_cur_transform[k].dy;

        warpAffine(cur, cur2, T, cur.size());

        cur2 = cur2(Range(aspectRatio, cur2.rows - aspectRatio), Range(ZOOM_IMAGEN, cur2.cols - ZOOM_IMAGEN));

        // Igualamos el tamaño de cur2 al del original cur, para facilitar la comparación
        resize(cur2, cur2, cur.size());

        outVideo.write(cur2);

        progress = (k * 100 / max_frames);
        cout << "\r" << setw(-10) << printBarProg(progress) << " " << progress + 1 << "% completed." << flush;
        k++;
    }

    cur.release();
    cur2.release();
    canvas.release();
};