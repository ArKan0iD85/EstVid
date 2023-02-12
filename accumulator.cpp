#include "estvid.h"

using namespace std;
using namespace cv;

vector <Trayectoria> accumulateTransform(vector <ParamTransformada> vector_transform, bool isStats, ofstream& out_trajectory) {
    // Transformada de acumulación frame a frame
    double x = 0;
    double y = 0;
    double a = 0;

    vector <Trayectoria> trajectory; // vector de trayectoria para todos los frames

    for (size_t i = 0; i < vector_transform.size(); i++) {
        x += vector_transform[i].dx;
        y += vector_transform[i].dy;
        a += vector_transform[i].da;

        trajectory.push_back(Trayectoria(x, y, a));

        if (isStats) out_trajectory << (i + 1) << " " << x << " " << y << " " << a << endl; // Para analisis estadistico
    }
    
    out_trajectory.close();

    return trajectory;
};