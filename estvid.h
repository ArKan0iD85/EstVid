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

bool getMode();

int realStabilizator();

string printBarProg(int x);

string getVideoInfo(string &in, string &out, string &demo, string &stats);

int getStabValues(int &suavizado, int &zoom);

int getVideoData(VideoCapture video, int &max_frames, int &fps, int &frame_width, int &frame_height);

void showInfo(string in, string out, int max, int fps, int width, int height, int suavizado, int zoom, bool demo);

vector <ParamTransformada> prev_cur_Transform(VideoCapture cap, Mat &cur, Mat  &prev, Mat &T, vector <ParamTransformada> tr, int max_frames, bool isStats, ofstream &out_transform);

vector <Trayectoria> accumulateTransform(vector <ParamTransformada> traj, bool isStats, ofstream& out_trajectory);

vector <Trayectoria> smoothTransform(vector <Trayectoria> trajectory, int suavizado, bool isStats, ofstream &out_smoothed_trajectory);

vector <ParamTransformada> calcNewFrames(vector <ParamTransformada> original_trajectory, vector <Trayectoria> smoothed_trajectory, bool isStats, ofstream &out_new_transform);

void videoWrite(VideoCapture inVid, VideoWriter outVid, Mat T, vector <ParamTransformada> newTrans, int aspectRatio, int delay, int maxFrames, int zoom, bool demo);

#endif // !ESTVID_H