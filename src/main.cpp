#include <thread>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <chicken3421/chicken3421.hpp>

#include <ass3/player.hpp>
#include <ass3/shapes.hpp>
#include <ass3/texture_2d.hpp>
#include <ass3/static_mesh.hpp>
#include <ass3/scene.hpp>
#include <ass3/utility.hpp>
#include <ass3/renderer.hpp>
#include <ass3/frustum.hpp>

#include <iostream>
#include <cmath>

const int WIN_HEIGHT = 720;
const int WIN_WIDTH = 1280;
const int SHADOW_WIDTH = 8192;
const int SHADOW_HEIGHT = 8192;
const float W_PRESS_SPACE = 0.4f;
const int BLOOM_INTENSITY = 30;
const int TOTAL_TONE_MAPS = 7;
const int TOTAL_KERNELS = 7;

// 0 -> Basic flat dirt world
// 1 -> Wooly world
// 2 -> Iron world
// 3 -> Classic Sky Block

struct pointerInformation {
    scene::world *gameWorld;
    renderer::renderer_t *defaultShader;
    float frameRate = 0, lastWPressed = 0;
    float exposureLevel = 10.0f, averageIlluminance = 0;
    GLint hdrType = 1, kernelType = 0;
    bool enableExperimental = false;
};

struct dayNightTextureSystem {
    private:
    GLuint day = texture_2d::loadCubemap("./res/textures/skybox/day");
    GLuint sunset = texture_2d::loadCubemap("./res/textures/skybox/sunset");
    GLuint night = texture_2d::loadCubemap("./res/textures/skybox/night");
    GLuint sunrise = texture_2d::loadCubemap("./res/textures/skybox/sunrise");
    
    public:
    GLuint practice = texture_2d::loadCubemap("./res/textures/tut_skybox");
    GLuint prevTex, currTex;

    float updatePrevCurr(float dayProgress) {
        dayProgress = fmod(dayProgress, 1.0f);
        if (utility::isInRange(dayProgress, 0.0f, 0.2f)) {
            prevTex = sunrise;
            currTex = day;
            return dayProgress / 0.2f;
        } else if (utility::isInRange(dayProgress, 0.2f, 0.3f)) {
            prevTex = day;
            currTex = day;
            return 1.0f;
        } else if (utility::isInRange(dayProgress, 0.3f, 0.5f)) {
            prevTex = day;
            currTex = sunset;
            return (dayProgress - 0.3f) / 0.2f;
        } else if (utility::isInRange(dayProgress, 0.5f, 0.7f)) {
            prevTex = sunset;
            currTex = night;
            return (dayProgress - 0.5f) / 0.2f;
        } else if (utility::isInRange(dayProgress, 0.7f, 0.8f)) {
            prevTex = night;
            currTex = night;
            return 1.0f;
        } else {
            prevTex = night;
            currTex = sunrise;
            return (dayProgress - 0.8f) / 0.2f;
        }
    }

    void deleteTextures() {
        texture_2d::destroy(day);
        texture_2d::destroy(night);
        texture_2d::destroy(sunset);
        texture_2d::destroy(sunrise);
    }

};

struct waterTextures {
    const float frameLen = 0.15f;
    GLuint frames[32];
    GLuint currFrame = 0;
    GLfloat frame;
    std::string filepath = "./res/textures/water/water_";
    waterTextures() {
        for (int i = 0; i < 32; i++) {
            frames[i] = texture_2d::init(filepath + std::to_string(i) + ".png");
        }
    }

    GLuint getFrame(float dt) {
        frame += dt;
        if (frame > frameLen) {
            frame -= frameLen;
            currFrame++;
            currFrame %= 32;
        }
        return frames[currFrame];
    }
};

