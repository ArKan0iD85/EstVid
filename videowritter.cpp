#include "estvid.h"

using namespace std;
using namespace cv;

void videoWrite(VideoCapture cap, VideoWriter outVideo, Mat T, int aspectRatio, int wK_delay, vector <ParamTransformada> new_prev_to_cur_transform, int max_frames, int ZOOM_IMAGEN, bool isDemo) {
    cap.set(CAP_PROP_POS_FRAMES, 0);
    Mat cur, cur2, canvas;
    int progress = 0;
    int k = 0;

    cout << "\nCreating stabilised video..." << endl;
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

        // Mostramos el video original y el estabilizado a la vez en una ventana, si el usuario lo pide.
        if (isDemo) {
            canvas = Mat::zeros(cur.rows, cur.cols * 2 + 10, cur.type());

            cur.copyTo(canvas(Range::all(), Range(0, cur2.cols)));
            cur2.copyTo(canvas(Range::all(), Range(cur2.cols + 10, cur2.cols * 2 + 10)));

            // Si la ventana es demasiado grande para caber en la pantalla, la escalamos a la mitad. Debería ser suficiente.
            if (canvas.cols > 1460) {
                resize(canvas, canvas, Size(int(canvas.cols / 1.25), int(canvas.rows / 1.25)));
            }

            imshow("Before and after", canvas);
            waitKey(wK_delay);
        }

        progress = (k * 100 / max_frames);
        cout << "\r" << setw(-10) << printBarProg(progress) << " " << progress + 1 << "% completed." << flush;
        k++;
    }
    cout << "\nOperation completed successfully.\n" << endl;
    cout << "\nPlease check the folder \\output for the stabilised video.\n\n" << endl;

    cur2.release();
    canvas.release();
};