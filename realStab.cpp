#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <Windows.h>
#include <cmath>
#include <fstream>
#include "estvid.h"

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

using namespace std;
using namespace cv;

int realStabilizator() {
    ofstream out_transform("../../../stats/prev_to_cur_transformation.txt");
    ofstream out_trajectory("../../../stats/trajectory.txt");
    ofstream out_smoothed_trajectory("../../../stats/smoothed_trajectory.txt");
    ofstream out_new_transform("../../../stats/new_prev_to_cur_transformation.txt");
        
    // User variables
    int VALOR_SUAVIZADO = 50;
    int ZOOM_IMAGEN = 50;
    bool isStat = false;
    bool writeVideo = false;

    // Other variables
    VideoWriter outVideo;
    Mat frame1, frame2, frameOut, gray1, gray2, last_T, canvas;
    Mat T(2, 3, CV_64F);
    bool cont = true;
    int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v');

    VideoCapture cap(0);
    int frame_width = int(cap.get(CAP_PROP_FRAME_WIDTH));
    int frame_height = int(cap.get(CAP_PROP_FRAME_HEIGHT));

    cout << "\n\nInput video info" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Video width: " << frame_width << " pixels" << endl;
    cout << "Video height: " << frame_height << " pixels" << endl;
    cout << "--------------------------------------------------" << endl;

    cout << "\n\nPress ESCAPE key to stop the streaming...\n" << endl;

    cap >> frame1;
    cvtColor(frame1, gray1, COLOR_BGR2GRAY);
    
    int aspectRatio = ZOOM_IMAGEN * frame1.rows / frame1.cols;
    if (writeVideo) outVideo.open("../../../output/realOutput.mp4", fourcc, 30, frame1.size());

    vector <ParamTransformada> prev_to_cur_transform;

    int k = 1;

    while (cont) {
        cap >> frame2;
        if (frame1.data == NULL) {
            break;
        }
        cvtColor(frame2, gray2, COLOR_BGR2GRAY); // Transformamos las imagenes a gris para facilitar la detección de esquinas y flujo de movimiento

        // Vector de previo a actual
        vector <Point2f> prev_corner, cur_corner;
        vector <Point2f> prev_corner2, cur_corner2;
        vector <uchar> status;
        vector <float> err;
        // Algoritmo "Shi - Tomasi Corner Detector" (mejora del "Harris Corner Detector")
        goodFeaturesToTrack(gray1, prev_corner, 500, 0.01, 30);
        // Algoritmo Lucas-Kanade para la detección de movimiento de un objeto

        calcOpticalFlowPyrLK(gray1, gray2, prev_corner, cur_corner, status, err);

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

        prev_to_cur_transform.push_back(ParamTransformada(dx, dy, da));

        frame2.copyTo(frame1);
        gray2.copyTo(gray1);

        if (isStat) out_transform << k << " " << dx << " " << dy << " " << da << endl;
        k++;


        // Transformada de acumulación de frame a frame
        double a = 0;
        double x = 0;
        double y = 0;

        vector <Trayectoria> trajectory; // vector de trayectoria para todos los frames

        for (size_t i = 0; i < prev_to_cur_transform.size(); i++) {
            x += prev_to_cur_transform[i].dx;
            y += prev_to_cur_transform[i].dy;
            a += prev_to_cur_transform[i].da;

            trajectory.push_back(Trayectoria(x, y, a));

            if (isStat) out_trajectory << (i + 1) << " " << x << " " << y << " " << a << endl;
        }

        // Paso 3 - Suavizamos la trayectoria usando una ventana deslizante usando los valores medios
        vector <Trayectoria> smoothed_trajectory; // vector de trayectoria suavizada para todos los frames

        for (size_t i = 0; i < trajectory.size(); i++) {
            double sum_x = 0;
            double sum_y = 0;
            double sum_a = 0;
            int count = 0;

            for (int j = -VALOR_SUAVIZADO; j <= VALOR_SUAVIZADO; j++) {
                if (i + j >= 0 && i + j < trajectory.size()) {
                    sum_x += trajectory[i + j].x;
                    sum_y += trajectory[i + j].y;
                    sum_a += trajectory[i + j].a;

                    count++;
                }
            }

            double avg_a = sum_a / count;
            double avg_x = sum_x / count;
            double avg_y = sum_y / count;

            smoothed_trajectory.push_back(Trayectoria(avg_x, avg_y, avg_a));

            if (isStat) out_smoothed_trajectory << (i + 1) << " " << avg_x << " " << avg_y << " " << avg_a << endl;
        }

        // Paso 4 - Generamos un nuevo set de frames previos al actual, el cual acabará teniendo una trayectoria entre frames igual a la trayectoria suavizada.
        vector <ParamTransformada> new_prev_to_cur_transform;

        // Transformada de acumulación de frame a frame
        a = 0;
        x = 0;
        y = 0;

        for (size_t i = 0; i < prev_to_cur_transform.size(); i++) {
            x += prev_to_cur_transform[i].dx;
            y += prev_to_cur_transform[i].dy;
            a += prev_to_cur_transform[i].da;

            // target - current
            double diff_x = smoothed_trajectory[i].x - x;
            double diff_y = smoothed_trajectory[i].y - y;
            double diff_a = smoothed_trajectory[i].a - a;

            double dx = prev_to_cur_transform[i].dx + diff_x;
            double dy = prev_to_cur_transform[i].dy + diff_y;
            double da = prev_to_cur_transform[i].da + diff_a;

            new_prev_to_cur_transform.push_back(ParamTransformada(dx, dy, da));

            if (isStat) out_new_transform << (i + 1) << " " << dx << " " << dy << " " << da << endl;
        }
        
        T.at<double>(0, 0) = cos(new_prev_to_cur_transform[k].da);
        T.at<double>(0, 1) = -sin(new_prev_to_cur_transform[k].da);
        T.at<double>(1, 0) = sin(new_prev_to_cur_transform[k].da);
        T.at<double>(1, 1) = cos(new_prev_to_cur_transform[k].da);

        T.at<double>(0, 2) = new_prev_to_cur_transform[k].dx;
        T.at<double>(1, 2) = new_prev_to_cur_transform[k].dy;

        warpAffine(frame2, frameOut, T, frame2.size());

        frameOut = frameOut(Range(aspectRatio, frameOut.rows - aspectRatio), Range(ZOOM_IMAGEN, frameOut.cols - ZOOM_IMAGEN));

        // Igualamos el tamaño de cur2 al del original cur, para facilitar la comparación
        resize(frameOut, frameOut, frame2.size());

        if (writeVideo) outVideo.write(frameOut);

        canvas = Mat::zeros(frame2.rows, frame2.cols * 2 + 10, frame2.type());

        frame2.copyTo(canvas(Range::all(), Range(0, frameOut.cols)));
        frameOut.copyTo(canvas(Range::all(), Range(frameOut.cols + 10, frameOut.cols * 2 + 10)));

        // Si la ventana es demasiado grande para caber en la pantalla, la escalamos a la mitad. Debería ser suficiente.
        if (canvas.cols > 1460) {
            resize(canvas, canvas, Size(int(canvas.cols / 1.25), int(canvas.rows / 1.25)));
        }

        imshow("Original vs stabilized", canvas);
        waitKey(1);

        if (GetAsyncKeyState(VK_ESCAPE)) cont = false;
    }

    cap.release();
    frame1.release();
    frame2.release();
    gray1.release();
    gray2.release();
    outVideo.release();
    canvas.release();
    destroyAllWindows();

    return 0;
};