int main() {

    // Printing welcome message
    int worldType = 0, renderDistance = 0, worldWidth = 0, frameLimiter = 1;
    std::cout << "\n\u001B[34mWelcome to a clone of Minecraft, created by z5309206 for COMP3421 ass3 21T3 UNSW!\n\n\u001B[0m";
    std::cout << "Presets:\n0 -> Basic Super Flat World\n1 -> Wooly World\n2 -> Iron World\n3 -> Classic Sky Block\n4 -> Parkour Course\n";
    std::cout << "Enter your desired preset world [If not recognised, Basic Super Flat World is used]: ";
    std::cin >> worldType;
    std::cout << "\n";
    std::cout << "Enter your world's width [Minimum 50 | Maximum 500]: ";
    std::cin >> worldWidth;
    if (worldWidth < 50) {
        worldWidth = 50;
    } else if (worldWidth > 500) {
        worldWidth = 500;
    }
    std::cout << "Enter your desired render distance [Minimum 15 | Recommended 30]: ";
    std::cin >> renderDistance;
    if (renderDistance < 15) {
        renderDistance = 15;
    }
    std::cout << "Enable frame limiter? [0 for \"No\" | Anything else for \"Yes\"]: ";
    std::cin >> frameLimiter;

    GLFWwindow *window = chicken3421::make_opengl_window(WIN_WIDTH, WIN_HEIGHT, "COMP3421 21T3 Assignment 2 [Minecraft: Clone Simulator]");
    chicken3421::image_t faviconImage = chicken3421::load_image("./res/textures/favicon.png", false);
    GLFWimage favicon = {faviconImage.width, faviconImage.height, (unsigned char *) faviconImage.data};
    glfwSetWindowIcon(window, 1, &favicon);
    chicken3421::delete_image(faviconImage);

    std::vector<scene::miniBlockData> listOfBlocks;

    std::vector<glm::vec2> listOfPositions;
    std::cout << "\n";
    // Creating worlds
    switch (worldType) {
        case 1:
            std::cout << "Wooly World selected\n";
            listOfBlocks.emplace_back(scene::miniBlockData("gray", glm::vec3(0,0,0), false, true));
            listOfBlocks.emplace_back(scene::miniBlockData("white", glm::vec3(0,1,0), false, true));
            break;
        case 2:
            std::cout << "Iron World selected\n";
            listOfBlocks.emplace_back(scene::miniBlockData("raw_iron", glm::vec3(0,0,0), false, true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(0,1,0), false, true));
            break;
        case 3:
            std::cout << "Classic Sky Block selected\n";
            listOfBlocks.emplace_back(scene::miniBlockData("barrel", glm::vec3(0,3,0), true));
            // Generating land
            listOfPositions = {
                {-1, 0},
                { 0, 0},
                { 1, 0},
                {-1, 1},
                { 0, 1},
                { 1, 1},
                {-1, 2},
                { 0, 2},
                { 1, 2},
                {-1, 3},
                { 0, 3},
                { 1, 3},
                {-1, 4},
                { 0, 4},
                { 1, 4},
                {-1, 5},
                { 0, 5},
                { 1, 5},
                { 1, 5},
                { 2, 3},
                { 2, 4},
                { 2, 5},
                { 3, 3},
                { 3, 4},
                { 3, 5},
                { 4, 3},
                { 4, 4},
                { 4, 5},
            };
            for (auto i : listOfPositions) {
                listOfBlocks.emplace_back(scene::miniBlockData("dirt", glm::vec3(i.x,0,i.y), true));
                listOfBlocks.emplace_back(scene::miniBlockData("dirt", glm::vec3(i.x,1,i.y), true));
                if (i.x == 4 && i.y == 5) {
                    listOfBlocks.emplace_back(scene::miniBlockData("dirt", glm::vec3(i.x,2,i.y), true));

                    // Creating the tree
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_log", glm::vec3(i.x,3,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_log", glm::vec3(i.x,4,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_log", glm::vec3(i.x,5,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_log", glm::vec3(i.x,6,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_log", glm::vec3(i.x,7,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_log", glm::vec3(i.x,8,i.y), true));
                    // Creating the leaves
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,9,i.y), true)); // +
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,9,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,9,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,9,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,9,i.y + 1), true));

                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,8,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,8,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,8,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,8,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,8,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,8,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,8,i.y + 1), true));

                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,7,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,7,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,7,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,7,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,7,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,7,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,7,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,7,i.y - 1), true));

                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 2,7,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 2,7,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 2,7,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 2,7,i.y + 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,7,i.y + 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,7,i.y + 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,7,i.y + 2), true));
                    //
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 2,7,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 2,7,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 2,7,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 2,7,i.y - 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,7,i.y - 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,7,i.y - 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,7,i.y - 2), true));

                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,6,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,6,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,6,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,6,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,6,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,6,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,6,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,6,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 2,6,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 2,6,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 2,6,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 2,6,i.y + 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,6,i.y + 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,6,i.y + 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,6,i.y + 2), true));
                    //
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 2,6,i.y), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 2,6,i.y + 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 2,6,i.y - 1), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 2,6,i.y - 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x - 1,6,i.y - 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x,6,i.y - 2), true));
                    listOfBlocks.emplace_back(scene::miniBlockData("oak_leaves", glm::vec3(i.x + 1,6,i.y - 2), true));
                } else {
                    listOfBlocks.emplace_back(scene::miniBlockData("grass_block", glm::vec3(i.x,2,i.y), true));
                }
            }
            break;
        case 4:
            std::cout << "Parkour Course selected\n";
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-13, 11, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-13, 12, -6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-13, 12, -5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-13, 12, 0), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-13, 15, -5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-12, 0, 10), true));
            listOfBlocks.emplace_back(scene::miniBlockData("copper_block", glm::vec3(-12, 0, 11), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-12, 3, 10), true));
            listOfBlocks.emplace_back(scene::miniBlockData("copper_block", glm::vec3(-11, 0, 10), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-11, 0, 11), true));
            listOfBlocks.emplace_back(scene::miniBlockData("copper_block", glm::vec3(-11, 0, 12), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-11, 3, 11), true));
            listOfBlocks.emplace_back(scene::miniBlockData("copper_block", glm::vec3(-10, 0, 11), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-10, 0, 12), true));
            listOfBlocks.emplace_back(scene::miniBlockData("copper_block", glm::vec3(-10, 0, 13), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-10, 3, 12), true));
            listOfBlocks.emplace_back(scene::miniBlockData("copper_block", glm::vec3(-9, 0, 12), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-9, 0, 13), true));
            listOfBlocks.emplace_back(scene::miniBlockData("copper_block", glm::vec3(-9, 0, 14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-9, 3, 13), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-9, 9, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-9, 10, -6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("copper_block", glm::vec3(-8, 0, 13), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-8, 0, 14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-8, 3, 14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-8, 7, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-8, 8, -6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-7, 5, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-7, 6, -6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-6, 3, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-6, 4, -6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(-3, 0, 19), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(-2, 3, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(0, 0, 19), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(0, 2, 1), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(0, 2, 2), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(0, 2, 3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(0, 2, 4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(0, 2, 5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glowstone", glm::vec3(0, 3, 0), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glowstone", glm::vec3(0, 3, 6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(1, 2, 0), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(1, 2, 1), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(1, 2, 2), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(1, 2, 3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(1, 2, 4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(1, 2, 5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(1, 2, 6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(2, 0, -14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(2, 1, -13), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(2, 2, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(2, 2, 0), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(2, 2, 1), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(2, 2, 2), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(2, 2, 3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(2, 2, 4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(2, 2, 5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(2, 2, 6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("magma", glm::vec3(3, 0, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(3, 2, -12), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(3, 2, 0), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(3, 2, 1), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(3, 2, 2), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(3, 2, 3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(3, 2, 4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(3, 2, 5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(3, 2, 6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(4, 2, 0), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(4, 2, 1), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(4, 2, 2), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(4, 2, 3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(4, 2, 4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(4, 2, 5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(4, 2, 6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("yellow", glm::vec3(4, 3, 8), true));
            listOfBlocks.emplace_back(scene::miniBlockData("yellow", glm::vec3(4, 3, 10), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glowstone", glm::vec3(4, 6, 8), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(5, 2, 0), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(5, 2, 1), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(5, 2, 2), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(5, 2, 3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("obsidian", glm::vec3(5, 2, 4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("iron_block", glm::vec3(5, 2, 5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(5, 2, 6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("yellow", glm::vec3(5, 2, 12), true));
            listOfBlocks.emplace_back(scene::miniBlockData("sea_lantern", glm::vec3(5, 4, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("diamond_ore", glm::vec3(6, 0, 20), true));
            listOfBlocks.emplace_back(scene::miniBlockData("diamond_ore", glm::vec3(6, 0, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("diamond_ore", glm::vec3(6, 0, 22), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(6, 2, 1), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(6, 2, 2), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(6, 2, 3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(6, 2, 4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(6, 2, 5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glowstone", glm::vec3(6, 3, 0), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glowstone", glm::vec3(6, 3, 6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(7, 0, -14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("diamond_ore", glm::vec3(7, 0, 20), true));
            listOfBlocks.emplace_back(scene::miniBlockData("diamond_ore", glm::vec3(7, 0, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("diamond_ore", glm::vec3(7, 0, 22), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glowstone", glm::vec3(7, 1, 19), true));
            listOfBlocks.emplace_back(scene::miniBlockData("marccoin_block", glm::vec3(7, 1, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glowstone", glm::vec3(7, 1, 23), true));
            listOfBlocks.emplace_back(scene::miniBlockData("marccoin_block", glm::vec3(7, 2, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(7, 3, -15), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(7, 3, -14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(7, 3, -13), true));
            listOfBlocks.emplace_back(scene::miniBlockData("marccoin_block", glm::vec3(7, 3, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("marccoin_block", glm::vec3(7, 4, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("marccoin_block", glm::vec3(7, 5, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("marccoin_block", glm::vec3(7, 6, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("marccoin_block", glm::vec3(7, 7, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("marccoin_block", glm::vec3(7, 8, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 0, -14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("diamond_ore", glm::vec3(8, 0, 20), true));
            listOfBlocks.emplace_back(scene::miniBlockData("diamond_ore", glm::vec3(8, 0, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("diamond_ore", glm::vec3(8, 0, 22), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 3, -15), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 3, -14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 3, -13), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 3, -9), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 3, -8), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 3, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 6, -9), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 6, -8), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(8, 6, -7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(9, 0, -14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glowstone", glm::vec3(9, 1, 21), true));
            listOfBlocks.emplace_back(scene::miniBlockData("yellow", glm::vec3(9, 2, 12), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(9, 3, -15), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(9, 3, -14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(9, 3, -13), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(10, 0, -14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("orange", glm::vec3(14, 2, 14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glass", glm::vec3(14, 8, -6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glass", glm::vec3(14, 8, -5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glass", glm::vec3(14, 8, -4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glass", glm::vec3(14, 8, -3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glass", glm::vec3(14, 9, -6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glass", glm::vec3(14, 9, -5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glass", glm::vec3(14, 9, -4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("glass", glm::vec3(14, 9, -3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("red", glm::vec3(15, 0, -14), true));
            listOfBlocks.emplace_back(scene::miniBlockData("orange", glm::vec3(15, 3, 10), true));
            listOfBlocks.emplace_back(scene::miniBlockData("orange", glm::vec3(15, 4, 7), true));
            listOfBlocks.emplace_back(scene::miniBlockData("orange", glm::vec3(15, 5, 4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("orange", glm::vec3(15, 7, -6), true));
            listOfBlocks.emplace_back(scene::miniBlockData("orange", glm::vec3(15, 7, -5), true));
            listOfBlocks.emplace_back(scene::miniBlockData("orange", glm::vec3(15, 7, -4), true));
            listOfBlocks.emplace_back(scene::miniBlockData("orange", glm::vec3(15, 7, -3), true));
            listOfBlocks.emplace_back(scene::miniBlockData("orange", glm::vec3(17, 6, 1), true));
            break;
        default:
            std::cout << "Basic Super Flat World selected\n";
            listOfBlocks.emplace_back(scene::miniBlockData("bedrock", glm::vec3(0,0,0), false, true));
            listOfBlocks.emplace_back(scene::miniBlockData("dirt", glm::vec3(0,1,0), false, true));
            listOfBlocks.emplace_back(scene::miniBlockData("dirt", glm::vec3(0,2,0), false, true));
            listOfBlocks.emplace_back(scene::miniBlockData("grass_block", glm::vec3(0,3,0), false, true));
            break;
    }

    // Setting up all the render informations
    renderer::renderer_t defaultShader;
    defaultShader.createProgram("default");
    defaultShader.initialise(WIN_WIDTH, WIN_HEIGHT);
    scene::world gameWorld(listOfBlocks, renderDistance, worldWidth, &defaultShader);
    
    // Creating different shaders
    renderer::renderer_t shadowShader;
    shadowShader.createProgram("shadow");
    shadowShader.setUpShadow();
    
    renderer::renderer_t blurShader;
    blurShader.createProgram("blur");
    renderer::renderer_t normalShader;
    normalShader.createProgram("normal");

    renderer::renderer_t hdrShader;
    hdrShader.createProgram("hdrBloom");

    renderer::renderer_t skyboxShader;
    skyboxShader.createProgram("skybox");

    renderer::renderer_t cubeReflectShader;
    cubeReflectShader.createProgram("cubeReflection");

    renderer::renderer_t waterShader;
    waterShader.createProgram("water");

    dayNightTextureSystem dayNightCalculator;
    waterTextures waterCalculator;

    // SETTING UP ALL CALLBACKS
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetKeyCallback(window, [](GLFWwindow *win, int key, int scancode, int action, int mods) {
        // Close program if esc is pressed
        
        pointerInformation *info = (pointerInformation *) glfwGetWindowUserPointer(win);
        if (key != GLFW_KEY_I && key != GLFW_KEY_TAB) info->gameWorld->toggleInstructions(true);
        switch(key) {
            case GLFW_KEY_W:
                if (action != GLFW_PRESS) return;
                // Double tap to run
                if ((float)glfwGetTime() - info->lastWPressed <= W_PRESS_SPACE) {
                    info->gameWorld->runningMode = true;
                } else {
                    info->lastWPressed = glfwGetTime();
                }
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(win, GLFW_TRUE);
                break;
            case GLFW_KEY_E:
                if (action != GLFW_PRESS) return;
                info->gameWorld->switchHotbars();
                break;
            case GLFW_KEY_F:
                if (action != GLFW_PRESS) return;
                info->gameWorld->toggleMode();
                break;
            case GLFW_KEY_G:
                if (action != GLFW_PRESS) return;
                // Printing debug information
                std::cout << "X: " << info->gameWorld->playerCamera.pos.x << " ";
                std::cout << "Y: " << info->gameWorld->playerCamera.pos.y << " ";
                std::cout << "Z: " << info->gameWorld->playerCamera.pos.z << "\n";
                std::cout << "Yaw: " << info->gameWorld->playerCamera.yaw << "\n";
                std::cout << "Pitch: " << info->gameWorld->playerCamera.pitch << "\n";
                std::cout << "Player Vertical Velocity: " << info->gameWorld->playerCamera.yVelocity << "\n";
                std::cout << "Ingame Time: " << info->gameWorld->worldTime << "\n";
                std::cout << "Exposure levels: " << info->exposureLevel << "\n";
                std::cout << "Illuminance: " << info->averageIlluminance << "\n";
                std::cout << "Current frame rate: " << info->frameRate << " frames per second\n\n";
                break;
            case GLFW_KEY_C:
                if (action != GLFW_PRESS) return;
                info->gameWorld->toggleCutscene();
                break;
            case GLFW_KEY_O:
                if (action != GLFW_PRESS) return;
                info->gameWorld->convertCurrWorldIntoData();
                break;
            case GLFW_KEY_I:
                if (action != GLFW_PRESS) return;
                info->gameWorld->toggleInstructions(!(info->gameWorld->getInstructionStatus()));
                break;

            case GLFW_KEY_TAB:
                if (action != GLFW_PRESS) return;
                if (!glfwGetWindowAttrib(win, GLFW_MAXIMIZED)) {
                    glfwMaximizeWindow(win);
                } else {
                    glfwRestoreWindow(win);
                }  
                break;
            case GLFW_KEY_COMMA:
                if (action != GLFW_PRESS) return;
                info->enableExperimental = !info->enableExperimental;
                break;
            case GLFW_KEY_F1:
                if (action != GLFW_PRESS) return;
                info->gameWorld->hideScreen = !info->gameWorld->hideScreen;
                break;
            case GLFW_KEY_LEFT_BRACKET:
                if (action != GLFW_PRESS) return;
                info->gameWorld->changeSeaLevel(-1);
                break;
            case GLFW_KEY_RIGHT_BRACKET:
                if (action != GLFW_PRESS) return;
                info->gameWorld->changeSeaLevel(1);
                break;
            case GLFW_KEY_R:
                if (action != GLFW_PRESS) return;
                info->hdrType++;
                info->hdrType %= TOTAL_TONE_MAPS;
                std::cout << "HDR set to ";
                switch (info->hdrType) {
                    case 0:
                        std::cout << "\"None\"\n";
                        break;
                    case 1:
                        std::cout << "\"Automatic Exposure Tone Mapping\"\n";
                        break;
                    case 2:
                        std::cout << "\"Filmic Tone Mapping\"\n";
                        break;
                    case 3:
                        std::cout << "\"Reinhard Tone Mapping\"\n";
                        break;
                    case 4:
                        std::cout << "\"Lotte Tone Mapping\"\n";
                        break;
                    case 5:
                        std::cout << "\"Unreal Tone Mapping\"\n";
                        break;
                    case 6:
                        std::cout << "\"Neg Tone Mapping\"\n";
                        break;
                }
                break;
            case GLFW_KEY_T:
                if (action != GLFW_PRESS) return;
                info->kernelType++;
                info->kernelType %= TOTAL_KERNELS;
                std::cout << "Filter set to ";
                switch (info->kernelType) {
                    case 0:
                        std::cout << "\"None\"\n";
                        break;
                    case 1:
                        std::cout << "\"Edge Kernal\"\n";
                        break;
                    case 2:
                        std::cout << "\"Emboss Kernal\"\n";
                        break;
                    case 3:
                        std::cout << "\"Sharpen Kernal\"\n";
                        break;
                    case 4:
                        std::cout << "\"Right Sobel Kernal\"\n";
                        break;
                    case 5:
                        std::cout << "\"Blur Kernal\"\n";
                        break;
                    case 6:
                        std::cout << "\"Pixelated\"\n";
                        break;
                }
                break;
        }
    });

    glfwSetScrollCallback(window, [](GLFWwindow *win, double xoffset, double yoffset) {
        pointerInformation *info = (pointerInformation *) glfwGetWindowUserPointer(win);

        if (yoffset > 0) {
            info->gameWorld->scrollHotbar(1);
        } else {
            info->gameWorld->scrollHotbar(-1);
        }
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int button, int action, int mods) {

        if (action == GLFW_RELEASE) return;

        pointerInformation *info = (pointerInformation *) glfwGetWindowUserPointer(win);

        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                info->gameWorld->leftClickDestroy(info->defaultShader);
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                info->gameWorld->rightClickPlace(info->defaultShader);
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                info->gameWorld->middleClickPick();
                break;
        }
    });

    auto xPos = gameWorld.terrain.size() / 2;
    auto zPos = gameWorld.terrain[0][0].size() / 2;
    gameWorld.playerCamera = player::createCamera(glm::vec3(xPos, 5, zPos), glm::vec3(xPos, -10.0f, zPos));
    gameWorld.cutsceneCamera = player::createCamera(glm::vec3(xPos, 5, zPos), glm::vec3(xPos, -10.0f, zPos));
    gameWorld.reflectionCamera = player::createCamera(glm::vec3(xPos, 5, zPos), glm::vec3(xPos, -10.0f, zPos));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /**
     * Setting up clicks
     */
    pointerInformation info;
    info.gameWorld = &gameWorld;
    info.defaultShader = &defaultShader;
    glfwSetWindowUserPointer(window, &info);

    float degrees = 90;

    gameWorld.updateBlocksToRender(true);
    
    /**
     * Creating post processing effects
     */

    // REGULAR UNTAMPERED SCENE
    GLuint untamperedFBO, untamperedTexID, untamperedRBO;
    glGenFramebuffers(1, &untamperedFBO);
    glBindFramebuffer(1, untamperedFBO);
    glGenRenderbuffers(1, &untamperedRBO);

    glGenTextures(1, &untamperedTexID);
    glBindTexture(GL_TEXTURE_2D, untamperedTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIN_WIDTH, WIN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindRenderbuffer(GL_RENDERBUFFER, untamperedRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIN_WIDTH, WIN_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, untamperedFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, untamperedTexID, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, untamperedRBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // END OF UNTAMPERED CREATION

    // REFLECTION UNTAMPERED SCENE
    GLuint waterReflectionFBO, waterReflectionTexID, waterReflectionRBO;
    glGenFramebuffers(1, &waterReflectionFBO);
    glBindFramebuffer(1, waterReflectionFBO);
    glGenRenderbuffers(1, &waterReflectionRBO);

    glGenTextures(1, &waterReflectionTexID);
    glBindTexture(GL_TEXTURE_2D, waterReflectionTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIN_WIDTH, WIN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindRenderbuffer(GL_RENDERBUFFER, waterReflectionRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIN_WIDTH, WIN_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, waterReflectionFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, waterReflectionTexID, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, waterReflectionRBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // END OF REFLECTION CREATION

    // FIRST STAGE OF BLOOM FRAME BUFFER
    GLuint onlyBloomFBO, onlyBloomTexID, onlyBloomRBO;
    glGenFramebuffers(1, &onlyBloomFBO);
    glBindFramebuffer(1, onlyBloomFBO);
    glGenRenderbuffers(1, &onlyBloomRBO);

    glGenTextures(1, &onlyBloomTexID);
    glBindTexture(GL_TEXTURE_2D, onlyBloomTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIN_WIDTH, WIN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindRenderbuffer(GL_RENDERBUFFER, onlyBloomRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIN_WIDTH, WIN_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, onlyBloomFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, onlyBloomTexID, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, onlyBloomRBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // END OF  FIRST STAGE OF BLOOM CREATION

    // PINGPONG FRAME BUFFERS
    GLuint pingpongFBO[2], pingpongBuffer[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    
    for (GLuint i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIN_WIDTH, WIN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer not complete!\n";
        }
            
    }

    // Final frame buffer
    GLuint finalFrameFBO, finalFrameTexID;
    glGenFramebuffers(1, &finalFrameFBO);
    glGenTextures(1, &finalFrameTexID);

    glBindFramebuffer(GL_FRAMEBUFFER, finalFrameFBO);
    glBindTexture(GL_TEXTURE_2D, finalFrameTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIN_WIDTH, WIN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalFrameTexID, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!\n";
    }

    // DEPTH MAP
    GLuint depthMapFBO, depthMapTexID;
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMapTexID);
    glBindTexture(GL_TEXTURE_2D, depthMapTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // Attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // END OF DEPTH MAP CREATION

    blurShader.setInt("screenTexture", 0);
    normalShader.setInt("uTex", 0);

    float totalTime = 0.0f;
    float totalFrames = 0.0f;
    float blendValue = 0.0f;
    float waterWaveCycle = 0.0f;

    glfwShowWindow(window);
    glfwFocusWindow(window);

    float worldSize = gameWorld.getWorldSize();
    glm::vec3 *playerPosPtr = &gameWorld.playerCamera.pos;
    // RENDER LOOP
    while (!glfwWindowShouldClose(window)) {
        
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        float dt = utility::time_delta();

        waterWaveCycle = fmod(waterWaveCycle + (dt / 3.0f), M_PI / 6.0f);

        // Calculating frames per second
        totalTime += dt;
        totalFrames++;
        if (totalTime >= 1) {
            info.frameRate = totalFrames / totalTime;
            totalFrames = 0;
            totalTime = 0;
        }

        // Moving the day cycle around
        if (gameWorld.cutsceneEnabled) {
            degrees += 20.0f * dt;
        } else {
            degrees += 0.125f * dt;
        }
        if (degrees >= 360) {
            gameWorld.updateMoonPhase();
            degrees -= 360.0f;
        }

        // Changing and updating where the sun will be
        // sunPosition = glm::vec3(gameWorld.getCurrCamera()->pos.x + (renderDistance) * glm::cos(glm::radians(degrees)), gameWorld.getCurrCamera()->pos.y + (renderDistance - 10) * glm::sin(glm::radians(degrees)), gameWorld.getCurrCamera()->pos.z);
        auto sunDistance = gameWorld.getSkyRadius();
        glm::vec3 sunPosition = glm::vec3(playerPosPtr->x + (sunDistance) * glm::cos(glm::radians(degrees)), playerPosPtr->y + (sunDistance - 10) * glm::sin(glm::radians(degrees)), playerPosPtr->z);
        glm::vec3 sunPosOpp = glm::vec3(playerPosPtr->x + (sunDistance) * glm::cos(glm::radians(degrees + 180)), playerPosPtr->y + (sunDistance - 10) * glm::sin(glm::radians(degrees + 180)), playerPosPtr->z);

        defaultShader.sun_light_dir = glm::normalize(sunPosOpp - sunPosition);
        defaultShader.changeSunlight(degrees);

        // Get input or animate cutscene depending on the cutscene status
        if (gameWorld.getCutsceneStatus()) {
            gameWorld.animateCutscene();
        } else {
            gameWorld.updatePlayerPositions(window, dt, &defaultShader);
        }

        gameWorld.updateSunPosition(degrees, dt);
    
        // Rendering the scene with an ortho camera
        shadowShader.activate();
        glm::mat4 lightProjection, lightView, lightSpaceMatrix;

        // Calculating light view
        float nearPlane = 0.0f, farPlane = 3 * sunDistance;
        lightProjection = glm::ortho(-worldSize / 2 - 5.0f, worldSize / 2 + 5.0f, -worldSize / 2 - 5.0f, worldSize / 2 + 5.0f, nearPlane, farPlane);
        lightView = glm::lookAt(sunPosition, {playerPosPtr->x, playerPosPtr->y, playerPosPtr->z}, glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        glUniformMatrix4fv(shadowShader.light_proj_loc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        // Drawing the world in the eyes of the shadows
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            gameWorld.drawWorld(shadowShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render scene as normal, using the shadow map as the 3rd texture
        // Change the view port back to normal
        
        glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

        defaultShader.activate();

        auto view_proj = defaultShader.projection * gameWorld.getCurrCamera()->get_view();
        glUniformMatrix4fv(defaultShader.view_proj_loc, 1, GL_FALSE, glm::value_ptr(view_proj));
        glUniformMatrix4fv(defaultShader.light_proj_loc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        std::vector<GLuint> twoFrameBuffers = {waterReflectionFBO, untamperedFBO};
        bool reflection = true;
        // Drawing world via reflection and then via untampered
        for (auto currFBO : twoFrameBuffers) {
            glBindFramebuffer(GL_FRAMEBUFFER, currFBO);
                if (reflection) {
                    gameWorld.useReflectionCam = false;
                    gameWorld.updateReflectionCamera();
                    gameWorld.useReflectionCam = true;
                } else {
                    gameWorld.useReflectionCam = false;
                }
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                defaultShader.activate();
                defaultShader.setMat4("uViewProj", defaultShader.projection * gameWorld.getCurrCamera()->get_view());
                defaultShader.setInt("forceBlack", false);
                defaultShader.setInt("affectedByShadows", true);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, depthMapTexID);
                gameWorld.drawWorld(defaultShader, false);

                if (info.enableExperimental) {
                    cubeReflectShader.activate();
                    gameWorld.drawShinyTerrain(
                        view_proj,
                        cubeReflectShader,
                        defaultShader,
                        skyboxShader,
                        {dayNightCalculator.prevTex, dayNightCalculator.currTex},
                        blendValue,
                        {WIN_WIDTH, WIN_HEIGHT},
                        dayNightCalculator.practice
                    );
                } else {
                    gameWorld.drawShinyTerrainNormally(glm::mat4(1.0f), defaultShader, false);
                }

                // Drawing skybox
                blendValue = dayNightCalculator.updatePrevCurr(gameWorld.worldTime);
                gameWorld.drawSkyBox(skyboxShader, defaultShader.projection, dayNightCalculator.prevTex, dayNightCalculator.currTex, blendValue);

                // Drawing the water only for the non reflection fbo
                if (!reflection) {
                    waterShader.activate();
                    waterShader.setInt("uReflection", 10);
                    waterShader.setMat4("uViewProj", view_proj);
                    waterShader.setMat4("uModel", utility::findModelMatrix(gameWorld.seaSurface.translation, gameWorld.seaSurface.scale, gameWorld.seaSurface.rotation));
                    gameWorld.seaSurface.textureID = waterCalculator.getFrame(dt);
                    glActiveTexture(GL_TEXTURE10);
                    glBindTexture(GL_TEXTURE_2D, waterReflectionTexID);
                    scene::drawElement(&gameWorld.seaSurface, glm::mat4(1.0f), waterShader);
                } else if (!gameWorld.cutsceneEnabled) {
                    // Draw the player
                    defaultShader.activate();
                    gameWorld.player.positionInWorld.translation = gameWorld.playerCamera.pos;
                    gameWorld.player.positionInWorld.translation.y -= 1.0f;
                    scene::drawElement(&gameWorld.player.positionInWorld, glm::mat4(1.0f), defaultShader);
                }
    
                // Drawing transparent block
                defaultShader.activate();
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, depthMapTexID);
                
                gameWorld.drawTransTerrain(glm::mat4(1.0f), defaultShader, false);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            reflection = false;
        }
        

        // Drawing the world again but with the bloom on only
        glBindFramebuffer(GL_FRAMEBUFFER, onlyBloomFBO);
            glClearColor(0, 0, 0, 1);
            defaultShader.activate();
            defaultShader.setInt("forceBlack", true);
            gameWorld.drawWorld(defaultShader, true);
            gameWorld.drawShinyTerrainNormally(glm::mat4(1.0f), defaultShader, false);
            defaultShader.setInt("forceBlack", false);
            // Drawing the water
            waterShader.activate();
            waterShader.setMat4("uViewProj", view_proj);
            waterShader.setMat4("uModel", utility::findModelMatrix(gameWorld.seaSurface.translation, gameWorld.seaSurface.scale, gameWorld.seaSurface.rotation));
            gameWorld.seaSurface.textureID = waterCalculator.getFrame(dt);
            scene::drawElement(&gameWorld.seaSurface, glm::mat4(1.0f), waterShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Drawing the recently bloom only scene to the ping pong frame
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
            normalShader.activate();
            normalShader.setInt("kernelType", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, onlyBloomTexID);
            utility::renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Blurs the scene in pingpong buffer
        bool horizontal = true;
        blurShader.activate();
        for (int i = 0; i < BLOOM_INTENSITY; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
            utility::renderQuad();
            horizontal = !horizontal;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Drawing scene as usual with HDR + Bloom
        glBindFramebuffer(GL_FRAMEBUFFER, finalFrameFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            hdrShader.activate();
            hdrShader.setInt("isUnderwater", gameWorld.seaSurface.translation.y > gameWorld.playerCamera.pos.y);
            hdrShader.setFloat("cycle", waterWaveCycle);
            hdrShader.setInt("hdr", info.hdrType);
            hdrShader.setFloat("exposure", info.exposureLevel);
            hdrShader.setInt("scene", 0);
            hdrShader.setInt("bloomBlur", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, untamperedTexID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
            utility::renderQuad();

            // Drawing the HUD
            glActiveTexture(GL_TEXTURE0);
            if (!gameWorld.cutsceneEnabled) {
                glClear(GL_DEPTH_BUFFER_BIT);
                defaultShader.activate();
                defaultShader.setInt("affectedByShadows", false);
                gameWorld.drawScreen(glm::mat4(1.0f), defaultShader);
            }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Auto adjusting the exposure values
        if (info.hdrType == 1) {
            info.averageIlluminance = utility::findIlluminance(WIN_WIDTH, WIN_HEIGHT, finalFrameTexID);
            auto correctExposure = 0.5 / info.averageIlluminance * 1.8;
            if (utility::roundUp(info.exposureLevel, 2) != utility::roundUp(correctExposure, 2)) {
                info.exposureLevel = utility::lerp(info.exposureLevel, correctExposure, 0.05f);
            }
        } else {
            info.exposureLevel = 2.0f;
        }

        // Drawing the final frame
        utility::resizeWindow(WIN_WIDTH, WIN_HEIGHT, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        normalShader.activate();
        normalShader.setInt("kernelType", info.kernelType);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, finalFrameTexID);
        utility::renderQuad();

        glfwSwapBuffers(window);
        glfwPollEvents();

        // not entirely correct as a frame limiter, but close enough
        // it would be more correct if we knew how much time this frame took to render
        // and calculated the distance to the next "ideal" time to render and only slept that long
        // the current way just always sleeps for 16.67ms, so in theory we'd drop frames
        if (frameLimiter != 0) std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(1000.f / 60));
    }
    
    std::cout << "Terminating program, please standby as everything gets wiped out...\n";
    // deleting the whole window also removes the opengl context, freeing all our memory in one fell swoop.
    defaultShader.deleteProgram();
    hdrShader.deleteProgram();
    normalShader.deleteProgram();
    blurShader.deleteProgram();
    skyboxShader.deleteProgram();
    gameWorld.destroyEverthing();
    chicken3421::delete_opengl_window(window);

    chicken3421::delete_framebuffer(finalFrameFBO);
    texture_2d::destroy(finalFrameTexID);
    chicken3421::delete_framebuffer(untamperedFBO);
    chicken3421::delete_renderbuffer(untamperedRBO);
    texture_2d::destroy(untamperedTexID);
    chicken3421::delete_framebuffer(onlyBloomFBO);
    chicken3421::delete_renderbuffer(onlyBloomRBO);
    texture_2d::destroy(onlyBloomTexID);
    chicken3421::delete_framebuffer(pingpongFBO[0]);
    chicken3421::delete_framebuffer(pingpongFBO[1]);
    texture_2d::destroy(pingpongBuffer[0]);
    texture_2d::destroy(pingpongBuffer[1]);

    std::cout << "Good bye!\n";

    return EXIT_SUCCESS;
}
