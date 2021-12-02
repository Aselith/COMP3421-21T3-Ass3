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
#include <ass3/loader.hpp>

#include <iostream>
#include <cmath>

const int WIN_HEIGHT = 720;
const int WIN_WIDTH = 1280;
const int SHADOW_WIDTH = 8192;
const int SHADOW_HEIGHT = 8192;
const float W_PRESS_SPACE = 0.4f;
const int BLOOM_INTENSITY = 12;
const int TOTAL_TONE_MAPS = 6;
const int TOTAL_KERNELS = 8;
const int REFLECTION_REFRESH_RATE = 3;

// 0 -> Basic flat dirt world
// 1 -> Wooly world
// 2 -> Iron world
// 3 -> Classic Sky Block
// 4 -> Parkour
// 5 -> Parkour v2

struct pointerInformation {
    scene::world *gameWorld;
    renderer::renderer_t *defaultShader;
    float frameRate = 0, lastWPressed = 0;
    glm::vec3 avgColor = glm::vec3(0.0f);
    float exposureLevel = 10.0f, averageIlluminance = 0;
    GLint hdrType = 1, kernelType = 0, enableExperimental = 0;
};

struct dayNightTextureSystem {
    private:
    GLuint day = texture_2d::loadCubemap("./res/textures/skybox/day");
    GLuint sunset = texture_2d::loadCubemap("./res/textures/skybox/sunset");
    GLuint night = texture_2d::loadCubemap("./res/textures/skybox/night");
    GLuint sunrise = texture_2d::loadCubemap("./res/textures/skybox/sunrise");
    glm::vec3 dayRGB = glm::vec3(139.0f, 181.0f, 253.0f) / 255.0f;
    glm::vec3 sunriseRGB = glm::vec3(247.0f, 205.0f, 93.0f) / 255.0f;
    glm::vec3 sunsetRGB = glm::vec3(223.0f, 170.0f, 100.0f) / 255.0f;
    glm::vec3 nightRGB = glm::vec3(16.0f, 16.0f, 16.0f) / 255.0f;
    
    public:
    GLuint practice = texture_2d::loadCubemap("./res/textures/premadeSkybox");
    GLuint prevTex, currTex;
    glm::vec3 skyColor = {0, 0, 0};

