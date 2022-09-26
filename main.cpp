#include <iostream>
#include <fstream>
#include <vector>
#include <cfloat>
#include <math.h>

using namespace std;

struct vec3
{
    vec3(double inp)
    {
        xyz[0] = inp;
        xyz[1] = inp;
        xyz[2] = inp;
    }
    vec3(double _x,double _y,double _z)
    {
        xyz[0] = _x;
        xyz[1] = _y;
        xyz[2] = _z;
    }
    double operator [] (int i) const { return xyz[i]; }
    double& operator [] (int i) { return xyz[i]; }
private:
    double xyz[3];
};

double diff[3] {0.f, 0.f, 0.f};

void lesFil(const std::string& filePath, std::vector <vec3>& points)
{
//    points.reserve(8637772);
    std::ifstream file;
    file.open(filePath);

    float min[3] =    {  FLT_MAX,  FLT_MAX,  FLT_MAX };
    float max[3] =    { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    if (file.is_open())
    {
        float temp[3];
        while (true)
        {
            //Henter ut x, z, og y siden filen betrakter siste tallet som opp
            // Strukturen i temp er nå x, y og z ;)

            file >> temp[0] >> temp[2] >> temp[1];

            // Finner min max
            for (int i = 0; i < 3; i++)
            {
                if (temp[i] > max[i])
                    max[i] = temp[i];

                else if (temp[i] < min[i])
                    min[i] = temp[i];
            }
            if (file.eof())
                break;

            points.emplace_back(vec3{ temp[0],temp[1],temp[2] });
        }
    }
    // Dette flytter alle punktene mot origo(ved å trekke posisionen til punktet med minimum verdiene)
    for(vec3& p : points)
    {
        p[0] -= min[0];
        p[1] -= min[1];
        p[2] -= min[2];
    }
    for(int i = 0; i < 3; i++)
        diff[i] = max[i] - min[i];

    std::cout << "Dimensions \n";
    std::cout << "- Bredde " << diff[0]<< "\n";
    std::cout << "- Lengde " << diff[2]<< "\n";
    std::cout << "- Hoyde " << diff[1]<< "\n";

}

void komprimer(std::vector<vec3>& pts, const float& opplosning )
{
    const int bredde = ceil(diff[0]);
    const int dybde = ceil(diff[2]);
    std::vector<std::vector<std::pair<vec3, int>>> arr(bredde, vector<std::pair<vec3, int>>(dybde, { vec3{0.f}, 0}));

    for(int x = 0; x < bredde; x++)
    {
        for(int z = 0; z < dybde; z++)
        {
            arr[x][z].first[0]  = (double)x;
            arr[x][z].first[1]  = 0.f;
            arr[x][z].first[2]  = (double)z;
            arr[x][z].second = 0;
        }
    }

    for(const vec3& pt : pts)
    {
        const int iX = floor(pt[0]);
        const int iZ = floor(pt[2]);
        arr[iX][iZ].first[1] += pt[1];
        arr[iX][iZ].second += 1;
    }

    for(int x = 0; x < bredde; x++)
    {
        for(int z = 0; z < dybde; z++)
        {
            if(arr[x][z].second != 0)
            {
                arr[x][z].first[1] = arr[x][z].first[1] / (float)arr[x][z].second;
                arr[x][z].second = 0;
            }
        }
    }
}

int main()
{
    std::vector <vec3> points;
    std::cout << "\n starting read\n";
    lesFil("../VSIM22H_LAS_TRIANGULERING/Gol.txt", points);
    std::cout << "\n step 1 done\n";
    komprimer(points, 0.5);
    std::cout << "\n step 2 done\n";
    std::cout << "\n Program Ferdig\n";
    return 0;
}
