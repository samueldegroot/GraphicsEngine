#define OLC_PGE_APPLICATION
#define _USE_MATH_DEFINES
#include "olcPixelGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>
#include <math.h>

struct Vector3d { //struct defining vector in 3d space, determined by xyz coords
    float x, y, z;
    //vector3d(float x, float y, float z) : x(x), y(y), z(z) { }
};

struct Triangle { //struct defining a triangle, which is made of 3 points
    Vector3d points[3];
    olc::Pixel color;
    //triangle(vector3d a, vector3d b, vector3d c) : points{ a, b, c } { }
};

struct Mesh { //struct defining mesh, which is collection of triangles
    std::vector<Triangle> triangles;

    bool LoadObjectFromFile(std::string sFilename)
    {
        std::ifstream f(sFilename);
        if (!f.is_open())
            return false;

        // Local cache of verts
        std::vector<Vector3d> verts;

        while (!f.eof())
        {
            char line[128];
            f.getline(line, 128);

            std::strstream s;
            s << line;

            char junk;

            if (line[0] == 'v')
            {
                Vector3d v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }

            if (line[0] == 'f')
            {
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                triangles.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
            }
        }

        return true;
    }
};

struct Matrix4x4 {
    float matrix[4][4] = { 0 };
};

class GrahpicsEngine : public olc::PixelGameEngine {

private:
    Mesh meshCube;
    Matrix4x4 projectionMatrix;
    float theta = 0.0f;

    Vector3d camera;

    void MultiplyVectorByMatrix(Vector3d& input_vector, Vector3d& output_vector, Matrix4x4& matrix) {
        output_vector.x = input_vector.x * matrix.matrix[0][0] + input_vector.y * matrix.matrix[1][0] + input_vector.z * matrix.matrix[2][0] + matrix.matrix[3][0];
        output_vector.y = input_vector.x * matrix.matrix[0][1] + input_vector.y * matrix.matrix[1][1] + input_vector.z * matrix.matrix[2][1] + matrix.matrix[3][1];
        output_vector.z = input_vector.x * matrix.matrix[0][2] + input_vector.y * matrix.matrix[1][2] + input_vector.z * matrix.matrix[2][2] + matrix.matrix[3][2];
        float w = input_vector.x * matrix.matrix[0][3] + input_vector.y * matrix.matrix[1][3] + input_vector.z * matrix.matrix[2][3] + matrix.matrix[3][3];

        if (w != 0) {
            output_vector.x /= w;
            output_vector.y /= w;
            output_vector.z /= w;
        }
    }

    void ScaleTriangleToScreen(Triangle& triangle) {
        for (int i = 0; i < 3; i++) {
            triangle.points[i].x += 1.0f;
            triangle.points[i].y += 1.0f;
            triangle.points[i].x *= 0.5f * (float)ScreenWidth();
            triangle.points[i].y *= 0.5f * (float)ScreenHeight();
        }
    }

    void NormalizeVector(Vector3d& input_vector) {
        float length = sqrtf(input_vector.x * input_vector.x + input_vector.y * input_vector.y + input_vector.z * input_vector.z);
        input_vector.x /= length;
        input_vector.y /= length;
        input_vector.z /= length;
    }

    olc::Pixel GetShadeFromLumosity(float lumosity)
    {
        olc::Pixel shadedColor = olc::PixelF(lumosity, lumosity, lumosity);
        return shadedColor;
    }

    //rotation matrices from https://en.wikipedia.org/wiki/Rotation_matrix#:~:text=in%20its%20center.-,Basic%203D%20rotations,-%5Bedit%5D
    void RotateObjectX(Triangle& input_triangle, Triangle& output_triangle, float theta) {
        Matrix4x4 rotationMatrixX;

        rotationMatrixX.matrix[0][0] = 1;
        rotationMatrixX.matrix[1][1] = -cosf(theta * 0.5f);
        rotationMatrixX.matrix[1][2] = sinf(theta * 0.5f);
        rotationMatrixX.matrix[2][1] = -sinf(theta * 0.5f);
        rotationMatrixX.matrix[2][2] = -cosf(theta * 0.5f);
        rotationMatrixX.matrix[3][3] = 1;

        for (int i = 0; i < 3; i++) {
            MultiplyVectorByMatrix(input_triangle.points[i], output_triangle.points[i], rotationMatrixX);
        }
    }

    void RotateObjectY(Triangle& input_triangle, Triangle& output_triangle, float theta) {
        Matrix4x4 rotationMatrixY;

        rotationMatrixY.matrix[0][0] = cosf(theta);
        rotationMatrixY.matrix[0][2] = -sinf(theta);
        rotationMatrixY.matrix[1][1] = 1;
        rotationMatrixY.matrix[2][0] = sinf(theta);
        rotationMatrixY.matrix[2][2] = cosf(theta);
        rotationMatrixY.matrix[3][3] = 1;

        for (int i = 0; i < 3; i++) {
            MultiplyVectorByMatrix(input_triangle.points[i], output_triangle.points[i], rotationMatrixY);
        }
    }

