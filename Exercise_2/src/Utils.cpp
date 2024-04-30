#include "Utils.hpp"
#include "PolygonalMesh.hpp"
#include "Eigen/Eigen"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

double VerticesDistance(const vector<Vector2d>& Coordinates, const unsigned int origin, const unsigned int end)
{
    return (Coordinates[origin]-Coordinates[end]).norm()/max(Coordinates[end].norm(),(1.));
}

double PolygonalArea(const VectorXi& vertices, vector<Vector2d>& coordinates)
{
    unsigned int n = vertices.size();
    double sum=0;
    for (unsigned int i=0; i < n; i++)
    {
        unsigned int v1= vertices(i);
        unsigned int v2= vertices(i+1);
        sum += coordinates[v1](0)*coordinates[v2](1) - coordinates[v2](0)*coordinates[v1](1);
    }

    return abs(sum)/2;;
}

namespace PolygonalLibrary{

bool ImportMesh(const string& filepath, PolygonalMesh& mesh)
{
    if(!ImportCell0Ds(filepath + "/Cell0Ds.csv", mesh))
    {
        return false;
    }
    else
    {
        // Lettura assegazione corretta marker cell0D
        cout << "Cell0D marker:" << endl;
        for(auto it = mesh.MarkersCell0D.begin(); it != mesh.MarkersCell0D.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;
            cout << endl;
        }
    }

    if(!ImportCell1Ds(filepath + "/Cell1Ds.csv", mesh))
    {
        return false;
    }
    else
    {
        // Lettura assegazione corretta marker cell1D
        cout << "Cell1D marker:" << endl;
        for(auto it = mesh.MarkersCell1D.begin(); it != mesh.MarkersCell1D.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;
            cout << endl;
        }

        // Test lunghezza lati non nulla (nulla se minore di epsilon di macchina)
        double tol1= max(numeric_limits<double>::epsilon(),mesh.tolerance);
        for(const Vector2i& edge : mesh.VerticesCell1D)
        {
            if(VerticesDistance(mesh.CoordinatesCell0D, edge(0), edge(1)) <= tol1)
            {
                cerr << "Error: edges with zero length" << endl;
                return 2;
            }
        }
    }

    if(!ImportCell2Ds(filepath + "/Cell2Ds.csv", mesh))
    {
        return false;
    }
    else
    {
        // Test area cell2D
        double tol2= max(numeric_limits<double>::epsilon(),pow(mesh.tolerance,2));
        for(const VectorXi& vertices : mesh.VerticesCell2D)
        {
            if(PolygonalArea(vertices, mesh.CoordinatesCell0D) <= tol2 )
            {
                cerr << "Error: polygonal cell with zero area" << endl;
                return 3;
            }
        }
    }

    return true;
}


bool ImportCell0Ds(const string& filename, PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
    {
        cerr << "Error while opening file Cell0D" << endl;
        return false;
    }

    string header;
    getline(file,header);

    list<string> listLines;
    string line;
    while (getline(file, line))
        listLines.push_back(line);

    file.close();

    mesh.NumberCell0D = listLines.size();

    if (mesh.NumberCell0D == 0)
    {
        cerr << "There is no cell 0D" << endl;
        return false;
    }

    mesh.IdCell0D.reserve(mesh.NumberCell0D);
    mesh.CoordinatesCell0D.reserve(mesh.NumberCell0D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2d coord;
        char separator;

        converter >>  id >> separator >> marker >> separator >> coord(0) >> separator >> coord(1);

        mesh.IdCell0D.push_back(id);
        mesh.CoordinatesCell0D.push_back(coord);

        if( marker != 0)
        {
            auto ret = mesh.MarkersCell0D.insert({marker, {id}});
            if(!ret.second)
                (ret.first)->second.push_back(id);
        }
    }

    return true;
}


bool ImportCell1Ds(const string& filename, PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
    {
        cerr << "Error while opening file Cell1D" << endl;
        return false;
    }

    string header;
    getline(file,header);

    list<string> listLines;
    string line;
    while (getline(file, line))
        listLines.push_back(line);

    file.close();

    mesh.NumberCell1D = listLines.size();

    if (mesh.NumberCell1D == 0)
    {
        cerr << "There is no cell 1D" << endl;
        return false;
    }

    mesh.IdCell1D.reserve(mesh.NumberCell1D);
    mesh.VerticesCell1D.reserve(mesh.NumberCell1D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2i vertices;
        char separator;

        converter >>  id >> separator >> marker >> separator >> vertices(0) >> separator >> vertices(1);

        mesh.IdCell1D.push_back(id);
        mesh.VerticesCell1D.push_back(vertices);

        if( marker != 0)
        {
            auto ret = mesh.MarkersCell1D.insert({marker, {id}});
            if(!ret.second)
                (ret.first)->second.push_back(id);
        }
    }

    return true;
}


bool ImportCell2Ds(const string& filename, PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
    {
        cerr << "Error while opening file Cell2D" << endl;
        return false;
    }

    string header;
    getline(file,header);

    list<string> listLines;
    string line;
    while (getline(file, line))
        listLines.push_back(line);

    file.close();

    mesh.NumberCell2D = listLines.size();

    if (mesh.NumberCell2D == 0)
    {
        cerr << "There is no cell 2D" << endl;
        return false;
    }

    mesh.IdCell2D.reserve(mesh.NumberCell2D);
    mesh.VerticesCell2D.reserve(mesh.NumberCell2D);
    mesh.EdgesCell2D.reserve(mesh.NumberCell2D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        unsigned int numVertices;
        unsigned int numEdges;
        VectorXi vertices;
        VectorXi edges;
        char separator;

        converter >>  id >> separator >> marker >> separator >> numVertices;

        for (unsigned int v=0; v<numVertices; v++)
            converter >> separator >> vertices(v);

        converter >> separator >> numEdges;

        //Test numero vertici = numero lati
        if (numEdges!=numVertices)
        {
            cerr << "Error in cell 2D: number of edges and vertices do NOT correspond" << endl;
            return 4;
        }

        for (unsigned int v=0; v<numEdges; v++)
            converter >> separator >> edges(v);

        mesh.IdCell2D.push_back(id);
        mesh.VerticesCell2D.push_back(vertices);
        mesh.EdgesCell2D.push_back(edges);
    }

    return true;
}

}
