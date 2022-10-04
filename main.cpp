#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <math.h>
#include <iomanip>
#include <execution>
//#include <boost/gil.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

using namespace std;

int _bredde;
int _dybde;
int _height;
float dimensions[3] {0.f, 0.f, 0.f};

bool Cmp(const float a, const float b, const float epsilon = 0.0005f)
{
    return (fabs(a - b) < epsilon);
}

struct vec3
{
    vec3()
    {
        xyz[0] = 0;
        xyz[1] = 0;
        xyz[2] = 0;
    }
    vec3(float inp)
    {
        xyz[0] = inp;
        xyz[1] = inp;
        xyz[2] = inp;
    }
    vec3(float _x,float _y,float _z)
    {
        xyz[0] = _x;
        xyz[1] = _y;
        xyz[2] = _z;
    }
    float operator [] (int i) const { return xyz[i]; }
    float& operator [] (int i) { return xyz[i]; }
    float operator - (vec3& a)
    {
        return pow(a.xyz[0] - xyz[0], 2) +
                pow(a.xyz[2] - xyz[2], 2);
    }

private:
    float xyz[3];
};

float shortSort(std::vector<vec3>& points, vec3 referance);

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
        std::cout << std::fixed;
        std::cout << std::setprecision(3);
        system("cls");
        std::cout << " Reading file\n";
        while (true)
        {
            if(points.size() % 1000000 == 0)
                std::cout << "| "<<points.size()<<"\n";
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
            points.emplace_back(vec3{ temp[0],temp[1],temp[2] });
            //cout << "x: " << temp[0] << " | y: " << temp[1] << " | z: " << temp[2] << "\n";
            if (file.eof())
                break;
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
        dimensions[i] = max[i] - min[i];

    std::cout << "Dimensions \n";
    std::cout << "- Bredde " << dimensions[0]<< "\n";
    std::cout << "- Lengde " << dimensions[2]<< "\n";
    std::cout << "- Hoyde " << dimensions[1]<< "\n";

}
struct point
{
    float y{0.f};
    float x{0.f};
    float z{0.f};
    int counter{0};
};

void komprimer(std::vector<vec3>& pts, const float& opplosning )
{
    const int bredde = (int)ceil(dimensions[0]/opplosning)+1;
    const int dybde =  (int)ceil(dimensions[2]/opplosning)+1;
    point** arr = new point*[bredde];

    for(int i = 0; i < bredde; i++)
    {
        arr[i] = new point[dybde];
        for (int j = 0; j < dybde; ++j)
        {
             arr[i][j].counter = 0;
             arr[i][j].x = i * opplosning;
             arr[i][j].y = 0.f;
             arr[i][j].z = j * opplosning;
        }
    }

    for(const vec3& pt : pts)
    {
        const int iX = (int)floor(pt[0] / opplosning);
        const int iZ = (int)floor(pt[2] / opplosning);

        arr[iX][iZ].y += pt[1];
        arr[iX][iZ].counter += 1;
    }


    std::atomic<int> cnt = 0; //std::atomic<int> x = 0;
    std::cout << "looping over points...\n";
    std::for_each(std::execution::par_unseq, arr, (point**)((size_t)arr + (bredde * sizeof(size_t))), [dybde, bredde, &cnt, opplosning](point* points) mutable
    {
        for (int z = 0; z < dybde; ++z)
        {
            auto& point = points[z];
            cnt++;
//            std::cout << "looping... (" << cnt << "/" << bredde*dybde << ")\n";

            if(point.counter != 0)
            {
                point.y = (point.y / (float)point.counter) / opplosning;
            }
            else
            {
                //point.y = (shortSort(pts, vec3(point.x,0,point.z)))/opplosning;
            }
        }
    });

    std::cout << "Main loop completed \n";

    int smoothingIterations = 30;

    for(int y = 0; y < smoothingIterations; y++)
    {
        std::cout << "Smoothing step ("<<y+1<<"/"<<smoothingIterations<<")\n";
        for(int i = 0; i < bredde-1; i++)
            for(int j = 0; j < dybde-1; j++)
            {

                float avg = 0;
                int counter = 0;
                if(i != 0 && j != 0)
                {
                    avg += arr[i-1][j-1].y;counter++;
                }
                if(i > 0)
                {
                    avg += arr[i-1][j].y; counter++;
                    avg += arr[i-1][j+1].y; counter++;
                }
                if(j > 0)
                {
                    avg += arr[i][j-1].y; counter++;
                    avg += arr[i+1][j-1].y; counter++;
                }

                avg += arr[i+1][j].y;counter++;
                avg += arr[i+1][j+1].y;counter++;
                avg += arr[i][j+1].y;counter++;
                avg /= (float)counter;
                float diff = arr[i][j].y-avg;
                arr[i][j].y = avg;

            }
    }
/*

    int cnt = 0;
    for(int z = 0 ; z< dybde; z++)
        for(int x = 0 ; x< bredde; x++)
        {
            cnt++;
            std::cout << "looping... (" << cnt << "/" << bredde*dybde << ")\n";

            auto& point = arr[x][z];
            point.x = x; point.z = z;
            if(point.counter != 0)
            {
                point.y = point.y / (float)point.counter;
            }
            else
            {
                point.y = shortSort(pts, vec3(x,0,z));
            }

        }
    */

    pts.clear();

    for(int z = 0; z < dybde; z++)
    {
        for(int x = 0; x < bredde; x++)
        {
            pts.emplace_back(vec3(arr[x][z].x, arr[x][z].y, arr[x][z].z));
        }
    }
    std::cout << "Sorting pts " << std::endl;

    struct {
            bool operator()(vec3& a, vec3& b) const {

                if (a[2] < b[2]) return true;
                if (b[2] < a[2]) return false;

                if (a[0] < b[0]) return true;
                if (b[0] < a[0]) return false;

                return false;
            }
        } customLess;

    std::sort(pts.begin(), pts.end(), customLess);


    /*
    for (int i = 0; i < pts.size(); i++)
    {
        float sampleVal = 0; float numSamples = 0.f;
        for(int u = 0; u < 5; u++)
            for(int v = 0; v < 5 && u * bredde + i + v < pts.size(); v++)
            {
                sampleVal = pts[u * bredde + i + v][1];
                numSamples += 1.f;
            }

        pts[i][1] = sampleVal / numSamples;
    }
*/

    _dybde = dybde;
    _bredde = bredde;
    _height = dimensions[1];
}

