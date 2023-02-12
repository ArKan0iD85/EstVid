#include "estvid.h"

using namespace std;
using namespace cv;

int getVideoData(VideoCapture video, int &max_frames, int &fps, int &frame_width, int &frame_height) {
    fps = int(video.get(CAP_PROP_FPS));
    frame_width = int(video.get(CAP_PROP_FRAME_WIDTH));
    frame_height = int(video.get(CAP_PROP_FRAME_HEIGHT));
    int video_length = int(video.get(CAP_PROP_FRAME_COUNT));

    //Pruebo método rapido para calcular max_frames. Si no funciona, uso el método lento.
    //Método rápido: Usando comando get(CAP_PROP_FRAME_COUNT) de OpenCV. Lee los metadatos del video y devuelve el numero de frames.
    if (video_length > 0) {
        max_frames = video_length;
    }
    else {
        //Metodo lento: Leo rapidamente el video y cuento los frames. Guardo el numero en el contador.
        Mat frame;

        while (video.isOpened()) {
            // Contar frame a frame mientras dure el video.
            video >> frame;
            max_frames++;
            // Si el frame está vacio, parar inmediatamente. video roto.
            if (frame.empty())
                cout << "Error. Broken video." << endl;
            cout << "Frame number " << max_frames << " is empty." << endl;
            cout << "--------------------------------------------------\n\n" << endl;
            break;
        }
        frame.release();
    }
    video.release();

    return max_frames, fps, frame_width, frame_height;
};

void showInfo(string in, string out, int max, int fps, int width, int height, int suavizado, int zoom, bool demo, bool stats) {
    cout << "\n\n" << endl;
    cout << "Video paths" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Input video: " << in << endl;
    cout << "Output video: " << out << endl;
    cout << "--------------------------------------------------\n\n" << endl;

    cout << "Input video info" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Frames per second: " << fps << " fps" << endl;
    cout << "Video width: " << width << " pixels" << endl;
    cout << "Video height: " << height << " pixels" << endl;
    cout << "Number of frames to process: " << max << " frames" << endl;
    cout << "--------------------------------------------------\n\n" << endl;

    cout << "User values" << endl;
    cout << "----------------------------------------------------" << endl;
    cout << "Smoothing value: " << suavizado << " frames" << endl;
    cout << "Image zoom value: " << zoom << " pixels" << endl;
    if (demo) { cout << "Side-by-side demonstration: Y" << endl; }
    else { cout << "Side-by-side demonstration: N" << endl; };
    if (stats) { cout << "Output statistical data: Y" << endl; }
    else { cout << "Output statistical data: N" << endl; };
    cout << "--------------------------------------------------\n\n" << endl;
};

string printBarProg(int x) {
    string s;
    s = "[";
    for (int i = 1; i <= 50; i++) {
        if (i <= (x / 2) - 1 || x == 99)
            s += "=";
        else if (i == (x / 2))
            s += ">";
        else
            s += " ";
    }
    s += "]";
    return s;
};