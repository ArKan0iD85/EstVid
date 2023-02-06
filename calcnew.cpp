#include "estvid.h"

using namespace std;
using namespace cv;

vector <ParamTransformada> calcNewFrames(vector <ParamTransformada> prev, vector <Trayectoria> smoothTraj, bool isStats, ofstream &out_new_transform) {
    // Transformada de acumulación de frame a frame
    double x = 0;
    double y = 0;
    double a = 0;
    vector <ParamTransformada> newFrames;

    for (size_t i = 0; i < prev.size(); i++) {
        x += prev[i].dx;
        y += prev[i].dy;
        a += prev[i].da;

        // target - current
        double diff_x = smoothTraj[i].x - x;
        double diff_y = smoothTraj[i].y - y;
        double diff_a = smoothTraj[i].a - a;

        double dx = prev[i].dx + diff_x;
        double dy = prev[i].dy + diff_y;
        double da = prev[i].da + diff_a;

        newFrames.push_back(ParamTransformada(dx, dy, da));

        if (isStats) out_new_transform << (i + 1) << " " << dx << " " << dy << " " << da << endl; // Para analisis estadistico
    }
    return newFrames;
};

vector <ParamTransformada> calcNewFrames(vector <ParamTransformada> prev, vector <Trayectoria> smoothTraj) {
    // Transformada de acumulación de frame a frame
    double x = 0;
    double y = 0;
    double a = 0;
    vector <ParamTransformada> newFrames;

    for (size_t i = 0; i < prev.size(); i++) {
        x += prev[i].dx;
        y += prev[i].dy;
        a += prev[i].da;

        // target - current
        double diff_x = smoothTraj[i].x - x;
        double diff_y = smoothTraj[i].y - y;
        double diff_a = smoothTraj[i].a - a;

        double dx = prev[i].dx + diff_x;
        double dy = prev[i].dy + diff_y;
        double da = prev[i].da + diff_a;

        newFrames.push_back(ParamTransformada(dx, dy, da));

    }
    return newFrames;
};