    float updatePrevCurr(float dayProgress) {
        dayProgress = fmod(dayProgress, 1.0f);
        if (utility::isInRange(dayProgress, 0.0f, 0.2f)) {
            prevTex = sunrise;
            currTex = day;
            skyColor = glm::mix(sunriseRGB, dayRGB, dayProgress / 0.2f);
            return dayProgress / 0.2f;
        } else if (utility::isInRange(dayProgress, 0.2f, 0.3f)) {
            prevTex = day;
            currTex = day;
            skyColor = dayRGB;
            return 1.0f;
        } else if (utility::isInRange(dayProgress, 0.3f, 0.5f)) {
            prevTex = day;
            currTex = sunset;
            skyColor = glm::mix(dayRGB, sunsetRGB, (dayProgress - 0.3f) / 0.2f);
            return (dayProgress - 0.3f) / 0.2f;
        } else if (utility::isInRange(dayProgress, 0.5f, 0.7f)) {
            prevTex = sunset;
            currTex = night;
            skyColor = glm::mix(sunsetRGB, nightRGB, (dayProgress - 0.5f) / 0.2f);
            return (dayProgress - 0.5f) / 0.2f;
        } else if (utility::isInRange(dayProgress, 0.7f, 0.8f)) {
            prevTex = night;
            currTex = night;
            skyColor = nightRGB;
            return 1.0f;
        } else {
            prevTex = night;
            currTex = sunrise;
            skyColor = glm::mix(nightRGB, sunriseRGB, (dayProgress - 0.8f) / 0.2f);
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
    const float waveSpeed = 0.06;
    GLuint frames[32];
    GLuint blackTex, dudvMap, normalMap;
    GLuint currFrame = 0;
    GLfloat frame;
    GLfloat waterWaveCycle = 0.0f, rippleCycle = 0.0f;
    std::string filepath = "./res/textures/water/water_";
    waterTextures() {
        for (int i = 0; i < 32; i++) {
            frames[i] = texture_2d::init(filepath + std::to_string(i) + ".png");
        }
        blackTex = texture_2d::init("./res/textures/water/water_-1.png");
        dudvMap = texture_2d::init("./res/textures/water/dudv.png", texture_2d::params_t{GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR});
        normalMap = texture_2d::init("./res/textures/water/normal.png", texture_2d::params_t{GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR});
    }

    GLuint getFrame(float dt) {
        frame += dt;
        waterWaveCycle = fmod(waterWaveCycle + (dt / 3.0f), M_PI / 6.0f);
        rippleCycle = fmod(rippleCycle + (dt * waveSpeed), 80.0f);
        if (frame > frameLen) {
            frame -= frameLen;
            currFrame++;
            currFrame %= 32;
        }
        return frames[currFrame];
    }
};

int main() {
    srand(time(nullptr));
    // Printing welcome message
    int worldType = 0, renderDistance = 0, worldWidth = 0, frameLimiter = 1;
    pointerInformation info;
    std::cout << "\n\u001B[34mWelcome to a clone of Minecraft, created by z5309206 for COMP3421 ass3 21T3 UNSW!\n\n\u001B[0m";
    loader::printOutPresets();
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
    std::cout << "Which reflections for Mirror Block? [0 for \"None\" | 1 for \"Premade Cubemap\" | 2 for \"Realtime Cubemap\"] (You can change this ingame with F2): ";
    std::cin >> info.enableExperimental;
    info.enableExperimental = glm::clamp(info.enableExperimental, 0, 2);

    GLFWwindow *window = chicken3421::make_opengl_window(WIN_WIDTH, WIN_HEIGHT, "COMP3421 21T3 Assignment 2 [Minecraft: Clone Simulator]");
    chicken3421::image_t faviconImage = chicken3421::load_image("./res/textures/favicon.png", false);
    GLFWimage favicon = {faviconImage.width, faviconImage.height, (unsigned char *) faviconImage.data};
    glfwSetWindowIcon(window, 1, &favicon);
    chicken3421::delete_image(faviconImage);

    std::vector<scene::miniBlockData> listOfBlocks;
    loader::loadWorld(&listOfBlocks, worldType);
    std::cout << "\n";
    // Creating worlds

    // Setting up all the render informations
    renderer::renderer_t defaultShader;
    defaultShader.createProgram("default");
    defaultShader.initialise(WIN_WIDTH, WIN_HEIGHT);

    scene::world gameWorld(listOfBlocks, renderDistance, worldWidth, &defaultShader);
    if (worldType == 3) {
        // Turns off sea for skyblock
        gameWorld.changeSeaLevel(-1);
    } else if (worldType == 5) {
        // Moves the water level up for parkour course map 2
        std::cout << "\nFind the single lone Marccoin hidden in the map!\n";
        gameWorld.seaSurface.translation.y = 0.501;
    }

    // Creating different shaders

    renderer::renderer_t shadowShader;
    shadowShader.createProgram("shadow");
    shadowShader.setUpShadow();
    
    renderer::renderer_t blurShader;
    blurShader.createProgram("blur");
    renderer::renderer_t finalFrameShader;
    finalFrameShader.createProgram("finalFrame");

    renderer::renderer_t hdrShader;
    hdrShader.createProgram("hdrBloom");

    renderer::renderer_t skyboxShader;
    skyboxShader.createProgram("skybox");

    renderer::renderer_t cubeReflectShader;
    cubeReflectShader.createProgram("cubeReflection");

    renderer::renderer_t waterShader;
    waterShader.createProgram("water");

    renderer::renderer_t copyFrameShader;
    copyFrameShader.createProgram("copyFrame");

    renderer::renderer_t particleShader;
    particleShader.createProgram("particle");

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
                std::cout << "Sea level: " << info->gameWorld->seaSurface.translation.y << "\n";
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
            case GLFW_KEY_LEFT_BRACKET:
                if (action == GLFW_RELEASE) return;
                info->gameWorld->changeSeaLevel(-1);
                break;
            case GLFW_KEY_RIGHT_BRACKET:
                if (action == GLFW_RELEASE) return;
                info->gameWorld->changeSeaLevel(1);
                break;
            case GLFW_KEY_F1:
                if (action != GLFW_PRESS) return;
                info->gameWorld->hideScreen = !info->gameWorld->hideScreen;
                break;
            case GLFW_KEY_F2:
                if (action != GLFW_PRESS) return;
                info->enableExperimental = (info->enableExperimental + 1) % 3;
                break;
            case GLFW_KEY_F3:
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
                }
                break;
            case GLFW_KEY_F4:
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
                    case 7:
                        std::cout << "\"Neg\"\n";
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
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_CLIP_DISTANCE0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glAlphaFunc(GL_LEQUAL, 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /**
     * Setting up clicks
     */
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
    utility::createFramebuffers(&untamperedFBO, &untamperedTexID, &untamperedRBO, WIN_WIDTH, WIN_HEIGHT);
    // END OF UNTAMPERED CREATION

    // REFLECTION UNTAMPERED SCENE
    GLuint waterReflectionFBO, waterReflectionTexID, waterReflectionRBO;
    utility::createFramebuffers(&waterReflectionFBO, &waterReflectionTexID, &waterReflectionRBO, WIN_WIDTH, WIN_HEIGHT);
    // END OF REFLECTION CREATION

    // REFRACTION UNTAMPERED SCENE
    GLuint waterRefractionFBO, waterRefractionTexID, waterRefractionRBO;
    utility::createFramebuffers(&waterRefractionFBO, &waterRefractionTexID, &waterRefractionRBO, WIN_WIDTH, WIN_HEIGHT);
    // END OF REFRACTION CREATION

    // FIRST STAGE OF BLOOM FRAME BUFFER
    GLuint onlyBloomFBO, onlyBloomTexID, onlyBloomRBO;
    utility::createFramebuffers(&onlyBloomFBO, &onlyBloomTexID, &onlyBloomRBO, WIN_WIDTH, WIN_HEIGHT);
    // END OF FIRST STAGE OF BLOOM CREATION

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
    utility::createFramebuffers(&finalFrameFBO, &finalFrameTexID, WIN_WIDTH, WIN_HEIGHT);
    //

    // Previous frame buffer for temporal effects
    GLuint temporalFrameAFBO, temporalFrameATexID;
    utility::createFramebuffers(&temporalFrameAFBO, &temporalFrameATexID, WIN_WIDTH, WIN_HEIGHT);
    GLuint temporalFrameBFBO, temporalFrameBTexID;
    utility::createFramebuffers(&temporalFrameBFBO, &temporalFrameBTexID, WIN_WIDTH, WIN_HEIGHT);

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
    finalFrameShader.setInt("uTex", 0);

    float totalTime = 0.0f;
    float totalFrames = 0.0f;
    float blendValue = 0.0f;
    unsigned int reflectionFrames = 0;
    bool firstPass = true;

    glfwShowWindow(window);
    glfwFocusWindow(window);

    float worldSize = gameWorld.getWorldSize();
    glm::vec3 *playerPosPtr = &gameWorld.playerCamera.pos;
    glm::vec4 clipPlane;

    glm::mat4 lightProjection, lightView, lightSpaceMatrix;
    auto sunDistance = gameWorld.getSkyRadius();
    float nearPlane = 0.0f, farPlane = 3 * sunDistance;
    lightProjection = glm::ortho(-worldSize / 2 - 5.0f, worldSize / 2 + 5.0f, -worldSize / 2 - 5.0f, worldSize / 2 + 5.0f, nearPlane, farPlane);

    std::vector<GLuint> framebufferList = {waterReflectionFBO, waterRefractionFBO, untamperedFBO};


    // RENDER LOOP
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        float dt = utility::time_delta();

        gameWorld.spawnBlockParticles();

        // Calculating frames per second
        totalTime += dt;
        totalFrames++;
        if (totalTime >= 1) {
            info.frameRate = totalFrames / totalTime;
            totalFrames = 0;
            totalTime = 0;
        }

        // Get input or animate cutscene depending on the cutscene status
        if (gameWorld.getCutsceneStatus()) {
            gameWorld.animateCutscene();
        } else {
            gameWorld.updatePlayerPositions(window, dt, &defaultShader);
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

        gameWorld.updateSunPosition(degrees, dt);

        // Changing and updating where the sun will be
        // sunPosition = glm::vec3(gameWorld.getCurrCamera()->pos.x + (renderDistance) * glm::cos(glm::radians(degrees)), gameWorld.getCurrCamera()->pos.y + (renderDistance - 10) * glm::sin(glm::radians(degrees)), gameWorld.getCurrCamera()->pos.z);
        glm::vec3 sunPosition = glm::vec3(playerPosPtr->x + (sunDistance) * glm::cos(glm::radians(degrees)), playerPosPtr->y + (sunDistance - 10) * glm::sin(glm::radians(degrees)), playerPosPtr->z);
        glm::vec3 sunPosOpp = glm::vec3(playerPosPtr->x + (sunDistance) * glm::cos(glm::radians(degrees + 180)), playerPosPtr->y + (sunDistance - 10) * glm::sin(glm::radians(degrees + 180)), playerPosPtr->z);

        defaultShader.sun_light_dir = glm::normalize(sunPosOpp - sunPosition);
        defaultShader.changeSunlight(degrees);
    
        // Rendering the scene with an ortho camera
        shadowShader.activate();

        // Calculating light view
        lightView = glm::lookAt(sunPosition, {playerPosPtr->x, playerPosPtr->y, playerPosPtr->z}, glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        glUniformMatrix4fv(shadowShader.light_proj_loc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        // Drawing the world in the eyes of the shadows
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            gameWorld.drawWorld(shadowShader, false, false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render scene as normal, using the shadow map as the 3rd texture
        // Change the view port back to normal
        
        glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

        defaultShader.activate();

        auto view_proj = defaultShader.projection * gameWorld.getCurrCamera()->get_view();
        glUniformMatrix4fv(defaultShader.view_proj_loc, 1, GL_FALSE, glm::value_ptr(view_proj));
        glUniformMatrix4fv(defaultShader.light_proj_loc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        reflectionFrames = (reflectionFrames + 1) % REFLECTION_REFRESH_RATE;
        if (info.enableExperimental > 0 && reflectionFrames == 0) {

            // Updating the shiny terrain
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            gameWorld.updateShinyTerrain(
                defaultShader,
                dayNightCalculator.skyColor,
                {WIN_WIDTH, WIN_HEIGHT}
            );
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

        // Drawing world via reflection, refraction and then via untampered
        
        for (auto currFBO : framebufferList) {
            glBindFramebuffer(GL_FRAMEBUFFER, currFBO);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                defaultShader.activate();
                gameWorld.drawCelestials = false;
                if (currFBO == waterReflectionFBO) {
                    gameWorld.useReflectionCam = false;
                    gameWorld.updateReflectionCamera();
                    gameWorld.useReflectionCam = true;
                    if (gameWorld.isUnderwater()) {
                        // Clip above if underwater
                        clipPlane = glm::vec4(0, -1, 0, gameWorld.seaSurface.translation.y);
                    } else{
                        // Clip below if not under water
                        clipPlane = glm::vec4(0, 1, 0, -gameWorld.seaSurface.translation.y);
                    }
                } else if (currFBO == waterRefractionFBO) {
                    if (!gameWorld.isUnderwater()) {
                        // Clip above if not underwater
                        clipPlane = glm::vec4(0, -1, 0, gameWorld.seaSurface.translation.y);
                    } else{
                        // Clip below if under water
                        clipPlane = glm::vec4(0, 1, 0, -gameWorld.seaSurface.translation.y);
                    }
                    gameWorld.useReflectionCam = false;
                } else {
                    gameWorld.drawCelestials = true;
                    clipPlane = glm::vec4(0, 1, 0, -scene::VOID_LEVEL);
                }
                defaultShader.setVec4("plane", clipPlane);
                defaultShader.setMat4("uViewProj", defaultShader.projection * gameWorld.getCurrCamera()->get_view());
                defaultShader.setInt("forceBlack", false);
                defaultShader.setInt("affectedByShadows", true);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, depthMapTexID);
                gameWorld.drawWorld(defaultShader, false, currFBO == untamperedFBO);

                
                if (info.enableExperimental > 0 && currFBO == untamperedFBO) {
                    cubeReflectShader.activate();
                    gameWorld.drawShinyTerrain(
                        view_proj,
                        cubeReflectShader,
                        (info.enableExperimental == 1) ? dayNightCalculator.practice : 0
                    );
                } else {
                    gameWorld.drawShinyTerrainNormally(glm::mat4(1.0f), defaultShader, false);
                }
                defaultShader.activate();
                

                // Drawing skybox
                blendValue = dayNightCalculator.updatePrevCurr(gameWorld.worldTime);
                gameWorld.drawSkyBox(skyboxShader, defaultShader.projection, dayNightCalculator.prevTex, dayNightCalculator.currTex, blendValue);

                // Drawing the water only for the last draw of the world
                // DRAWWATER 
                if (currFBO == untamperedFBO) {
                    waterShader.activate();
                    waterShader.setVec3("uCameraPos", gameWorld.getCurrCamera()->pos);
                    int i = 0;
                    for (auto light : defaultShader.allLightSources) {
                        waterShader.setVec3("allLights[" + std::to_string(i) + "].specular", light.specular);
                        waterShader.setVec3("allLights[" + std::to_string(i) + "].position", light.position);
                        i++;
                    }
                    waterShader.setFloat("waterLevel", (GLfloat)gameWorld.seaSurface.translation.y);
                    waterShader.setVec2("rippleCycle", {waterCalculator.rippleCycle * 1000.0f, 1000.0f});
                    waterShader.setVec2("uPitchFactor", {abs(gameWorld.playerCamera.pitch) * 1000.0f, 90000.0f});
                    waterShader.setInt("uTex", 9);
                    waterShader.setInt("uReflection", 10);
                    waterShader.setInt("uRefraction", 11);
                    waterShader.setInt("uDuDvMap", 12);
                    waterShader.setInt("uNormalMap", 13);
                    waterShader.setMat4("uViewProj", view_proj);
                    waterShader.setMat4("uModel", utility::findModelMatrix(gameWorld.seaSurface.translation, gameWorld.seaSurface.scale, gameWorld.seaSurface.rotation));
                    
                    glActiveTexture(GL_TEXTURE9);
                    glBindTexture(GL_TEXTURE_2D, waterCalculator.getFrame(dt));
                    glActiveTexture(GL_TEXTURE10);
                    glBindTexture(GL_TEXTURE_2D, waterReflectionTexID);
                    glActiveTexture(GL_TEXTURE11);
                    glBindTexture(GL_TEXTURE_2D, waterRefractionTexID);
                    glActiveTexture(GL_TEXTURE12);
                    glBindTexture(GL_TEXTURE_2D, waterCalculator.dudvMap);
                    glActiveTexture(GL_TEXTURE13);
                    glBindTexture(GL_TEXTURE_2D, waterCalculator.normalMap);
                    scene::drawElement(&gameWorld.seaSurface, glm::mat4(1.0f), waterShader);
                    glActiveTexture(GL_TEXTURE0);
                } else if (!gameWorld.cutsceneEnabled && currFBO == waterReflectionFBO) {
                    // Draw the player
                    defaultShader.activate();
                    gameWorld.player.positionInWorld.translation = gameWorld.playerCamera.pos;
                    scene::drawElement(&gameWorld.player.positionInWorld, glm::mat4(1.0f), defaultShader);
                }
                

                // Drawing transparent block
                defaultShader.activate();
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, depthMapTexID);

                if (!gameWorld.cutsceneEnabled) gameWorld.drawHand(defaultShader);
                
                // Drawing cloud
                scene::drawElement(&gameWorld.clouds, glm::mat4(1.0f), defaultShader);

                gameWorld.drawTransTerrain(glm::mat4(1.0f), defaultShader, false);

                // Draw particles
                particleShader.activate();
                particleShader.setInt("forceBlack", false);
                particleShader.setVec4("plane", clipPlane);
                defaultShader.changeSunlight(degrees);
                particleShader.setVec3("uSun.ambient", defaultShader.sun_light_color);
                particleShader.setVec3("uSun.direction", defaultShader.sun_light_dir);
                gameWorld.drawParticles(particleShader, defaultShader.projection, dt);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        

        // Drawing the world again but with the bloom on only
        glBindFramebuffer(GL_FRAMEBUFFER, onlyBloomFBO);
            glClearColor(0, 0, 0, 1);
            defaultShader.activate();
            defaultShader.setMat4("uViewProj", defaultShader.projection * gameWorld.getCurrCamera()->get_view());
            defaultShader.setInt("forceBlack", true);
            gameWorld.drawWorld(defaultShader, true, true);
            gameWorld.drawShinyTerrainNormally(glm::mat4(1.0f), defaultShader, true);
            scene::drawBlock(&gameWorld.clouds, glm::mat4(1.0f), defaultShader, true);
            if (!gameWorld.cutsceneEnabled) gameWorld.drawHand(defaultShader);
            defaultShader.setInt("forceBlack", false);
            // Drawing the water
            waterShader.activate();
            waterShader.setMat4("uViewProj", view_proj);
            waterShader.setMat4("uModel", utility::findModelMatrix(gameWorld.seaSurface.translation, gameWorld.seaSurface.scale, gameWorld.seaSurface.rotation));
            // Setting up all the textures of the water to be black
            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D, waterCalculator.blackTex);
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D, waterCalculator.blackTex);
            glActiveTexture(GL_TEXTURE11);
            glBindTexture(GL_TEXTURE_2D, waterCalculator.blackTex);
            glActiveTexture(GL_TEXTURE12);
            glBindTexture(GL_TEXTURE_2D, waterCalculator.blackTex);
            glActiveTexture(GL_TEXTURE13);
            glBindTexture(GL_TEXTURE_2D, waterCalculator.normalMap);

            scene::drawElement(&gameWorld.seaSurface, glm::mat4(1.0f), waterShader);
            // Drawing particles
            particleShader.activate();
            particleShader.setInt("forceBlack", true);
            particleShader.setVec4("plane", clipPlane);
            particleShader.setVec3("uSun.ambient", defaultShader.sun_light_color);
            particleShader.setVec3("uSun.direction", defaultShader.sun_light_dir);
            gameWorld.drawParticles(particleShader, defaultShader.projection, dt, false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Copy the frame over to the ping pong FBO
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
            copyFrameShader.activate();
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
            hdrShader.setInt("isUnderwater", gameWorld.isUnderwater());
            hdrShader.setFloat("cycle", waterCalculator.waterWaveCycle);
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

        info.avgColor = utility::findAvgColor(WIN_WIDTH, WIN_HEIGHT, finalFrameTexID);
        info.averageIlluminance = 0.2126f * info.avgColor.r + 0.7152f * info.avgColor.g + 0.0722f * info.avgColor.b;
        if (info.averageIlluminance != 0 && info.averageIlluminance < 50) {
            auto correctExposure = 0.5 / info.averageIlluminance * 1.8;
            if (utility::roundUp(info.exposureLevel, 2) != utility::roundUp(correctExposure, 2)) {
                info.exposureLevel = utility::lerp(info.exposureLevel, correctExposure, 0.05f);
            }
        }
        // Drawing the final frame, mixed with the previous frame
        utility::resizeWindow(WIN_WIDTH, WIN_HEIGHT, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        finalFrameShader.activate();
        finalFrameShader.setInt("kernelType", info.kernelType);
        finalFrameShader.setInt("uTempA", 10);
        finalFrameShader.setInt("uTempB", 11);
        finalFrameShader.setInt("isUnderwater", gameWorld.isUnderwater());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, finalFrameTexID);
        if (firstPass) {
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D, finalFrameTexID);
        } else {
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D, temporalFrameATexID);
            glActiveTexture(GL_TEXTURE11);
            glBindTexture(GL_TEXTURE_2D, temporalFrameBTexID);
        }
        utility::renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // not entirely correct as a frame limiter, but close enough
        // it would be more correct if we knew how much time this frame took to render
        // and calculated the distance to the next "ideal" time to render and only slept that long
        // the current way just always sleeps for 16.67ms, so in theory we'd drop frames
        if (frameLimiter != 0) std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(1000.f / 60));

        firstPass = false;

        // Drawing to previous frame only if under water
        if (!gameWorld.isUnderwater()) continue;
        glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, temporalFrameBFBO);
            glClear(GL_DEPTH_BITS | GL_COLOR_BUFFER_BIT);
            copyFrameShader.activate();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, temporalFrameATexID);
            utility::renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, temporalFrameAFBO);
            glClear(GL_DEPTH_BITS | GL_COLOR_BUFFER_BIT);
            copyFrameShader.activate();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, finalFrameTexID);
            utility::renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    std::cout << "Terminating program, please standby as everything gets wiped out...\n";
    // deleting the whole window also removes the opengl context, freeing all our memory in one fell swoop.
    defaultShader.deleteProgram();
    hdrShader.deleteProgram();
    finalFrameShader.deleteProgram();
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
