// EstVid.cpp: Este archivo contiene la funci?n "main". La ejecuci?n del programa comienza y termina ah?.

/*
1.- Encontrar la matriz "transformada" del frame previo al actual usando el flujo ?ptico de imagen para todos los frames. La transformada consiste en tres parametros: dx, dy , da (angulo). Es b?sicamente una transformada Euclideana r?gida, sin escalado.
2.- Acumular las matrices transformadas para obtener la "trayectoria" de x, y, angulo, para cada frame.
3.- Suavizar la trayectoria usando desplazamiento de ventana. El usuario puede definir el radio de suavizado, donde el radio ser? el numero de frames usados para suavizar.
4.- Crear una nueva matriz transformada con el resultado de nueva_transformada = transformada + (trayectoria_suavizada - trayectoria)
5.- Aplicar la nueva matriz a los frames del video.
6.- Crear el video de salida estabilizado.
*/


// IMPORTANTE: Usar el siguiente comando de ffmpeg para convertir los videos a un tama?o adecuado para procesarlo:
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

int main(int argc, char* argv[])
{
    if (argc < 5) {
        cout << "\nMissing parameters to load the program\n" << endl;
        return 0;
    }

    // Paso 0 - Creaci?n de variables. 
    string input = argv[1];
    string output = argv[2];
    int VALOR_SUAVIZADO = stoi(argv[3]); // En frames. Mayor valor, m?s estabilidad del video, pero menos reactivo a movimientos r?pidos.
    int ZOOM_IMAGEN = stoi(argv[4]); // En pixeles. Recorta la imagen para reducir los bordes negros resultantes de la estabilizaci?n (hace zoom en la imagen). Mayor valor, m?s zoom y menos borde negro.
    Mat cur, prev;
    Mat T(2, 3, CV_64F);
    int max_frames, fps, frame_width, frame_height;
    bool isDemo = false;
    bool isStats = false;

    for (int i = 5; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) { isDemo = true; }
        if (strcmp(argv[i], "-s") == 0) { isStats = true; }
    }

    if (CreateDirectory(L"output", NULL))
    {
        // Directorio creado
    }
    else if (ERROR_ALREADY_EXISTS == GetLastError())
    {
        cerr << "Directory /output already exists. Using it..." << endl;
    }
    else
    {
        cerr << "Cannot create the required directory /output." << endl;
    }

    if (CreateDirectory(L"stats", NULL))
    {
        // Directorio creado
    }
    else if (ERROR_ALREADY_EXISTS == GetLastError())
    {
        cerr << "Directory /stats already exists. Using it..." << endl;
    }
    else
    {
        cerr << "Cannot create the required directory /stats." << endl;
    }
    


    // Para analisis estadistico
    ofstream out_transform("stats/prev_to_cur_transformation.txt");
    ofstream out_trajectory("stats/trajectory.txt");
    ofstream out_smoothed_trajectory("stats/smoothed_trajectory.txt");
    ofstream out_new_transform("stats/new_prev_to_cur_transformation.txt");

    VideoCapture cap(input);
    // Compruebo que existe el video de entrada
    if (!cap.isOpened())
    {
        cerr << "\nCannot open the input file.\n" << endl;
        system("pause"); // WINDOWS ONLY

        return -1;
    }

    getVideoData(cap, max_frames, fps, frame_width, frame_height);

    showInfo(input, output, max_frames, fps, frame_width, frame_height, VALOR_SUAVIZADO, ZOOM_IMAGEN, isDemo, isStats);
    system("pause"); // WINDOWS ONLY

    // Paso 1 - Obtengo la transformada de los frames previos al frame actual, para todos los frames (dx, dy, da).    
    vector <ParamTransformada> prev_to_cur_transform = prev_cur_Transform(cap, cur, prev, T, prev_to_cur_transform, max_frames, isStats, out_transform);
        
    // Paso 2 - Acumulamos las transformadas para obtener la trayectoria     
    vector <Trayectoria> trajectory = accumulateTransform(prev_to_cur_transform, isStats, out_trajectory);

    // Paso 3 - Suavizamos la trayectoria usando una ventana deslizante usando los valores medios
    vector <Trayectoria> smoothed_trajectory = smoothTransform(trajectory, VALOR_SUAVIZADO, isStats, out_smoothed_trajectory);

    // Paso 4 - Generamos un nuevo set de frames previos al actual, el cual acabar? teniendo una trayectoria entre frames igual a la trayectoria suavizada.
    vector <ParamTransformada> new_prev_to_cur_transform = calcNewFrames(prev_to_cur_transform, smoothed_trajectory, isStats, out_new_transform);

    // Paso 5 - Aplicamos la nueva transformada al video y lo mostramos
    int wK_delay = int(1000 / fps);
    int aspectRatio = ZOOM_IMAGEN * prev.rows / prev.cols; // cogemos la relaci?n de aspecto correcta
    int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v');

    VideoWriter outVideo("./output/" + output, fourcc, fps, Size(frame_width, frame_height));
;

    // Compruebo que puedo crear el video de salida
    if (!outVideo.isOpened())
    {
        cerr << "\nCannot create the output file.\n" << endl;
        system("pause"); // WINDOWS ONLY

        return -1;
    }

    videoWrite(cap, outVideo, T, new_prev_to_cur_transform, aspectRatio, wK_delay, max_frames, ZOOM_IMAGEN);
    
    // Release resources
    cap.release();
    cur.release();
    prev.release();
    T.release();
    outVideo.release();

    if(isDemo) { 
        //waitKey(500);
        VideoCapture vid1(input);
        VideoCapture vid2("./output/" + output);
        VideoWriter demoVideo("./output/demo_" + output, fourcc, fps, Size(frame_width*2, frame_height));

        if (!demoVideo.isOpened())
        {
            cerr << "\nCannot create the demo file.\n" << endl;
            system("pause"); // WINDOWS ONLY

            return -1;
        }
        if (!vid1.isOpened() || !vid2.isOpened())
        {
            cerr << "\nCannot read the videos.\n" << endl;
            system("pause"); // WINDOWS ONLY

            return -1;
        }

        //int screen_width = GetSystemMetrics(SM_CXSCREEN);
        displayDemo(vid1, vid2, demoVideo, max_frames, wK_delay);

        vid1.release();
        vid2.release();
        demoVideo.release();
    }

    cout << "\n\n\nOperation completed successfully." << endl;
    if(isDemo){
        cout << "\nPlease, check the /output folder for the stabilised and comparison videos.\n\n" << endl;
    }
    else {
        cout << "\nPlease, check the /output folder for the stabilised video '" << output << "'\n\n" << endl;
    }

    destroyAllWindows();

    return 0;
}