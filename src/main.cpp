
#include "headers.hpp"

#include <iostream>
// Local includes
#include "mainLoop.hpp"
#include "camera.hpp"
#include "scene.hpp"

#include <glm/glm.hpp>

int main(int argc, char* argv[]) {
    
    int windowWidth = 800;
    int windowHeight = 600;

    //tard flemme TODO: à faire
    Ui interface{};

    // initialize camera
    Camera cam{glm::vec3{0,0,6}, 50.0f, windowWidth, windowHeight};
    //Create a light
    Light light1{glm::vec3{0,0,4},glm::vec3{2.0f}};
    Light light2{glm::vec3{0,2.5,-4},glm::vec3{3.5f}};
    // create a scene
    Scene baseScene{};

    // add object to scene
    Cube cube1(1.0f);
    Cube cube2(1.0f);
    Cube cubelight1(0.2f);
    Cube cubelight2(0.2f);

    FileModel teapot{"teapot.obj"};
    FileModel gtx{"GTX_1070TI.obj"};
    //Cube lightpos(0.3f);

    //lightpos.setPosition(glm::vec3{0,2,0});
    cube1.setTex("containerDiffuse.png","containerSpecular.png");

    cube2.setScale(glm::vec3{1.2}).setRotation(40.0,glm::vec3{0,0.5,0})
        .setPosition(glm::vec3{2.5,0,0});

    cubelight1.setPosition(light1.getPos());

    cubelight2.setPosition(light2.getPos());

    teapot.setScale(glm::vec3{0.2}).setPosition(glm::vec3{-3,0,0});

    gtx.setScale(glm::vec3{4.0});

    baseScene.addModel(cube1).addModel(cube2).addModel(cubelight1).addModel(cubelight2)
            .addLight(light1).addLight(light2).addModel(teapot).addModel(gtx);

    // start render loop, open GLFW window
    MainLoop renderLoop{baseScene,interface,cam};
    //load scene to GPU
    baseScene.load();
    //start render loop
    renderLoop.run();

    return 0;
}