    void RotateObjectZ(Triangle& input_triangle, Triangle& output_triangle, float theta) {
        Matrix4x4 rotationMatrixZ;

        rotationMatrixZ.matrix[0][0] = cosf(theta);
        rotationMatrixZ.matrix[0][1] = sinf(theta);
        rotationMatrixZ.matrix[1][0] = -sinf(theta);
        rotationMatrixZ.matrix[1][1] = cosf(theta);
        rotationMatrixZ.matrix[2][2] = 1;
        rotationMatrixZ.matrix[3][3] = 1;

        for (int i = 0; i < 3; i++) {
            MultiplyVectorByMatrix(input_triangle.points[i], output_triangle.points[i], rotationMatrixZ);
        }
    }

public:
    GrahpicsEngine() {
        sAppName = "Cube Demo";
    }

    bool OnUserCreate() override {
        /*meshCube.triangles = {
        { 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },                                                    
        { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },                                                   
        { 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },                                                     
        { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },                                                     
        { 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },                                                  
        { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f }
        };*/

        if (!meshCube.LoadObjectFromFile("peter_griffin.obj")) {
            return false;
        }

        float zNear = 0.1f;
        float zFar = 1000.0f;
        float fieldOfView = 90.0f;
        float aspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
        float fovRadians = 1.0f / tanf(fieldOfView * 0.5f / 180.0f * M_PI);

        projectionMatrix.matrix[0][0] = aspectRatio * fovRadians;
        projectionMatrix.matrix[1][1] = fovRadians;
        projectionMatrix.matrix[2][2] = zFar / (zFar - zNear);
        projectionMatrix.matrix[3][2] = (-zFar * zNear) / (zFar - zNear);
        projectionMatrix.matrix[2][3] = 1.0f;
        projectionMatrix.matrix[3][3] = 0.0f;

        return true;
    }

    bool OnUserUpdate(float elapsedTime) override {
        FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);

        theta += 1.0f * elapsedTime;

        for (auto triangle : meshCube.triangles) {
            Triangle triangleProjected, triangleTranslated, triangleRotatedZ, triangleRotatedX, triangleRotatedY;

            RotateObjectX(triangle, triangleRotatedX, 0);
            RotateObjectY(triangleRotatedX, triangleRotatedY, theta);
            RotateObjectZ(triangleRotatedY, triangleRotatedZ, 0);

            triangleTranslated = triangleRotatedZ;
            
            for (int i = 0; i < 3; i++) {
                triangleTranslated.points[i].z += 2.0f;
                triangleTranslated.points[i].y += 1.0f;
            }

            Vector3d normal, line1, line2;
            line1.x = triangleTranslated.points[1].x - triangleTranslated.points[0].x;
            line1.y = triangleTranslated.points[1].y - triangleTranslated.points[0].y;
            line1.z = triangleTranslated.points[1].z - triangleTranslated.points[0].z;

            line2.x = triangleTranslated.points[2].x - triangleTranslated.points[0].x;
            line2.y = triangleTranslated.points[2].y - triangleTranslated.points[0].y;
            line2.z = triangleTranslated.points[2].z - triangleTranslated.points[0].z;

            normal.x = line1.y * line2.z - line1.z * line2.y;
            normal.y = line1.z * line2.x - line1.x * line2.z;
            normal.z = line1.x * line2.y - line1.y * line2.x;

            NormalizeVector(normal);

            if (normal.x * (triangleTranslated.points[0].x - camera.x) +
                normal.y * (triangleTranslated.points[0].y - camera.y) +
                normal.z * (triangleTranslated.points[0].z - camera.z) < 0)
            {
                Vector3d directionalLight = { 0, 0, -1 };
                NormalizeVector(directionalLight);

                float dotProduct = normal.x * directionalLight.x + normal.y * directionalLight.y + normal.z * directionalLight.z;

                triangleTranslated.color = GetShadeFromLumosity(dotProduct);

                for (int i = 0; i < 3; i++) {
                    MultiplyVectorByMatrix(triangleTranslated.points[i], triangleProjected.points[i], projectionMatrix);
                }
                triangleProjected.color = triangleTranslated.color;
                ScaleTriangleToScreen(triangleProjected);

                FillTriangle(triangleProjected.points[0].x, triangleProjected.points[0].y, triangleProjected.points[1].x,
                    triangleProjected.points[1].y, triangleProjected.points[2].x, triangleProjected.points[2].y, triangleProjected.color);
            }
        }

        return true;
    }
};

int main()
{
    GrahpicsEngine demo;
    if (demo.Construct(800, 600, 1, 1)) {
        demo.Start();
    }
    else {
        throw std::runtime_error("Console not successfully constructed");
    }
    return 0;
}