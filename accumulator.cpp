#include "estvid.h"

using namespace std;
using namespace cv;

vector <Trayectoria> accumulateTransform(vector <ParamTransformada> traj, bool isStats, ofstream& out_trajectory) {
    // Transformada de acumulación frame a frame
    double x = 0;
    double y = 0;
    double a = 0;

    vector <Trayectoria> trajectory; // vector de trayectoria para todos los frames

    for (size_t i = 0; i < traj.size(); i++) {
        x += traj[i].dx;
        y += traj[i].dy;
        a += traj[i].da;

        trajectory.push_back(Trayectoria(x, y, a));

        if (isStats) out_trajectory << (i + 1) << " " << x << " " << y << " " << a << endl; // Para analisis estadistico
    }
    return trajectory;
};