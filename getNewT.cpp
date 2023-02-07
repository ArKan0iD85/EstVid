#include "estvid.h"

using namespace std;
using namespace cv;

vector <ParamTransformada> calcNewFrames(vector <ParamTransformada> original_trajectory, vector <Trayectoria> smoothed_trajectory, bool isStats, ofstream &out_new_transform) {
    // Transformada de acumulación de frame a frame
    double x = 0;
    double y = 0;
    double a = 0;
    vector <ParamTransformada> newFrames;

    for (size_t i = 0; i < original_trajectory.size(); i++) {
        x += original_trajectory[i].dx;
        y += original_trajectory[i].dy;
        a += original_trajectory[i].da;

        // target - current
        double diff_x = smoothed_trajectory[i].x - x;
        double diff_y = smoothed_trajectory[i].y - y;
        double diff_a = smoothed_trajectory[i].a - a;

        double dx = original_trajectory[i].dx + diff_x;
        double dy = original_trajectory[i].dy + diff_y;
        double da = original_trajectory[i].da + diff_a;

        newFrames.push_back(ParamTransformada(dx, dy, da));

        if (isStats) out_new_transform << (i + 1) << " " << dx << " " << dy << " " << da << endl; // Para analisis estadistico
    }
    return newFrames;
};