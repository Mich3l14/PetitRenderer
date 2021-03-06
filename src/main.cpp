
#include "headers.hpp"

#include <iostream>
// Local includes
#include "mainLoop.hpp"
#include "camera.hpp"
#include "scene.hpp"

#include <glm/glm.hpp>

int main(int argc, char* argv[]) {
    
    int windowWidth = 1200;
    int windowHeight = 800;

    //init the interface/camera/scene
    Ui interface{};
    Camera cam{glm::vec3{0,4,-8}, 50.0f, windowWidth, windowHeight};

    Scene scene{cam};
    // enable SSAO for this scene
    scene.setSSAO(true).setExposure(1.0);



    // ######################### adding lights ###########################
    
    // create a grid of lights for the scene (good to see reflections)
    PointLight light{{0,0,0},{0.1,0.1,0.1}};

    int liS = 10; //size of grid
    int liNum = 3; // 4*4 lights
    std::vector<PointLight> lights = std::vector(liNum*liNum,light);
    //Create our lights
    float liStep = liS/liNum;
    for(int i=0; i<liNum*liNum;i++){
        lights[i].setPosition({-liS/2+(i%(int)liNum)*liStep*2,3.5,-liS/2 +(int)(i/liNum)*liStep*2});
        scene.addLight(lights[i]);
    }

    DistantLight light3{glm::vec3{-0.2,0.25,-0.8},glm::vec3{0.8f}};
    light3.enableShadowMap(4096,10.0f);
    scene.addLight(light3);

    //create a cubemap and add it to the scene
    CubeMap cubemap{"textures/cubemaps/yokohama/"};
    scene.setCubeMap(cubemap);



    //####################### creating models ###########################

    Cube cube0{1.0f};
    cube0.setPosition({1.1f,0.6f,0.0f}).setDiffuse({0.0f,1.0f,0.0f}).setSpecular(glm::vec3{0.9})
        .setShaderType(PHONG);

    Cube cube1{1.0f};
    cube1.setPosition({0.0f,0.6f,0.0f}).setAlbedo({0.0f,1.0f,0.0f}).setRoughness(0.4).setMetallic(0.0)
        .setShaderType(PBR);

    Cube cube2(1.0f);
    cube2.setPosition({-2.5f,0.6f,0.5f}).setScale({3.0,0.2,3.0}).enableTesselation(HIGH)
                .setTexAlbedo("textures/tiles/basecolor.jpg")
                .setTexRoughness("textures/tiles/roughness.jpg")
                .setTexHeight("textures/tiles/height.png").setShaderType(PBR);

    Cube wall(1.0f);
    wall.setScale({8.0f,2.5f,0.3f}).setRotation(45,{0,1,0}).setPosition({-4,1.0,-2});

    scene.addModel(cube0).addModel(cube1).addModel(cube2).addModel(wall);


    UVSphere sphere1(1.0,25,20);
    sphere1.setPosition({3.5,0.7,3.5}).setRotation(-90,{1,0,0}).
    setAlbedo({0.08,0,0.3}).setRoughness(0.52).setMetallic(1.0).setShaderType(PBR).enableTesselation(HIGH);

    scene.addModel(sphere1);


    Plane plane1{{20,20},30,30};
    plane1.setRotation(-90,{1,0,0}).setTexAlbedo("textures/stoneWall/diffuse.png")
                .setTexRoughness("textures/stoneWall/roughness.png")
                .setTexHeight("textures/stoneWall/height.png").displacementStrength(0.1f)
                .setTexNormal("textures/stoneWall/normal.png")
                .setTexAO("textures/stoneWall/ao.png")
                .setTexScaling({4,4}).enableTesselation(LOW).setShaderType(PBR);
    scene.addModel(plane1);


    //gold-ish utah teapot
    FileModel teapot{"models/teapot.obj",SMOOTH_NORMAL_ENABLE};
    teapot.setScale(glm::vec3{0.45f}).setPosition({0.0f,1.2f,-1.8f})
       .setAlbedo({1.0f,0.18f,0.0f}).setRoughness(0.24).setMetallic(01.0)
       .enableTesselation(HIGH).setShaderType(PBR);

    // hard on the gpu
    //FileModel acCobra{"models/AC_Cobra/Shelby.obj",SMOOTH_NORMAL_ENABLE};
    //acCobra.setScale(glm::vec3{1.0f}).setPosition({0.0f,1.0f,6.0f}).setAlbedo({1.0,0.0,0.0}).setMetallic(0.65).setRoughness(0.08).setShaderType(PBR);
    
    FileModel suzanne{"models/suzanne.obj",SMOOTH_NORMAL_ENABLE};
    suzanne.setScale(glm::vec3{0.4f}).setPosition({2.5f,1.0f,-3.0f}).enableTesselation(HIGH).setDiffuse({0.1f,0.4f,0.9f});

    scene.addModel(suzanne).addModel(teapot);


    // ########## start the render loop, give it the scene/cameras #########

    MainLoop renderLoop{scene,interface,cam};
    // first, load scene objects to GPU
    scene.load();
    //start the render loop
    renderLoop.run();

    return 0;
}







