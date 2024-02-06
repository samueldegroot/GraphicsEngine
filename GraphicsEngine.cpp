#include "olcPixelGameEngine.h"

struct vector3d { //struct defining vector in 3d space, determined by xyz coords
    float x, y, z;
    //vector3d(float x, float y, float z) : x(x), y(y), z(z) { }
};

struct triangle { //struct defining a triangle, which is made of 3 points
    vector3d points[3];
    //triangle(vector3d a, vector3d b, vector3d c) : points{ a, b, c } { }
};

struct mesh { //struct defining mesh, which is collection of triangles
    std::vector<triangle> triangles;
};

class GrahpicsEngine : public olc::PixelGameEngine {

private:
    mesh meshCube;

public:
    GrahpicsEngine() {
        sAppName = "Cube Demo";
    }

    bool OnUserCreate() override {
        meshCube.triangles = {
            {0,0,0,  0,1,0,  1,1,0},
            {0,0,0,  1,1,0,  1,0,0},
            {1,0,0,  1,1,0,  1,1,1},
            {1,0,0,  1,1,1,  1,0,1},
            {1,0,1,  1,1,1,  0,1,1},
            {1,0,1,  0,1,1,  0,0,1},
            {0,0,1,  0,1,1,  0,1,0},
            {0,0,1,  0,1,0,  0,0,0},
            {0,1,0,  0,1,1,  1,1,1},
            {0,1,0,  1,1,1,  1,1,0},
            {1,0,1,  0,0,1,  0,0,0},
            {1,0,1,  0,0,0,  1,0,0}
        };
    }

    bool OnUserUpdate(float elapsedTime) override {
        FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);
        return true;
    }
};

int main()
{
    GrahpicsEngine demo;
    if (demo.Construct(256, 240, 4, 4)) {
        demo.Start();
    }
    else {
        throw std::runtime_error("Console not successfully constructed");
    }
    return 0;
}