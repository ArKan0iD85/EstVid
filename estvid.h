#ifndef ESTVID_H
#define ESTVID_H

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <fstream>

using namespace std;
using namespace cv;

struct ParamTransformada
{
    ParamTransformada() {}
    ParamTransformada(double _dx, double _dy, double _da) {
        dx = _dx;
        dy = _dy;
        da = _da;
    }

    double dx = 0;
    double dy = 0;
    double da = 0; // angulo
};

struct Trayectoria
{
    Trayectoria() {}
    Trayectoria(double _x, double _y, double _a) {
        x = _x;
        y = _y;
        a = _a;
    }

    double x = 0;
    double y = 0;
    double a = 0; // angulo
};

string printBarProg(int x);

string getVideoInfo(string &in, string &out, string &demo, string &stats);

int getUserValues(int &suavizado, int &zoom);

int getVideoData(VideoCapture video, int &max_frames, int &fps, int &frame_width, int &frame_height);

void showInfo(string in, string out, int max, int fps, int width, int height, int suavizado, int zoom, bool demo);

vector <ParamTransformada> prev_cur_Transform(VideoCapture cap, vector <ParamTransformada> tr, int border, int max_frames, bool isStats, ofstream &out_transform);

vector <Trayectoria> accumulateTransform(vector <ParamTransformada> traj, bool isStats, ofstream& out_trajectory);

vector <Trayectoria> smoother(vector <Trayectoria> trajj, int suavizado, bool isStats, ofstream &out_smoothed_trajectory);

#endif // !ESTVID_H