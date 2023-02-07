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
        Mat cur, prev;
        Mat T(2, 3, CV_64F);
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
        vector <ParamTransformada> prev_to_cur_transform = prev_cur_Transform(cap, cur, prev, T, prev_to_cur_transform, max_frames, isStats, out_transform);
        
        // Paso 2 - Acumulamos las transformadas para obtener la trayectoria     
        vector <Trayectoria> trajectory = accumulateTransform(prev_to_cur_transform, isStats, out_trajectory);

        // Paso 3 - Suavizamos la trayectoria usando una ventana deslizante usando los valores medios
        vector <Trayectoria> smoothed_trajectory = smoothTransform(trajectory, VALOR_SUAVIZADO, isStats, out_smoothed_trajectory);

        // Paso 4 - Generamos un nuevo set de frames previos al actual, el cual acabará teniendo una trayectoria entre frames igual a la trayectoria suavizada.
        vector <ParamTransformada> new_prev_to_cur_transform = calcNewFrames(prev_to_cur_transform, smoothed_trajectory, isStats, out_new_transform);

        // Paso 5 - Aplicamos la nueva transformada al video y lo mostramos
        int wK_delay = int(1000 / fps);
        int aspectRatio = ZOOM_IMAGEN * prev.rows / prev.cols; // cogemos la relación de aspecto correcta

        int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v');
        VideoWriter outVideo(output, fourcc, fps, Size(frame_width, frame_height));

        videoWrite(cap, outVideo, T, new_prev_to_cur_transform, aspectRatio, wK_delay, max_frames, ZOOM_IMAGEN, isDemo);
        
        // Release resources
        cap.release();
        cur.release();
        prev.release();
        T.release();
        outVideo.release();
        destroyAllWindows();
        
    }

    return 0;
}