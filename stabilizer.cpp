#include "estvid.h"

using namespace std;
using namespace cv;

vector <ParamTransformada> prev_cur_Transform(VideoCapture cap, Mat &cur, Mat &prev, Mat &T, vector <ParamTransformada> tr, int max_frames, bool isStats, ofstream& out_transform) {
    Mat cur_grey, cur2, prev_grey, last_T;
    int k = 1;
    int progress = 0;

    cap >> prev;
    cvtColor(prev, prev_grey, COLOR_BGR2GRAY);

    cout << "\nDecomposing input video and calculating smoothed trajectory..." << endl;

    while (true) {
        cap >> cur;

        if (cur.data == NULL) {
            break;
        }

        cvtColor(cur, cur_grey, COLOR_BGR2GRAY); // Transformamos las imagenes a gris para facilitar la detección de esquinas y flujo de movimiento

        // Vector de previo a actual
        vector <Point2f> prev_corner, cur_corner;
        vector <Point2f> prev_corner2, cur_corner2;
        vector <uchar> status;
        vector <float> err;

        // Algoritmo "Shi - Tomasi Corner Detector" (mejora del "Harris Corner Detector")
        goodFeaturesToTrack(prev_grey, prev_corner, 500, 0.01, 30);

        // Algoritmo Lucas-Kanade para la detección de movimiento de un objeto
        calcOpticalFlowPyrLK(prev_grey, cur_grey, prev_corner, cur_corner, status, err);

        // Limpiamos las malas correspondencias
        for (size_t i = 0; i < status.size(); i++) {
            if (status[i]) {
                prev_corner2.push_back(prev_corner[i]);
                cur_corner2.push_back(cur_corner[i]);
            }
        }

        // solo traslación y rotación
        T = estimateAffinePartial2D(prev_corner2, cur_corner2);

        // En algunos casos extraños, no se encuentra la transformada. Usaremos la última correcta.
        if (T.data == NULL) {
            last_T.copyTo(T);
        }

        T.copyTo(last_T);

        // descomponemos T
        double dx = T.at<double>(0, 2);
        double dy = T.at<double>(1, 2);
        double da = atan2(T.at<double>(1, 0), T.at<double>(0, 0));

        tr.push_back(ParamTransformada(dx, dy, da));

        if (isStats) out_transform << k << " " << dx << " " << dy << " " << da << endl; // Para analisis estadistico

        cur.copyTo(prev);
        cur_grey.copyTo(prev_grey);

        progress = (k * 100 / max_frames);
        cout << "\r" << setw(-10) << printBarProg(progress) << " " << progress + 1 << "% completed." << flush;

        k++;
    }

    last_T.release();

    return tr;
};

vector <ParamTransformada> prev_cur_Transform(VideoCapture cap, vector <ParamTransformada> tr) {
    Mat cur, cur_grey, cur2, prev, prev_grey, last_T;
    Mat T(2, 3, CV_64F);
    int k = 1;
    int progress = 0;

    while (true) {
        cap >> cur;

        if (cur.data == NULL) {
            break;
        }

        cvtColor(cur, cur_grey, COLOR_BGR2GRAY); // Transformamos las imagenes a gris para facilitar la detección de esquinas y flujo de movimiento

        // Vector de previo a actual
        vector <Point2f> prev_corner, cur_corner;
        vector <Point2f> prev_corner2, cur_corner2;
        vector <uchar> status;
        vector <float> err;

        // Algoritmo "Shi - Tomasi Corner Detector" (mejora del "Harris Corner Detector")
        goodFeaturesToTrack(prev_grey, prev_corner, 500, 0.01, 30);

        // Algoritmo Lucas-Kanade para la detección de movimiento de un objeto
        calcOpticalFlowPyrLK(prev_grey, cur_grey, prev_corner, cur_corner, status, err);

        // Limpiamos las malas correspondencias
        for (size_t i = 0; i < status.size(); i++) {
            if (status[i]) {
                prev_corner2.push_back(prev_corner[i]);
                cur_corner2.push_back(cur_corner[i]);
            }
        }

        // solo traslación y rotación
        T = estimateAffinePartial2D(prev_corner2, cur_corner2);

        // En algunos casos extraños, no se encuentra la transformada. Usaremos la última correcta.
        if (T.data == NULL) {
            last_T.copyTo(T);
        }

        T.copyTo(last_T);

        // descomponemos T
        double dx = T.at<double>(0, 2);
        double dy = T.at<double>(1, 2);
        double da = atan2(T.at<double>(1, 0), T.at<double>(0, 0));

        tr.push_back(ParamTransformada(dx, dy, da));

        cur.copyTo(prev);
        cur_grey.copyTo(prev_grey);

        k++;
    }

    cur.release();
    cur2.release();
    cur_grey.release();
    prev.release();
    prev_grey.release();
    T.release();
    last_T.release();

    return tr;
};