void eksporter(std::vector<vec3>& points, const float& opplosning)
{
    //Simplifisering
    std::vector<uint32_t> indices;
    for(int j = 0; j < _dybde-1; j++)
        for(int i = 0; i < _bredde-1; i++)
        {
            int index = j * _bredde + i;
            indices.push_back(index);               //0
            indices.push_back(index + 1);           //1
            indices.push_back(index + 1 + _bredde); //2
            indices.push_back(index);               //0
            indices.push_back(index + 1 + _bredde); //2
            indices.push_back(index + _bredde);     //3
        }

    std::string navn;
    std::cout << "\n\n----- EKSPORTERING -----\n";
    std::cout << "Hva skal den komprimerte filen hete? \n|-> ";
    std::cin >> navn;
    std::cout << "\nEksporterer nå til " << navn << "_compressed.txt";

//    fstream file;
//    file.open("../VSIM22H_LAS_TRIANGULERING/"+navn+"_compressed.txt", ios_base::out);
////        file << (const char*)points.size();
////        file.write((const char*)points.data(), points.size() * sizeof(vec3));
////        file << (const char*)indices.size();
////        file.write((const char*)indices.data(), indices.size() * sizeof(uint32_t));
//    for(int i = 0; i < points.size(); i++)
//    {
//        file << "("<<points[i][0]<<", "<<points[i][1]<<", "<<points[i][2]<<")"<<"\n";
//    }
//    file << indices.size()<<"\n";;
//    for(int i = 0; i < indices.size(); i++)
//    {
//        file << indices[i]<<"\n";
//    }
//    file.close();

// PNG BITEN!
    //er kun ute etter rgb
    uint32_t Kanaler = 1;
    int size{1024};
    int bredde = _bredde;
    int dybde = _dybde;
    int høyde = _height;
//    if(bredde < dybde)
//        size = bredde;
//    else
//        size = dybde;
//Hentet fra http://chanhaeng.blogspot.com/2018/12/how-to-use-stbimagewrite.html
    // Med noen modifikasjoner til size og filnavn
    //points.size();
    float* pixels = new float[size * size * Kanaler];
    int index = 0;

    for (int j = size-1; j >= 0; --j)
    {
        for (int i = 0; i < size; ++i)
        {
            int pointIndex = j * bredde + i;
            float r = points[pointIndex][1]/(float)_height;
            //std::cout << " | r : " << r << "\n";
//            float g = points[pointIndex][1]/_height;
//            float b = points[pointIndex][1]/_height;
            //int ir = int(255.99 * r);
//            int ig = int(255.99 * g);
//            int ib = int(255.99 * b);

//            pixels[index++] = ir;
//            pixels[index++] = ig;
//            pixels[index++] = ib;
            pixels[index++] = r;
//            pixels[index++] = g;
//            pixels[index++] = b;
        }
    }
    std::string exportNavn = "../VSIM22H_LAS_TRIANGULERING/png/"+navn + "_compressed.hdr";
    //stbi_write_png( exportNavn.c_str(), size, size, Kanaler, pixels, size * Kanaler);
    stbi_write_hdr(exportNavn.c_str(), size, size, Kanaler, pixels);
    delete[] pixels;
}

struct helper
{
    helper(float x, int i)
    {
        dist = x; index = i;
    }
    float dist;
    int index;
    bool operator <(const helper& helper1) const
    {
        return helper1.dist > dist;
    }
};

float shortSort(std::vector<vec3>& points, vec3 referancePoint)
{
    //Finn de nærmeste 10 punktene og ta gjennomsnittshøyden av de 10 som høydeverdien her.
    std::vector<helper> help;
    for(int i = 0; i < points.size(); i++)
    {
        float dist = referancePoint -  points[i];
        help.push_back({dist, i});
    }

    std::sort(help.begin(), help.end());

    float returnValue{0.f};
    for(int i = 0; i < 10; i++)
    {
        returnValue += (points[help[i].index])[1];
    }
    help.clear();
    return returnValue/10.f;
}

int main()
{
    std::vector <vec3> points;
    std::cout << "\n starting read\n";
    //lesFil("../VSIM22H_LAS_TRIANGULERING/Fjell2.txt", points);
    lesFil("../VSIM22H_LAS_TRIANGULERING/stoooor.txt", points);
    //lesFil("../VSIM22H_LAS_TRIANGULERING/Gol.txt", points);
    std::cout << "\n step 1 done\n";

    komprimer(points, 4.0);
    std::cout << "\n step 2 done\n";
    eksporter(points, 4.0);
    std::cout << "\n Program Ferdig\n";
    return 0;
}
