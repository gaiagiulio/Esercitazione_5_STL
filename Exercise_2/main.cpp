#include "Utils.hpp"
#include "PolygonalMesh.hpp"
#include <iostream>

using namespace std;
using namespace PolygonalLibrary;

int main()
{
    PolygonalMesh mesh;

    string filepath = "PolygonalMesh";
    if(!ImportMesh(filepath, mesh))
    {
        return 1;
    }

    return 0;
}
