#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <math.h>
#include <iomanip>
#include <execution>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

using namespace std;

int _bredde;
int _dybde;
double dimensions[3] {0.f, 0.f, 0.f};
struct vec3
{
    vec3()
    {
        xyz[0] = 0;
        xyz[1] = 0;
        xyz[2] = 0;
    }
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
    double operator - (vec3& a)
    {
        return pow(a.xyz[0] - xyz[0], 2) +
                pow(a.xyz[2] - xyz[2], 2);
    }
private:
    double xyz[3];
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
void eksporter(std::vector<vec3>& points);
void komprimer(std::vector<vec3>& pts, const float& opplosning )
{
    const int bredde = (int)ceil(dimensions[0]/opplosning);
    const int dybde =  (int)ceil(dimensions[2]/opplosning);
    point** arr = new point*[bredde];
    for(int i = 0; i < bredde; i++)
    {
        arr[i] = new point[dybde];
    }

    for(const vec3& pt : pts)
    {
        const int iX = (int)floor(pt[0]/opplosning);
        const int iZ = (int)floor(pt[2]/opplosning);
        if(iX < 0.f || iZ < 0.f)
        {
            std::cout << "x: "<<pt[0]<<" | z: " <<pt[2];
        }
        arr[iX][iZ].x = pt[0];
        arr[iX][iZ].y += pt[1];
        arr[iX][iZ].z = pt[2];
        arr[iX][iZ].counter += 1;
    }

    std::vector<vec3> newPoints;
    newPoints.reserve(bredde * dybde);
    int cnt = 0;
    for(int z = 0; z < dybde; z++)
    {
        for(int x = 0; x < bredde; x++)
        {
            cnt++;
            system("cls");
            std::cout << "looping... (" <<cnt <<"/"<< bredde*dybde<<")\n";
            std::cout << "|- z loop  (" <<z <<"/"<< dybde<<")\n";
            std::cout << "|- x loop  (" <<x <<"/"<< bredde<<")\n";

            if(arr[x][z].counter != 0)
            {
                arr[x][z].y = arr[x][z].y / (float)arr[x][z].counter;
            }
            else
            {
                arr[x][z].y = shortSort(pts, vec3(x, 0, z));
            }
            newPoints.push_back(vec3{(double)x,arr[x][z].y, (double)z});
        }
    }
//    std::for_each(std::execution::par_unseq, arr, arr+sizeof(point)*bredde*dybde, [&pts](point* point)
//    {

//        if(point->counter != 0)
//        {
//            point->y = point->y / (float)point->counter;
//        }
//        else
//        {
//            point->y = shortSort(pts, vec3(point->x,0,point->z));
//        }
//    });
    _dybde = dybde;
    _bredde = bredde;
    eksporter(newPoints);
}

void eksporter(std::vector<vec3>& points)
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

    fstream file;
    file.open("../VSIM22H_LAS_TRIANGULERING/"+navn+"_compressed.txt", ios_base::out);
    file << points.size()<<"\n";
    for(int i = 0; i < points.size(); i++)
    {
        file << "("<<points[i][0]<<", "<<points[i][1]<<", "<<points[i][2]<<")"<<"\n";
    }
    file << indices.size()<<"\n";;
    for(int i = 0; i < indices.size(); i++)
    {
        file << indices[i]<<"\n";
    }
}

void lagPNG(std::vector<vec3>& points)
{
    //int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
    //stbi_write_png("png_testing",_bredde,_dybde,);
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

void komprimer2(std::vector<vec3>& pts, float resolution)
{

    // Kode som regner ut antall punkter i den nye meshen
    // Hvor totalen er xPoints * zPoints;
    const int xPoints = (int)ceil(dimensions[0] / resolution);
    const int zPoints = (int)ceil(dimensions[2] / resolution);
    const int amount = xPoints * zPoints;

    std::vector<vec3> vertices;
    vertices.reserve(amount);

    // Formelen for å finne indexen til et punkt er
    // z * xPoints + x;

    int counter{0};
    for(int j = 0; j < zPoints; j++)
    {
        for(int i = 0; i < xPoints; i++)
        {
            counter++;
            system("cls");
            std::cout << "Loop count ("<<counter<<"/"<<amount<<")";
            vec3 temp {(double)i, 0.f, (double)j};
            temp[1] = shortSort( pts, temp);
            vertices.push_back(temp);
        }
    }
    std::vector<int> indexes;
    for(int j = 0; j < zPoints-1; j++)
        for(int i = 0; i < xPoints-1; i++)
        {
            int idx = j * xPoints + i;

            indexes.push_back(idx);
            indexes.push_back(idx+1);
            indexes.push_back(idx+1+xPoints);
            indexes.push_back(idx+1+xPoints);
            indexes.push_back(idx+xPoints);
            indexes.push_back(idx);
        }
    system("cls");
    std::cout << "Loop Complete ("<<counter<<"/"<<amount<<")\n";
    std::string navn;
    std::cout << "\n\n----- EKSPORTERING -----\n";
    std::cout << "Hva skal den komprimerte filen hete? \n|-> ";
    std::cin >> navn;
    std::cout << "\nEksporterer nå til " << navn << "_compressed.txt";

    fstream file;
    file.open("../VSIM22H_LAS_TRIANGULERING/"+navn+"_compressed.txt", ios_base::out);
    file << vertices.size()<<"\n";
    for(int i = 0; i < vertices.size(); i++)
    {
        file << "("<<vertices[i][0]<<", "<<vertices[i][1]<<", "<<vertices[i][2]<<")"<<"\n";
    }
    file << indexes.size()<<"\n";;
    for(int i = 0; i < indexes.size(); i++)
    {
        file << indexes[i]<<"\n";
    }
}

int main()
{
    std::vector <vec3> points;
    std::cout << "\n starting read\n";
    lesFil("../VSIM22H_LAS_TRIANGULERING/Gol.txt", points);
    std::cout << "\n step 1 done\n";

    komprimer(points, 1.0);
    std::cout << "\n step 2 done\n";



    std::cout << "\n Program Ferdig\n";
    return 0;
}
