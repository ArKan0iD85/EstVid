#include "estvid.h"

using namespace std;
using namespace cv;

vector <ParamTransformada> prev_cur_Transform(VideoCapture cap, Mat &cur, Mat &prev, Mat &T, vector <ParamTransformada> vector_transform, int max_frames, bool isStats, ofstream& out_transform) {
    Mat cur_grey, prev_grey, last_T;
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

        cvtColor(cur, cur_grey, COLOR_BGR2GRAY); // Transformamos las imagenes a gris para facilitar la detecci?n de esquinas y flujo de movimiento

        // Vector de previo a actual
        vector <Point2f> prev_corner, cur_corner;
        vector <Point2f> good_prev_corner, good_cur_corner;
        vector <uchar> status;
        vector <float> err;

        // Algoritmo "Shi - Tomasi Corner Detector" (mejora del "Harris Corner Detector")
        goodFeaturesToTrack(prev_grey, prev_corner, 500, 0.01, 30);

        // Algoritmo Lucas-Kanade para la detecci?n de movimiento de un objeto
        calcOpticalFlowPyrLK(prev_grey, cur_grey, prev_corner, cur_corner, status, err);

        // Limpiamos las malas correspondencias
        for (size_t i = 0; i < status.size(); i++) {
            if (status[i]) {
                good_prev_corner.push_back(prev_corner[i]);
                good_cur_corner.push_back(cur_corner[i]);
            }
        }

        // solo traslaci?n y rotaci?n
        T = estimateAffinePartial2D(good_prev_corner, good_cur_corner);

        // En algunos casos extra?os, no se encuentra la transformada. Usaremos la ?ltima correcta.
        if (T.data == NULL) {
            last_T.copyTo(T);
        }

        T.copyTo(last_T);

        // descomponemos T
        double dx = T.at<double>(0, 2);
        double dy = T.at<double>(1, 2);
        double da = atan2(T.at<double>(1, 0), T.at<double>(0, 0));

        vector_transform.push_back(ParamTransformada(dx, dy, da));

        if (isStats) out_transform << k << " " << dx << " " << dy << " " << da << endl; // Para analisis estadistico

        cur.copyTo(prev);
        cur_grey.copyTo(prev_grey);

        progress = (k * 100 / max_frames);
        cout << "\r" << setw(-10) << printBarProg(progress) << " " << progress + 1 << "% completed." << flush;

        k++;
    }

    out_transform.close();
    cur_grey.release();
    prev_grey.release();
    last_T.release();

    return vector_transform;
};