#include "estvid.h"

using namespace std;
using namespace cv;

vector <Trayectoria> smoother(vector <Trayectoria> trajectory, int suavizado, bool isStats, ofstream &out_smoothed_trajectory) {
    vector <Trayectoria> smoothed_trajectory;

    for (size_t i = 0; i < trajectory.size(); i++) {
        double sum_x = 0;
        double sum_y = 0;
        double sum_a = 0;
        int count = 0;

        for (int j = -suavizado; j <= suavizado; j++) {
            if (i + j >= 0 && i + j < trajectory.size()) {
                sum_x += trajectory[i + j].x;
                sum_y += trajectory[i + j].y;
                sum_a += trajectory[i + j].a;

                count++;
            }
        }

        double avg_a = sum_a / count;
        double avg_x = sum_x / count;
        double avg_y = sum_y / count;

        smoothed_trajectory.push_back(Trayectoria(avg_x, avg_y, avg_a));

        if (isStats) out_smoothed_trajectory << (i + 1) << " " << avg_x << " " << avg_y << " " << avg_a << endl; // Para analisis estadistico
    }
    return smoothed_trajectory;
};