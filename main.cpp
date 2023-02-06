// EstVid.cpp: Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.

/*
1.- Encontrar la matriz "transformada" del frame previo al actual usando el flujo óptico de imagen para todos los frames. La transformada consiste en tres parametros: dx, dy , da (angulo). Es básicamente una transformada Euclideana rígida, sin escalado.
2.- Acumular las matrices transformadas para obtener la "trayectoria" de x, y, angulo, para cada frame.
3.- Suavizar la trayectoria usando desplazamiento de ventana. El usuario puede definir el radio de suavizado, donde el radio será el numero de frames usados para suavizar.
4.- Crear una nueva matriz transformada con el resultado de nueva_transformada = transformada + (trayectoria_suavizada - trayectoria)
5.- Aplicar la nueva matriz a los frames del video.
6.- Crear el video de salida estabilizado.
*/


// IMPORTANTE: Usar el siguiente comando de ffmpeg para convertir los videos a un tamaño adecuado para procesarlo:
// ffmpeg -i input.mp4 -vf scale=-1:720 -preset slow -crf 18 output.mp4

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

int main()
{
    bool mode = getMode();
    if (mode) { realStabilizator(); }
    else {
        // Para analisis estadistico
        ofstream out_transform("../../../stats/prev_to_cur_transformation.txt");
        ofstream out_trajectory("../../../stats/trajectory.txt");
        ofstream out_smoothed_trajectory("../../../stats/smoothed_trajectory.txt");
        ofstream out_new_transform("../../../stats/new_prev_to_cur_transformation.txt");

        // Paso 0 - Creación de variables. 
        int VALOR_SUAVIZADO = 0; // En frames. Mayor valor, más estabilidad del video, pero menos reactivo a movimientos rápidos.
        int ZOOM_IMAGEN = 0; // En pixeles. Recorta la imagen para reducir los bordes negros resultantes de la estabilización (hace zoom en la imagen). Mayor valor, más zoom y menos borde negro.

        string input, output, demo, stats;
        int max_frames, fps, frame_width, frame_height;
        bool isDemo = false;
        bool isStats = false;

        getVideoInfo(input, output, demo, stats);

        VideoCapture cap(input);
        // Compruebo que existe el video
        if (!cap.isOpened())
        {
            cout << "\nCannot open the file.\n" << endl;
            return -1;
        }

        getStabValues(VALOR_SUAVIZADO, ZOOM_IMAGEN);
        getVideoData(cap, max_frames, fps, frame_width, frame_height);

        if (demo == "y" || demo == "Y") isDemo = true;
        if (stats == "y" || stats == "Y") isStats = true;

        showInfo(input, output, max_frames, fps, frame_width, frame_height, VALOR_SUAVIZADO, ZOOM_IMAGEN, isDemo);
        system("pause"); // WINDOWS ONLY

        // Paso 1 - Obtengo la transformada de los frames previos al frame actual, para todos los frames (dx, dy, da).    
        //vector <ParamTransformada> prev_to_cur_transform = prev_cur_Transform(cap, prev_to_cur_transform, max_frames, isStats, out_transform);

        Mat cur, cur_grey, cur2, prev, prev_grey, last_T;
        Mat T(2, 3, CV_64F);
        vector <ParamTransformada> prev_to_cur_transform;
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

            prev_to_cur_transform.push_back(ParamTransformada(dx, dy, da));

            if (isStats) out_transform << k << " " << dx << " " << dy << " " << da << endl; // Para analisis estadistico

            cur.copyTo(prev);
            cur_grey.copyTo(prev_grey);

            progress = (k * 100 / max_frames);
            cout << "\r" << setw(-10) << printBarProg(progress) << " " << progress + 1 << "% completed." << flush;

            k++;
        }

        // Paso 2 - Acumulamos las transformadas para obtener la trayectoria     
        vector <Trayectoria> trajectory = accumulateTransform(prev_to_cur_transform, isStats, out_trajectory);

        // Paso 3 - Suavizamos la trayectoria usando una ventana deslizante usando los valores medios
        vector <Trayectoria> smoothed_trajectory = smoothTransform(trajectory, VALOR_SUAVIZADO, isStats, out_smoothed_trajectory); // vector de trayectoria suavizada para todos los frames

        // Paso 4 - Generamos un nuevo set de frames previos al actual, el cual acabará teniendo una trayectoria entre frames igual a la trayectoria suavizada.
        vector <ParamTransformada> new_prev_to_cur_transform = calcNewFrames(prev_to_cur_transform, smoothed_trajectory, isStats, out_new_transform);

        // Paso 5 - Aplicamos la nueva transformada al video y lo mostramos
        //videoWrite(cap, output, new_prev_to_cur_transform, frame_width, frame_height, fps, max_frames, ZOOM_IMAGEN, isDemo);

        cap.set(CAP_PROP_POS_FRAMES, 0);
        Mat canvas;
        int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v');
        int wK_delay = int(1000 / fps);
        int vert_border = ZOOM_IMAGEN * prev.rows / prev.cols; // cogemos la relación de aspecto correcta

        k = 0;

        VideoWriter outVideo(output, fourcc, fps, Size(frame_width, frame_height));
        ofstream matrizImagen("../../stats/matrizImagen.txt"); //prueba

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

            cur2 = cur2(Range(vert_border, cur2.rows - vert_border), Range(ZOOM_IMAGEN, cur2.cols - ZOOM_IMAGEN));

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

        cap.release();
        cur.release();
        cur2.release();
        outVideo.release();
        destroyAllWindows();
    }
    return 0;
}