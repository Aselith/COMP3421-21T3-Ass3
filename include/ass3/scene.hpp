#ifndef COMP3421_SCENE_OBJECT_HPP
#define COMP3421_SCENE_OBJECT_HPP

#include "static_mesh.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <ass3/utility.hpp>
#include <ass3/frustum.hpp>
#include <ass3/shapes.hpp>
#include <ass3/player.hpp>
#include <ass3/texture_2d.hpp>
#include <ass3/renderer.hpp>
#include <ass3/particle.hpp>

#include <math.h>
#include <vector>
#include <iostream>

namespace scene {

    const float GRAVITY               = -19.6f;
    const float JUMP_POWER            = 6.0f;
    const float JUMP_POWER_UNDERWATER = 4.0f;
    const float CAMERA_SPEED          = 5.0f;
    const float PLAYER_RADIUS         = 0.25f; // 0.25
    const float SCREEN_DISTANCE       = 0.25f;
    const int   REFLECTION_SIZE       = 256;
    const int   VOID_LEVEL            = -5;
    const float ZFIGHT_OFFSET         = 0.02f;

    const int   TOTAL_SMOKE           = 12;
    const int   TOTAL_FISH            = 14;
    const int   TOTAL_FIREFLY         = 2;
    const int   TOTAL_DUST            = 4;

    struct node_t {
        static_mesh::mesh_t mesh;

        // Materials
        GLuint textureID = 0;
        GLuint specularID = 0;
        GLuint bloomTexID = 0;
        GLuint reflectionTexID = 0;
        glm::vec4 color = glm::vec4(1.0f);
        glm::vec3 ambient = glm::vec3(1.0f);
		glm::vec3 diffuse = glm::vec3(1.0f);
		glm::vec4 specular = glm::vec4(1.0f);
		float phong_exp = 5.0f;

        glm::vec3 translation = glm::vec3(0.0);
        glm::vec3 rotation = glm::vec3(0.0);
        glm::vec3 scale = glm::vec3(1.0);
        std::vector<node_t> children;
        std::vector<bool> culledFaces = {true, true, true, true, true, true};
        std::string name;

        int x = 0, y = 0, z = 0;
        int lightID = -1;
        bool air = true, transparent = false, illuminating = false, ignoreCulling = false;
    };

    struct playerModel {
        const GLfloat PIXEL_SIZE = 0.0625f;
        const GLfloat OFFSET = 8.0f;

        GLfloat walkAnimationCycle = 0.0f, idleAnimationCycle = 0.0f;
        std::vector<glm::vec3> controlPointWalk = {
            {000.0f,   000.0f, 000.0f},
            {200.0f,   200.0f, 000.0f},
            {200.0f,  -200.0f, 000.0f},
            {400.0f,   000.0f, 000.0f},
        };
        std::vector<glm::vec3> controlPointIdle = {
            {00.0f,   00.0f, 00.0f},
            {10.0f,   10.0f, 00.0f},
            {10.0f,   10.0f, 00.0f},
            {20.0f,   00.0f, 00.0f},
        };

        scene::node_t positionInWorld;

        scene::node_t *headNodePtrs;
        scene::node_t *torsoNodePtrs;
        scene::node_t *rightLegNodePtrs;
        scene::node_t *leftLegNodePtrs;
        scene::node_t *rightArmNodePtrs;
        scene::node_t *swingArmNodePtrs;
        scene::node_t *leftArmNodePtrs;
        scene::node_t *blockInHandPtrs;

        void initialise(GLuint playerTexID, GLuint blockTex) {

            scene::node_t headNode;
            scene::node_t torsoNode;
            scene::node_t rightLegNode;
            scene::node_t leftLegNode;
            scene::node_t rightArmNode;
            scene::node_t leftArmNode;

            scene::node_t playerTorso;
            playerTorso.mesh = shapes::createPlayerTorso();
            playerTorso.textureID = playerTexID;
            playerTorso.air = false;
            torsoNode.children.push_back(playerTorso);

            scene::node_t playerHead;
            playerHead.mesh = shapes::createPlayerHead();
            playerHead.textureID = playerTexID;
            playerHead.translation.y += PIXEL_SIZE * 4;
            playerHead.air = false;
            headNode.children.push_back(playerHead);
            headNode.translation.y += PIXEL_SIZE * 6;

            scene::node_t playerLeftArm;
            playerLeftArm.mesh = shapes::createPlayerArmLeft();
            playerLeftArm.textureID = playerTexID;
            playerLeftArm.translation.x += PIXEL_SIZE * 2; // Moving pivot point
            playerLeftArm.translation.y -= PIXEL_SIZE * 6;
            playerLeftArm.air = false;
            leftArmNode.children.push_back(playerLeftArm);
            leftArmNode.translation.x += PIXEL_SIZE * 4; // Push to the right position
            leftArmNode.translation.y += PIXEL_SIZE * 6;

            scene::node_t playerLeftLeg;
            playerLeftLeg.mesh = shapes::createPlayerLegLeft();
            playerLeftLeg.textureID = playerTexID;
            playerLeftLeg.translation.y -= PIXEL_SIZE * 6; // Moving pivot point
            playerLeftLeg.air = false;
            leftLegNode.children.push_back(playerLeftLeg);
            leftLegNode.translation.x += PIXEL_SIZE * 2; // Push to the right position
            leftLegNode.translation.y -= PIXEL_SIZE * 6; // Push to the right position

            scene::node_t playerRightArm;
            playerRightArm.mesh = shapes::createPlayerArmRight();
            playerRightArm.textureID = playerTexID;
            playerRightArm.translation.x -= PIXEL_SIZE * 2; // Moving pivot point
            playerRightArm.translation.y -= PIXEL_SIZE * 6;
            playerRightArm.air = false;
            // Creating block holding in hand
            scene::node_t blockInHand;
            blockInHand.mesh = shapes::createCube(false, true);
            blockInHand.scale *= 0.3f;
            blockInHand.textureID = blockTex;
            blockInHand.air = false;
            blockInHand.translation.y -= PIXEL_SIZE * 7;
            blockInHand.translation.z += PIXEL_SIZE * 3;
            blockInHand.rotation.z += 45.0f;
            blockInHand.rotation.x += 45.0f;
            blockInHand.rotation.y += 45.0f;
            scene::node_t fluff;
            blockInHand.children.push_back(fluff); 
            playerRightArm.children.push_back(blockInHand);  

            rightArmNode.children.push_back(playerRightArm);
            rightArmNode.translation.x -= PIXEL_SIZE * 4; // Push to the right position
            rightArmNode.translation.y += PIXEL_SIZE * 6;

            scene::node_t playerRightLeg;
            playerRightLeg.mesh = shapes::createPlayerLegRight();
            playerRightLeg.textureID = playerTexID;
            playerRightLeg.translation.y -= PIXEL_SIZE * 6; // Moving pivot point
            playerRightLeg.air = false;
            rightLegNode.children.push_back(playerRightLeg);
            rightLegNode.translation.x -= PIXEL_SIZE * 2; // Push to the right position
            rightLegNode.translation.y -= PIXEL_SIZE * 6; // Push to the right position

            positionInWorld.children.push_back(torsoNode);
            positionInWorld.children.push_back(headNode);
            positionInWorld.children.push_back(leftArmNode);
            positionInWorld.children.push_back(rightArmNode);
            positionInWorld.children.push_back(leftLegNode);
            positionInWorld.children.push_back(rightLegNode);

            torsoNodePtrs = &positionInWorld.children.at(0);
            headNodePtrs = &positionInWorld.children.at(1);
            leftArmNodePtrs = &positionInWorld.children.at(2);
            rightArmNodePtrs = &positionInWorld.children.at(3);
            leftLegNodePtrs = &positionInWorld.children.at(4);
            rightLegNodePtrs = &positionInWorld.children.at(5);
            blockInHandPtrs = &positionInWorld.children.at(3).children.at(0).children.at(0);
        }

        void tiltHead(float headPitch) {
            headNodePtrs->rotation.x = -headPitch;
        }

        void rotateBody(float headYaw) {
            positionInWorld.rotation.y = -headYaw + 180.0f;
        }

        void moveBody(glm::vec3 newPos, bool shiftMode, float ingameTime) {
            positionInWorld.translation = newPos;
            positionInWorld.translation.y -= 0.4f;
            positionInWorld.translation.x -= 1.1f * ((ingameTime - 0.25f) / 0.25f);
            if (shiftMode) positionInWorld.translation.y += 0.25f;
        }

        void idleAnimation(float dt, float swingCycle) {
            if (walkAnimationCycle != 0) {
                walkAnimationCycle = 0;
                leftLegNodePtrs->rotation.x = utility::cubicBezier(controlPointWalk, 0).y;
                rightLegNodePtrs->rotation.x = -utility::cubicBezier(controlPointWalk, 0).y;

                leftArmNodePtrs->rotation.x = utility::cubicBezier(controlPointWalk, 0).y;
                rightArmNodePtrs->rotation.x = -utility::cubicBezier(controlPointWalk, 0).y;
            }
            idleAnimationCycle += dt * 0.4f;
            idleAnimationCycle = fmod(idleAnimationCycle, 1.0f);
            // Right arm
            if (swingCycle >= 0) {
                rightArmNodePtrs->rotation.x = -utility::cubicBezier(controlPointWalk, swingCycle).y / 2.0f;
            } else {
                rightArmNodePtrs->rotation.x = 0;
                rightArmNodePtrs->rotation.z = -utility::cubicBezier(controlPointIdle, idleAnimationCycle).y;
            }
            // Left arm
            leftArmNodePtrs->rotation.z = utility::cubicBezier(controlPointIdle, idleAnimationCycle).y;
        }

        void changeBlockInHandTex(GLuint texID, GLuint specID) {
            blockInHandPtrs->textureID = texID;
            blockInHandPtrs->specularID = specID;
            return;
        }

        void walkAnimation(float dt, bool runningMode) {
            if (idleAnimationCycle != 0) {
                idleAnimationCycle = 0;
                leftArmNodePtrs->rotation.z = utility::cubicBezier(controlPointIdle, 0).y;
                rightArmNodePtrs->rotation.z = -utility::cubicBezier(controlPointIdle, 0).y;
            }

            auto amplitude = 0.5f, speed = 1.0f;
            if (runningMode) amplitude = 1.2f, speed = 1.5f;

            walkAnimationCycle += dt * speed;
            walkAnimationCycle = fmod(walkAnimationCycle, 1.0f);

            auto rotation = utility::cubicBezier(controlPointWalk, walkAnimationCycle).y * amplitude;
            leftLegNodePtrs->rotation.x = rotation;
            rightLegNodePtrs->rotation.x = -rotation;

            leftArmNodePtrs->rotation.x = rotation;
            rightArmNodePtrs->rotation.x = -rotation;
        }
    };

    struct blockData {
        std::string blockName;
        GLuint texture = 0;
        GLuint specularMap = 0;
        GLuint bloomTexID = 0;
        glm::vec3 rgb = {0, 0, 0};
        bool transparent = false;
        bool illuminating = false;
        bool rotatable = false;
        float intensity = 1.0f;
    };

    struct miniBlockData {
        std::string blockName;
        glm::vec3 position;
        bool entireLayer, startAtMiddle;
        miniBlockData(std::string name, glm::vec3 pos, bool startFromCentre = false, bool fillEntireLayer = false) {
            blockName = name;
            position = pos;
            entireLayer = fillEntireLayer;
            startAtMiddle = startFromCentre;
        }
    };

    /**
     * @brief Render the given node and its children recursively. Renders selected sides based on
     * the node's culled faces
     * 
     * @param node 
     * @param model 
     * @param renderInfo 
     */
    void drawBlock(const node_t *node, glm::mat4 model, renderer::renderer_t renderInfo, bool onlyIlluminating);

    /**
     * @brief Render the given node and its children recursively. Renders all sides regardless
     * of culled faces
     * 
     * @param node 
     * @param model 
     * @param renderInfo 
     */
    void drawElement(const node_t *node, glm::mat4 model, renderer::renderer_t renderInfo);

    /**
     * @brief Takes in the parameters and returns blockData with all the information given stored inside
     * 
     * @param stringName 
     * @param transparent 
     * @param illuminating 
     * @param rotatable 
     * @param color 
     * @param intensity 
     * @return blockData 
     */
    blockData combineBlockData(std::string stringName, bool transparent, bool illuminating, bool rotatable = false, glm::vec3 color = {0, 0, 0}, float intensity = 1.0f);

    /**
     * @brief Gives a view matrix depending on the given float values
     * 
     * @param pitch 
     * @param yaw 
     * @param pos 
     * @return glm::mat4 
     */
    glm::mat4 rotateViewMatrix(GLfloat pitch, GLfloat yaw, glm::vec3 pos);

    /**
     * @brief Create a Block object with the given texID and specIDs
     * 
     * @param x 
     * @param y 
     * @param z 
     * @param texID 
     * @param specID 
     * @param transparent 
     * @param invertNormals 
     * @param affectedByLight 
     * @return node_t 
     */
    node_t createBlock(int x, int y, int z, GLuint texID, GLuint specID, bool transparent, bool invertNormals, bool affectedByLight);
    
    /**
     * @brief Create a Block object with the given blockData
     * 
     * @param x 
     * @param y 
     * @param z 
     * @param data 
     * @param invertNormals 
     * @param affectedByLight 
     * @return node_t 
     */
    node_t createBlock(int x, int y, int z, blockData data, bool invertNormals, bool affectedByLight);

    /**
     * @brief Create a Flat Square object that is one dimensional
     * 
     * @param texID 
     * @param invert 
     * @return node_t 
     */
    node_t createFlatSquare(GLuint texID, bool invert);

    /**
     * @brief Create a Flat Surface object that has both sides rendered
     * 
     * @param texID 
     * @param width 
     * @return node_t 
     */
    node_t createFlatSurface(GLuint texID, int width);

    /**
     * @brief Create a Sky Box object
     * 
     * @return node_t 
     */
    node_t createSkyBox();

    /**
     * @brief Create a Bed Player object. This node_t will have the right children to construct a Minecraft Player Model
     * 
     * @param bedTexID 
     * @param playerTexID 
     * @return node_t 
     */
    node_t createBedPlayer(GLuint bedTexID, GLuint playerTexID);

    /**
     * @brief Destroys the node. destroyTexture = true to delete the texture as well.
     * 
     * @param node 
     * @param destroyTexture 
     */
    void destroy(const node_t *node, bool destroyTexture);

    // WORLD = Everything that shows up on the screen is controlled from here

    struct world {

        size_t worldWidth = 110;
        const size_t WORLD_HEIGHT = 50;
        size_t seaSize = 0, cloudSize = 0;
        GLfloat cloudOffset = 0, cloudOriginalZ = 0;
        
        playerModel player;
        GLfloat worldTime = 0;

        float eyeLevel = 1.0f;
        bool shiftMode = false;
        float walkingMultiplier = 0.5f;
        bool cutsceneEnabled = false, useReflectionCam = false, drawCelestials = true;;
        player::playerPOV playerCamera, cutsceneCamera, reflectionCamera;
        glm::vec3 oldPos, oldHandPos, oldHandRotation, lastRenderedPos;
        int increments = 200;
        int playerReachRange = 4 * increments;
        int groundLevel = -99999, aboveLevel = 99999;
        float cutsceneTick = 0;
        float swingCycle = -1.0f, walkCycle = 0.0f;
        std::vector<GLuint> moonPhases;
        size_t moonPhase = 0;

        std::vector<std::vector<std::vector<node_t>>> terrain;
        std::vector<particle::particle_t *> listOfParticles;
        std::vector<node_t *> listOfBlocksToRender;
        std::vector<node_t *> listOfTransBlocksToRender;
        std::vector<node_t *> listOfShinyBlocksToRender;

        node_t screen;
        node_t screenHand;
        node_t centreOfWorld;
        node_t highlightedBlock;
        node_t bed;
        node_t skyBox;
        node_t seaSurface, clouds;

        GLuint mirrorGreenscreen = texture_2d::init("./res/textures/blocks/mirror_gs.png");
        // Particles
        GLuint bubble = texture_2d::init("./res/textures/particles/bubble.png");
        GLuint tear = texture_2d::init("./res/textures/particles/obsidian_tear.png");
        GLuint glint = texture_2d::init("./res/textures/particles/glint.png");
        GLuint smokeParticles[TOTAL_SMOKE] = {
            texture_2d::init("./res/textures/particles/smoke/big_smoke_0.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_1.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_2.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_3.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_4.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_5.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_6.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_7.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_8.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_9.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_10.png"),
            texture_2d::init("./res/textures/particles/smoke/big_smoke_11.png")
        };
        GLuint fish[TOTAL_FISH] = {
            texture_2d::init("./res/textures/particles/fish/salmonA.png"),
            texture_2d::init("./res/textures/particles/fish/salmonB.png"),
            texture_2d::init("./res/textures/particles/fish/codA.png"),
            texture_2d::init("./res/textures/particles/fish/codB.png"),
            texture_2d::init("./res/textures/particles/fish/redTropicalFishA.png"),
            texture_2d::init("./res/textures/particles/fish/redTropicalFishB.png"),
            texture_2d::init("./res/textures/particles/fish/orangeTropicalFishA.png"),
            texture_2d::init("./res/textures/particles/fish/orangeTropicalFishB.png"),
            texture_2d::init("./res/textures/particles/fish/blueTropicalFishA.png"),
            texture_2d::init("./res/textures/particles/fish/blueTropicalFishB.png"),
            texture_2d::init("./res/textures/particles/fish/whiteTropicalFishA.png"),
            texture_2d::init("./res/textures/particles/fish/whiteTropicalFishB.png"),
            texture_2d::init("./res/textures/particles/fish/grayTropicalFishA.png"),
            texture_2d::init("./res/textures/particles/fish/grayTropicalFishB.png")
        };
        GLuint firefly[TOTAL_FIREFLY] = {
            texture_2d::init("./res/textures/particles/fireflyA.png"),
            texture_2d::init("./res/textures/particles/fireflyB.png")
        };
        GLuint dust[TOTAL_DUST] = {
            texture_2d::init("./res/textures/particles/dust/dust_0.png"),
            texture_2d::init("./res/textures/particles/dust/dust_1.png"),
            texture_2d::init("./res/textures/particles/dust/dust_2.png"),
            texture_2d::init("./res/textures/particles/dust/dust_3.png")
        };
        
        std::vector<blockData> hotbar;
        std::vector<blockData> hotbarSecondary;

        std::vector<int> hotbarTextureIndex;

        int renderDistance = 0;
        int hotbarIndex = 0;
        size_t handIndex = 0, hotbarHUDIndex = 0, flyingIconIndex = 0, moonIndex = 0, instructionIndex = 0;
        bool flyingMode = false, startSwingHandAnim = false, runningMode = false, hideScreen = false;

        /**
         * @brief Creates a world with the given render distance and the world width
         * 
         * @param listOfBlocks 
         * @param inputRenderDistance 
         * @param inputWidth 
         */
        world(std::vector<miniBlockData> listOfBlocks, int inputRenderDistance, int inputWidth, renderer::renderer_t *renderInfo) {

            renderDistance = inputRenderDistance;
            worldWidth = (size_t)inputWidth;
            terrain = {worldWidth , std::vector< std::vector<node_t> > (WORLD_HEIGHT, std::vector<node_t> (worldWidth) ) };
            
            std::cout << "Generating world of size " << worldWidth << "x" << worldWidth << " with render distance " << renderDistance << ". Please standby...\n";
            
            // SETTING UP BED SCENE GRAPH
            GLuint playerTex = texture_2d::init("./res/textures/player.png");
            bed = createBedPlayer(texture_2d::init("./res/textures/blocks/bed.png"), playerTex);

            GLuint flyingIcon = texture_2d::init("./res/textures/flying_mode.png");

            // SETTING UP SEA FLOOR
            seaSize = 100 * worldWidth;
            cloudSize = 10 * worldWidth;
            seaSurface = createFlatSurface(0, seaSize);
            seaSurface.translation.x = worldWidth / 2.0f;
            seaSurface.translation.z = worldWidth / 2.0f;
            seaSurface.translation.y = VOID_LEVEL + ZFIGHT_OFFSET;

            // SETTING UP CLOUDS
            clouds = createFlatSurface(texture_2d::init("./res/textures/clouds.png"), 4);
            clouds.bloomTexID = texture_2d::init("./res/textures/clouds_bloom.png");
            clouds.translation.x = (float)worldWidth / 2.0f;
            cloudOriginalZ = clouds.translation.z;
            clouds.translation.z = (float)worldWidth / 2.0f;
            clouds.translation.y = ((float)WORLD_HEIGHT * 0.8f) + ZFIGHT_OFFSET * 2;
            clouds.scale *= glm::vec3(cloudSize, 1, cloudSize);
            clouds.ignoreCulling = true;

            // SETTING UP CENTRE OF WORLD SCENE GRAPH
            // Setting up moon phases
            moonPhases.push_back(texture_2d::init("./res/textures/blocks/moon/moon_0.png"));
            moonPhases.push_back(texture_2d::init("./res/textures/blocks/moon/moon_1.png"));
            moonPhases.push_back(texture_2d::init("./res/textures/blocks/moon/moon_2.png"));
            moonPhases.push_back(texture_2d::init("./res/textures/blocks/moon/moon_3.png"));
            moonPhases.push_back(texture_2d::init("./res/textures/blocks/moon/moon_4.png"));
            moonPhases.push_back(texture_2d::init("./res/textures/blocks/moon/moon_5.png"));
            moonPhases.push_back(texture_2d::init("./res/textures/blocks/moon/moon_6.png"));
            moonPhases.push_back(texture_2d::init("./res/textures/blocks/moon/moon_7.png"));

            moonPhase = rand() % (int)moonPhases.size();

            highlightedBlock = scene::createBlock(0, 0, 0, texture_2d::init("./res/textures/blocks/highlight.png"), 0, false, false, true);
            highlightedBlock.scale = glm::vec3(1.0005f, 1.0005f, 1.0005f);
            // Setting up Sun
            node_t sun = scene::createBlock(0, 0, 0, texture_2d::init("./res/textures/blocks/sun.png"), 0, false, true, false);
            sun.illuminating = true;
            sun.scale = glm::vec3(0.001, 4.0, 4.0);
            sun.translation.x += (float)getSkyRadius();
            GLuint auraTextureID = texture_2d::init("./res/textures/blocks/sun_aura.png");

            // Setting up Moon
            node_t moonOrbit;
            node_t moon = scene::createBlock(0, 0, 0, moonPhases[(size_t)moonPhase], 0, false, false, false);
            moon.scale = glm::vec3(0.001, 2.5, 2.5);
            moon.illuminating = true;
            moonOrbit.translation.x -= (float)getSkyRadius();

            moonOrbit.children.push_back(moon);

            // Creating skybox
            skyBox = scene::createSkyBox();

            centreOfWorld.children.push_back(sun);
            centreOfWorld.children.push_back(moonOrbit);
            moonIndex = centreOfWorld.children.size() - (size_t)1;

            // SETTING UP SCREEN SCENE GRAPH

            // Crosshair
            node_t crosshair = scene::createFlatSquare(texture_2d::init("./res/textures/crosshair.png"), false);
            crosshair.translation.z = -1 * SCREEN_DISTANCE;
            crosshair.scale = glm::vec3(0.02, 0.02, 0.02);
            screen.children.push_back(crosshair);

            // ADD BLOCKS HERE //
            // First hotbar
            hotbar.push_back(combineBlockData("dirt", false, false));
            hotbar.push_back(combineBlockData("coarse_dirt", false, false));
            hotbar.push_back(combineBlockData("grass_block", false, false));
            hotbar.push_back(combineBlockData("barrel", false, false, true));
            hotbar.push_back(combineBlockData("crafting_table", false, false));
            hotbar.push_back(combineBlockData("oak_planks", false, false));
            hotbar.push_back(combineBlockData("oak_log", false, false, true));
            hotbar.push_back(combineBlockData("oak_leaves", false, false));
            hotbar.push_back(combineBlockData("cobblestone", false, false));
            hotbar.push_back(combineBlockData("mossy_cobblestone", false, false));
            hotbar.push_back(combineBlockData("stone", false, false));
            hotbar.push_back(combineBlockData("stone_bricks", false, false));
            hotbar.push_back(combineBlockData("mossy_stone_bricks", false, false));
            hotbar.push_back(combineBlockData("cracked_stone_bricks", false, false));
            hotbar.push_back(combineBlockData("glass", true, false));
            hotbar.push_back(combineBlockData("sea_lantern", false, true, false, glm::vec3(212.0f, 235.0f, 255.0f) * (1.0f / 255.0f), 2.0f));
            hotbar.push_back(combineBlockData("magma", false, true, false, glm::vec3(244.0f, 133.0f, 34.0f) * (1.0f / 255.0f), 1.5f));
            hotbar.push_back(combineBlockData("glowstone", false, true, false, glm::vec3(251.0f, 218.0f, 116.0f) * (1.0f / 255.0f), 1.6f));
            hotbar.push_back(combineBlockData("crying_obsidian", false, true, false, glm::vec3(131.0f, 8.0f, 228.0f) * (1.0f / 255.0f), 2.0f));
            hotbar.push_back(combineBlockData("obsidian", false, false));
            hotbar.push_back(combineBlockData("tnt", false, false));
            hotbar.push_back(combineBlockData("raw_iron", false, false));
            hotbar.push_back(combineBlockData("iron_block", false, false));
            hotbar.push_back(combineBlockData("raw_copper", false, false));
            hotbar.push_back(combineBlockData("copper_block", false, false));
            hotbar.push_back(combineBlockData("raw_gold", false, false));
            hotbar.push_back(combineBlockData("gold_block", false, false));
            hotbar.push_back(combineBlockData("coal_ore", false, false));
            hotbar.push_back(combineBlockData("iron_ore", false, false));
            hotbar.push_back(combineBlockData("copper_ore", false, false));
            hotbar.push_back(combineBlockData("gold_ore", false, false));
            hotbar.push_back(combineBlockData("lapis_ore", false, false));
            hotbar.push_back(combineBlockData("redstone_ore", false, true, false, glm::vec3(253.0f, 94.0f, 94.0f) * (1.0f / 255.0f), 0.5f));
            hotbar.push_back(combineBlockData("emerald_ore", false, false));
            hotbar.push_back(combineBlockData("diamond_ore", false, false));
            hotbar.push_back(combineBlockData("slime_block", false, false, true));
            hotbar.push_back(combineBlockData("marccoin_block", false, false, true));
            hotbar.push_back(combineBlockData("coral_brain", false, false));
            hotbar.push_back(combineBlockData("coral_bubble", false, false));
            hotbar.push_back(combineBlockData("coral_fire", false, false));
            hotbar.push_back(combineBlockData("coral_horn", false, false));
            hotbar.push_back(combineBlockData("coral_tube", false, false));
            hotbar.push_back(combineBlockData("mirror", false, false, false));
            hotbar.push_back(combineBlockData("bedrock", false, false));
            
            // Second hotbar
            hotbarSecondary.push_back(combineBlockData("white", false, false));
            hotbarSecondary.push_back(combineBlockData("orange", false, false));
            hotbarSecondary.push_back(combineBlockData("magenta", false, false));
            hotbarSecondary.push_back(combineBlockData("light_blue", false, false));
            hotbarSecondary.push_back(combineBlockData("yellow", false, false));
            hotbarSecondary.push_back(combineBlockData("lime", false, false));
            hotbarSecondary.push_back(combineBlockData("pink", false, false));
            hotbarSecondary.push_back(combineBlockData("gray", false, false));
            hotbarSecondary.push_back(combineBlockData("light_gray", false, false));
            hotbarSecondary.push_back(combineBlockData("cyan", false, false));
            hotbarSecondary.push_back(combineBlockData("purple", false, false));
            hotbarSecondary.push_back(combineBlockData("blue", false, false));
            hotbarSecondary.push_back(combineBlockData("brown", false, false));
            hotbarSecondary.push_back(combineBlockData("green", false, false));
            hotbarSecondary.push_back(combineBlockData("red", false, false));
            hotbarSecondary.push_back(combineBlockData("black", false, false));

            // Hand
            node_t blockHand = scene::createBlock(0, 0, 0, hotbar[0].texture, hotbar[0].specularMap, false, false, true);
            blockHand.translation.z = -1.0f * SCREEN_DISTANCE;
            blockHand.translation.y -= 0.15f;
            blockHand.translation.x += 0.25f;
            blockHand.scale = glm::vec3(0.15, 0.15, 0.15);

            player.initialise(playerTex, hotbar.at(2).texture);

            screenHand.children.push_back(blockHand);
            handIndex = screenHand.children.size() - 1;

            node_t flyingIconNode = scene::createFlatSquare(flyingIcon, false);
            flyingIconNode.translation.z = -2.8f * SCREEN_DISTANCE;
            flyingIconNode.translation.y += 0.096f;
            flyingIconNode.translation.x -= 0.17f;
            flyingIconNode.scale = glm::vec3(0.03, 0.03, 1);
            flyingIconNode.air = true;
            screen.children.push_back(flyingIconNode);
            flyingIconIndex = screen.children.size() - 1;

            node_t hotbarTexture = scene::createFlatSquare(texture_2d::init("./res/textures/hotbar.png"), false);
            hotbarTexture.translation.z = -2.7f * SCREEN_DISTANCE;
            hotbarTexture.translation.y -= 0.015f;
            hotbarTexture.scale = glm::vec3(0.25f, 0.25f, 1.0f);
            screen.children.push_back(hotbarTexture);
            hotbarHUDIndex = screen.children.size() - 1;

            // Filling up the hotbar and positioning it correctly onto the screen
            float xPos = -0.285f;
            for (int i = 0; i < 9; i++) {
                node_t hudHotbarBlock = scene::createBlock(0, 0, 0, hotbar[1].texture, 0, false, false, false);
                hudHotbarBlock.translation.z += 0.55f;
                hudHotbarBlock.translation.y += 0.3f;
                hudHotbarBlock.translation.x = xPos;
                xPos += 0.0715f;
                hudHotbarBlock.rotation = glm::vec3(45, 35, 30);
                hudHotbarBlock.scale = glm::vec3(0.035, 0.035, 0.00001);
                screen.children[(size_t)hotbarHUDIndex].children.push_back(hudHotbarBlock);
                hotbarTextureIndex.push_back((int)screen.children[(size_t)hotbarHUDIndex].children.size() - 1);
            }

            node_t instructions = scene::createFlatSquare(texture_2d::init("./res/textures/instructions.png"), false);
            instructions.translation.z = -2.699f * SCREEN_DISTANCE;
            instructions.translation.x -= 0.11f;
            instructions.translation.y -= 0.03f;
            instructions.scale = glm::vec3(0.13, 0.13, 1);
            screen.children.push_back(instructions);
            instructionIndex = screen.children.size() - 1;
            
            // ALL SCENE GRAPHS
            // Screen node -> Hand object
            //             -> Flying icon
            //             -> Instructions
            //             -> Hotbar object -> All 9 items

            // Centre of the World Node
            //                          -> Sun
            //                          -> Moon

            // Bed -> Centre Of Player Node -> Head
            //                              -> Torso
            //                              -> Left Arm
            //                              -> Right Arm
            //                              -> Left leg
            //                              -> Right leg

            // Update the textures of the hotbar
            scrollHotbar(1);
            scrollHotbar(-1);

            // Generating the new world
            for (size_t i = 0; i < listOfBlocks.size(); i++) {
                
                auto generatingBlock = findDataBlockName(listOfBlocks[i].blockName);
                if (listOfBlocks[i].entireLayer) {
                    // Fills up the entire y level if specified
                    for (int x = 0; x < (int)terrain.size(); x++) {
                        for (int z = 0; z < (int)terrain.at(0).at(0).size(); z++) {
                            placeBlock(scene::createBlock(x, (int)listOfBlocks[i].position.y, z, generatingBlock, false, !generatingBlock.illuminating));
                            if (generatingBlock.illuminating) {
                                terrain.at(x).at(listOfBlocks[i].position.y).at(z).lightID = renderInfo->addLightSource(listOfBlocks[i].position, generatingBlock.rgb, generatingBlock.intensity);
                            }
                        }
                    }
                } else {
                    if (listOfBlocks[i].startAtMiddle) {
                        // Repositioning all the co-oridinates into the centre of the world if specified
                        listOfBlocks[i].position += glm::vec3((float)terrain.size() / 2.0f, 0.0f, (float)terrain.at(0).at(0).size() / 2.0f);
                    }
                    placeBlock(scene::createBlock((int)listOfBlocks[i].position.x, (int)listOfBlocks[i].position.y, (int)listOfBlocks[i].position.z, generatingBlock, false, !generatingBlock.illuminating));
                    if (generatingBlock.illuminating) {
                        terrain.at(listOfBlocks[i].position.x).at(listOfBlocks[i].position.y).at(listOfBlocks[i].position.z).lightID = renderInfo->addLightSource(listOfBlocks[i].position, generatingBlock.rgb, generatingBlock.intensity);
                    }
                }
                terrain.at((size_t)listOfBlocks[i].position.x).at((size_t)listOfBlocks[i].position.y).at((size_t)listOfBlocks[i].position.z).transparent = generatingBlock.transparent;
            }
            // Keeping track of where the hand and rotation is
            oldHandPos = screenHand.children[handIndex].translation;
            oldHandRotation = screenHand.children[handIndex].rotation;
            std::cout << "World Created\n\n";
        }

        /**
         * @brief Get the current camera being used to render the scene
         * 
         * @return player::playerPOV* 
         */
        player::playerPOV *getCurrCamera() {

            if (useReflectionCam) {
                return &reflectionCamera;
            } else if (cutsceneEnabled) {
                return &cutsceneCamera;
            }
            return &playerCamera;
        }

        /**
         * @brief Repositions where the reflection camera is depending on where the current camera is
         * 
         */
        void updateReflectionCamera() {
            reflectionCamera.pos = getCurrCamera()->pos;
            reflectionCamera.pos.y -= 2 * ((getCurrCamera()->pos.y) - seaSurface.translation.y);
            
            reflectionCamera.yaw = getCurrCamera()->yaw;
            reflectionCamera.pitch = -getCurrCamera()->pitch;
        }

        /**
         * @brief Changes the sea level in the direction given. Clamps the value between world height and the void level
         * 
         */
        void changeSeaLevel(int direction) {
            if (cutsceneEnabled) return;
            seaSurface.translation.y -= ZFIGHT_OFFSET;
            if (direction > 0) {
                seaSurface.translation.y += 0.5f;
            } else {
                seaSurface.translation.y -= 0.5f;
            }
            seaSurface.translation.y = std::min(std::max(seaSurface.translation.y, (float)VOID_LEVEL - 0.5f), (float)WORLD_HEIGHT);
            if (seaSurface.translation.y < VOID_LEVEL) {
                seaSurface.air = true;
            } else {
                seaSurface.air = false;
            }
            // Preventing Z-fighting
            seaSurface.translation.y += ZFIGHT_OFFSET;
        }

        /**
         * @brief Based on the blockName, find the correct blockData that represents the desired block
         * Will return the first blockData if the name is not valid
         * 
         * @param blockName 
         * @return blockData 
         */
        blockData findDataBlockName(std::string blockName) {
            for (size_t i = 0; i < hotbar.size(); i++) {
                if (hotbar[i].blockName == blockName) {
                    return hotbar[i];
                }
            }
            for (size_t i = 0; i < hotbarSecondary.size(); i++) {
                if (hotbarSecondary[i].blockName == blockName) {
                    return hotbarSecondary[i];
                }
            }
            std::cout << "ERROR: BLOCK NOT FOUND!\n";
            return hotbar.front();
        }

        /**
         * @brief Turns the display of the instructions on or off
         * 
         * @param status 
         */
        void toggleInstructions(bool status) {
            screen.children[(size_t)instructionIndex].air = status;
            return;
        }

        /**
         * @brief Get the bool which governs if the instructions are to appear or not
         * 
         * @return true if the instructions are showing
         * @return false if the instructions are not showing
         */
        bool getInstructionStatus() {
            return screen.children[(size_t)instructionIndex].air;
        }

        /**
         * @brief Use this to change the moon phase to the next
         * 
         */
        void updateMoonPhase() {
            moonPhase++;
            moonPhase %= moonPhases.size();
            centreOfWorld.children[(size_t)moonIndex].children[0].textureID = moonPhases[moonPhase];
        }

        /**
         * @brief Returns the radius of the sky
         * 
         * @return int 
         */
        int getSkyRadius() {
            return 30;
        }

        /**
         * @brief Use this to enable the sleeping cutscene. Can only occur if the player is on the ground and aren't surrounded by blocks
         * 
         */
        void toggleCutscene() {
            if (playerCamera.pos.y != groundLevel + eyeLevel && !cutsceneEnabled) {
                return;
            } else if (!check3x3Area(playerCamera.pos) && !cutsceneEnabled) {
                std::cout << "You may not sleep, you are surrounded by blocks!\n";
                return;
            } else if (isUnderwater() && !cutsceneEnabled) {
                std::cout << "You may not sleep, you are underwater!\n";
                return;
            }
            cutsceneEnabled = !cutsceneEnabled;
            if (cutsceneEnabled) {
                cutsceneTick = (float) glfwGetTime();
                cutsceneCamera.pos.x = playerCamera.pos.x;
                cutsceneCamera.pos.y = playerCamera.pos.y;
                cutsceneCamera.pos.z = playerCamera.pos.z;
                cutsceneCamera.pitch = playerCamera.pitch;
                cutsceneCamera.yaw = playerCamera.yaw;
                bed.translation = playerCamera.pos;
                bed.translation.y -= eyeLevel;
            }
        }

        /**
         * @brief Checks if there are any blocks around the player
         * 
         * @param pos 
         * @return true if there aren't any blocks in the given 3 by 3
         * @return false if there are blocks in the given 3 by 3
         */
        bool check3x3Area(glm::vec3 pos) {
            glm::vec3 tempPos;
            tempPos.x = round(pos.x);
            tempPos.z = round(pos.z);
            tempPos.y = round(pos.y - eyeLevel);
            for (size_t i = tempPos.x - (size_t)1; i <= tempPos.x + 1; i++) {
                for (size_t j = tempPos.z - (size_t)1; j <= tempPos.z + 1; j++) {
                    if (isCoordOutBoundaries(i, tempPos.y, j)) continue;
                    if (!terrain.at(i).at(tempPos.y).at(j).air) {
                        return false;
                    }
                }
            }
            return true;
        }

        /**
         * @brief Get information on whether the sleeping cutscene is playing or not
         * 
         * @return true if cutscene is playing
         * @return false if the cutscene is not playing
         */
        bool getCutsceneStatus() {
            return cutsceneEnabled;
        }

        /**
         * @brief Advances the cutscene forward in the bezier curve
         * 
         */
        void animateCutscene() {
            // Calculating delta time
            auto now = (float) glfwGetTime() - cutsceneTick;

            auto desiredYaw = playerCamera.yaw - 180.0f;
            if (desiredYaw < 0.0f) {
                desiredYaw = 360.0f + desiredYaw;
            } else if (desiredYaw >= 360.0f) {
                desiredYaw -= 360.0f;
            }

            // Controls the path of camera
            std::vector<glm::vec3> controlPointA = {
                {0.0f,   0.0f, 0.0f},
                {0.0f,   4.0f, 0.0f},
                {4.0f * (float)glm::sin(glm::radians(playerCamera.yaw)), 0.0f, 4.0f * -glm::cos(glm::radians(playerCamera.yaw))},
                {4.0f * (float)glm::sin(glm::radians(playerCamera.yaw)), 4.0f, 4.0f * (float)-glm::cos(glm::radians(playerCamera.yaw))},
            };

            // Ease in, Ease out speed
            std::vector<glm::vec3> controlPointB = {
                {0.0f, 0.0f, 0.0f},
                {0.5f, 0.0f, 0.0f},
                {0.5f, 1.0f, 0.0f},
                {1.0f, 1.0f, 0.0f},
            };

            // Moves the controlPointA path to be relative to the current position
            for (size_t i = 0; i < controlPointA.size(); i++) {
                controlPointA[i] += playerCamera.pos;
            }

            float t = 0.25f * (float)now;
            if (t > 1.0f) {
                t = 1.0f;
            } else {
                updateBlocksToRender();
            }
            getCurrCamera()->yaw = playerCamera.yaw - (playerCamera.yaw - desiredYaw) * utility::cubicBezier(controlPointB, t).y;
            getCurrCamera()->pitch = playerCamera.pitch - (playerCamera.pitch + 30.0f) * utility::cubicBezier(controlPointB, t).y;
            getCurrCamera()->pos = playerCamera.pos + ((utility::cubicBezier(controlPointA, t) - playerCamera.pos) * utility::cubicBezier(controlPointB, t).y);

            return;
        }


        /**
         * @brief Switches between the main hotbar and the secondary hotbar with the wool blocks
         * 
         */
        void switchHotbars() {
            std::vector<blockData> tempHotbar = hotbar;
            hotbar.clear();
            for (auto i : hotbarSecondary) {
                hotbar.push_back(i);
            }
            hotbarSecondary.clear();
            for (auto i : tempHotbar) {
                hotbarSecondary.push_back(i);
            }
            hotbarIndex = 0;
            scrollHotbar(-1);
            scrollHotbar(1);
        }

        /**
         * @brief Updates the sun and moon position based on the given degree.
         * Also changes the diffuse of the sky sphere.
         * 
         * @param degree 
         * @param skyColor 
         * @param dt 
         */
        void updateSunPosition(float degree, float dt) {
            worldTime = fmod(abs(degree / 360.0f), 1.0f);
            centreOfWorld.translation = playerCamera.pos;
            centreOfWorld.rotation = glm::vec3(0, 0, degree);
            // Move the cloud along
            cloudOffset = fmod(cloudOffset + dt / ((cutsceneEnabled) ? 0.1f : 3.0f), cloudSize);
            clouds.translation.z = cloudOriginalZ + cloudOffset;
        }

        /**
         * @brief Advances the bobbing hand animation based on the given delta time
         * 
         * @param dt 
         */
        void bobHand(float dt) {

            // Only bob hand if the player is on the ground
            if (swingCycle == -1.0f) {
                // Controls how fast to bobhand
                dt *= walkingMultiplier;
                walkCycle += dt * 2.0f;
                walkCycle = fmod(walkCycle, 1.0f);
                std::vector<glm::vec3> controlPoint = {
                    {0.0f,  0.0f,  0.0f},
                    {1.0f, -1.0f,  0.0f},
                    {1.0f,  1.0f,  0.0f},
                    {2.0f,  0.0f,  0.0f},
                };

                // Moves the controlPoint path to be relative to the current position
                for (size_t i = 0; i < controlPoint.size(); i++) {
                    // Scaling it down
                    controlPoint[i] *= 0.05f;
                    controlPoint[i] += oldHandPos;
                }
                
                screenHand.children[handIndex].translation.y = utility::cubicBezier(controlPoint, walkCycle).y;
            }
        }

        /**
         * @brief Advances the swinging hand animation whenever the player presses left or right click
         * based on the given delta time
         * 
         * @param dt 
         */
        void swingHand(float dt) {
            if (swingCycle >= 0.0f) {

                std::vector<glm::vec3> controlPoint = {
                    {0.0f,  0.00f,  0.0f},
                    {0.0f, -0.15f, -0.1f},
                    {0.0f,  0.05f, -0.1f},
                    {0.0f,  0.00f,  0.0f},
                };

                std::vector<glm::vec3> controlPointRotation = {
                    {0.0f,  0.00f,  0.0f},
                    {0.0f, -0.1f, -0.1f},
                    {0.0f,  0.1f, -0.1f},
                    {0.0f,  0.00f,  0.0f},
                };

                // Moves the controlPoint path to be relative to the current position
                for (size_t i = 0; i < controlPoint.size(); i++) {
                    controlPoint[i] += oldHandPos;
                }

                for (size_t i = 0; i < controlPointRotation.size(); i++) {
                    // Scaling the parametric curves
                    controlPointRotation[i] *= 500.0f;
                    controlPointRotation[i] += oldHandRotation;
                }

                swingCycle += dt * 3.0f;
                float t = std::min(1.0f, swingCycle);

                screenHand.children[handIndex].rotation.x = utility::cubicBezier(controlPointRotation, t).z;
                screenHand.children[handIndex].translation = utility::cubicBezier(controlPoint, t);

                if (t == 1.0f) {
                    swingCycle = -1.0f;
                }
            }
        }

        /**
         * @brief Toggles between flying or walking mode
         * 
         */
        void toggleMode() {
            flyingMode = !flyingMode;
            screen.children[flyingIconIndex].air = !flyingMode;
        }

        /**
         * @brief Scrolls hotbar in the direction given. Negative to scroll down, positive to scroll up
         * 
         * @param direction 
         */
        void scrollHotbar(int direction) {
            hotbarIndex += direction;

            if (hotbarIndex >= (int)hotbar.size()) {
                hotbarIndex = 0;
            } else if (hotbarIndex < 0) {
                hotbarIndex = (int)hotbar.size() + hotbarIndex;
            }
            screenHand.children[handIndex].textureID = hotbar[hotbarIndex].texture;
            screenHand.children[handIndex].specularID = hotbar[hotbarIndex].specularMap;
            // Updating the textures in the hotbar
            int tempIndex = hotbarIndex - 4;
            if (tempIndex < 0) {
                tempIndex = hotbar.size() + tempIndex;
            }
            for (int i : hotbarTextureIndex) {
                screen.children[hotbarHUDIndex].children[i].textureID = hotbar[tempIndex].texture;
                tempIndex++;
                tempIndex %= hotbar.size();
            }

            player.changeBlockInHandTex(hotbar[hotbarIndex].texture, hotbar[hotbarIndex].specularMap);
        }
    
        /**
         * @brief Finds the block that the player is looking at within a certain limit.
         * giveBlockBefore to return the location of the nieghbouring block which is closest
         * to where the player was looking
         * 
         * @param giveBlockBefore 
         * @return glm::vec3 
         */
        glm::vec3 findCursorBlock(bool giveBlockBefore) {
            // Don't bother if the cutscene is playing
            if (cutsceneEnabled) return glm::vec3(-1, -1, -1); 

            glm::vec3 lookingDirection = player::getLookingDirection(&playerCamera, increments);
            float rayX = playerCamera.pos.x, rayY = playerCamera.pos.y, rayZ = playerCamera.pos.z;

            int limit = 0;
            while (limit < playerReachRange) {
    
                rayX += lookingDirection.x;
                rayY += lookingDirection.y;
                rayZ += lookingDirection.z;
                limit++;
                
                if (isCoordOutBoundaries((int)round(rayX), (int)round(rayY), (int)round(rayZ))) {
                    continue;
                } else if (!terrain.at((int)round(rayX)).at((int)round(rayY)).at((int)round(rayZ)).air) {
                    break;
                }
            }
            if (limit >= playerReachRange || isCoordOutBoundaries((int)round(rayX), (int)round(rayY), (int)round(rayZ))) {
                return glm::vec3(-1, -1, -1);
            } else if (giveBlockBefore) {
                return glm::vec3((int)round(rayX - lookingDirection.x), (int)round(rayY - lookingDirection.y), (int)round(rayZ - lookingDirection.z));
            }
            return glm::vec3((int)round(rayX), (int)round(rayY), (int)round(rayZ));
        }

        /**
         * @brief Call this to place the hotbar block in the players looking direction.
         * This function call also deals with rotating blocks and creating light sources
         * 
         * @param renderInfo 
         * @param forcedPlace 
         */
        void rightClickPlace(renderer::renderer_t *renderInfo) {
            swingCycle = 0;
            screenHand.children[handIndex].translation = oldHandPos;
            screenHand.children[handIndex].rotation = oldHandRotation;

            size_t placeX, placeY, placeZ;

            auto placeBlockVector = findCursorBlock(true);
            placeX = (size_t)placeBlockVector.x;
            placeY = (size_t)placeBlockVector.y;
            placeZ = (size_t)placeBlockVector.z;
            
            
            if (isCoordOutBoundaries((int)placeX, (int)placeY, (int)placeZ)) {
                return;
            }
            
            if (terrain.at(placeX).at(placeY).at(placeZ).air) {
                placeBlock(scene::createBlock((int)placeX, (int)placeY, (int)placeZ, hotbar[hotbarIndex], false, !hotbar[hotbarIndex].illuminating));

                // Adding a light source to the block
                if (terrain.at(placeX).at(placeY).at(placeZ).illuminating) {
                    terrain.at(placeX).at(placeY).at(placeZ).lightID = renderInfo->addLightSource(glm::vec3(placeX, placeY, placeZ), hotbar[hotbarIndex].rgb, hotbar[hotbarIndex].intensity);
                    
                    if (terrain.at(placeX).at(placeY).at(placeZ).lightID < 0) {
                        std::cout << "Maximum lights reached, can only have up to " << renderInfo->getMaxLights() << " point lights\n";
                    }
                }
                // If player is inside a block, then destroy the block
                if (checkInsideBlock()) {
                    renderInfo->removeLightSource(terrain.at(placeX).at(placeY).at(placeZ).lightID);
                    terrain.at(placeX).at(placeY).at(placeZ).air = true;
                    terrain.at(placeX).at(placeY).at(placeZ).transparent = true;
                    terrain.at(placeX).at(placeY).at(placeZ).lightID = -1;
                    return;
                } else {
                    if (hotbar[hotbarIndex].rotatable && abs(playerCamera.pitch) <= 35.0f && !shiftMode) {
                        // Rotate the block if it is a block that can be rotated
                        // Blocks will not rotate if shift is being pressed
                        switch (utility::getDirection(playerCamera.yaw)) {
                            case 0:
                                terrain.at(placeX).at(placeY).at(placeZ).rotation = glm::vec3(90.0f, 0.0f, 0.0f);
                                break;
                            case 1:
                                terrain.at(placeX).at(placeY).at(placeZ).rotation = glm::vec3(0.0f, 0.0f, 90.0f);
                                break;
                            case 2:
                                terrain.at(placeX).at(placeY).at(placeZ).rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
                                break;
                            case 3:
                                terrain.at(placeX).at(placeY).at(placeZ).rotation = glm::vec3(0.0f, 0.0f, -90.0f);
                                break;
                        }
                    }
                    if (isUnderwater(placeBlockVector)) {
                        particle::spawnFloatingParticles(&listOfParticles, glm::vec3(placeX, placeY, placeZ), bubble, seaSurface.translation.y);
                    }
                }

                // If program reaches here, the blocks to be rendered must be updated
                updateBlocksToRender(true);
            }
        }

        /**
         * @brief Call this to destroy the block in the players looking direction.
         * If forcedPlace is a valid position, it will destroy the block in that position instead.
         * This function call also deals deleting light sources
         * 
         * @param renderInfo 
         * @param forcedPlace 
         */
        void leftClickDestroy(renderer::renderer_t *renderInfo, glm::vec3 forcedPlace = {-10, -10, -10}) {
            swingCycle = 0;
            screenHand.children[handIndex].translation = oldHandPos;
            screenHand.children[handIndex].rotation = oldHandRotation;
            
            glm::vec3 placeBlockVector;
            if (forcedPlace.x < 0) {
                placeBlockVector = findCursorBlock(false);
            } else {
                placeBlockVector = forcedPlace;
            }
            size_t placeX = (size_t)placeBlockVector.x, placeY = (size_t)placeBlockVector.y, placeZ = (size_t)placeBlockVector.z;
            
            if (isCoordOutBoundaries((int)placeX, (int)placeY, (int)placeZ)) {
                return;
            }

            if (!terrain.at(placeX).at(placeY).at(placeZ).air) {
                // Removes the block from the world and also spawns particles
                auto blockTex = terrain.at(placeX).at(placeY).at(placeZ).textureID;
                terrain.at(placeX).at(placeY).at(placeZ).textureID = 0;
                texture_2d::destroy(terrain.at(placeX).at(placeY).at(placeZ).reflectionTexID);
                terrain.at(placeX).at(placeY).at(placeZ).air = true;
                terrain.at(placeX).at(placeY).at(placeZ).transparent = true;
                terrain.at(placeX).at(placeY).at(placeZ).rotation = glm::vec3(0.0f, 0.0f, 0.0f);
                if (terrain.at(placeX).at(placeY).at(placeZ).lightID != -1) {
                    renderInfo->removeLightSource(terrain.at(placeX).at(placeY).at(placeZ).lightID);
                    terrain.at(placeX).at(placeY).at(placeZ).lightID = -1;
                }
                // If program reaches here, the blocks to be rendered must be updated
                updateBlocksToRender(true);
                particle::spawnBlockBreakParticles(&listOfParticles, placeBlockVector, blockTex);
                if (isUnderwater(placeBlockVector)) {
                    particle::spawnFloatingParticles(&listOfParticles, placeBlockVector, bubble, seaSurface.translation.y);
                }
            }
        }

        /**
         * @brief Call this to change the hotbar block into the block that the player is looking at
         * 
         * @param renderInfo 
         * @param forcedPlace 
         */
        void middleClickPick() {
            auto placeBlockVector = findCursorBlock(false);
            auto placeX = placeBlockVector.x, placeY = placeBlockVector.y, placeZ = placeBlockVector.z;
            
            if (isCoordOutBoundaries(placeX, placeY, placeZ)) {
                return;
            }
            int index = 0;
            bool found = false;
            for (auto i : hotbar) {
                if (i.texture == terrain.at(placeX).at(placeY).at(placeZ).textureID) {
                    hotbarIndex = index;
                    found = true;
                    break;
                }
                index += 1;
            }
            
            if (!found) {
                index = 0;
                for (auto i : hotbarSecondary) {
                    if (i.texture == terrain.at(placeX).at(placeY).at(placeZ).textureID) {
                        switchHotbars();
                        hotbarIndex = index;
                        break;
                    }
                    index += 1;
                }
            }
            scrollHotbar(1);
            scrollHotbar(-1);
        }

        /**
         * @brief Places the block into the world using it's x, y, z values
         * 
         * @param block 
         */
        void placeBlock(node_t block) {
            size_t blockX = (size_t)block.x, blockY = (size_t)block.y, blockZ = (size_t)block.z;
            scene::destroy(&terrain.at(blockX).at(blockY).at(blockZ), false);
            block.transparent = hotbar[(size_t)hotbarIndex].transparent;
            block.illuminating = hotbar[(size_t)hotbarIndex].illuminating;
            terrain.at(blockX).at(blockY).at(blockZ) = block;
            return;
        }

        /**
         * @brief Finds a suitable respawn position of player and teleports them to there
         * If a block is in the way, the block will be deleted. If there aren't any blocks
         * beneath the respawn location, flying mode will be toggled on
         * 
         * @param block 
         */
        void findRespawnPosition(renderer::renderer_t *renderInfo) {
            playerCamera.pos = {round(terrain.size() / 2), 6.0f, round(terrain.at(0).at(0).size() / 2)};
            while (checkInsideBlock()) {
                std::cout << "Some blocks were destroyed to make room for you to respawn in\n";
                leftClickDestroy(renderInfo, playerCamera.pos);
                leftClickDestroy(renderInfo, {round(terrain.size() / 2), 5.0f, round(terrain.at(0).at(0).size() / 2)});
            }
            if (findClosestBlockAboveBelow(-1) < 0) {
                std::cout << "No blocks below detected. Force flying mode enabled\n";
                flyingMode = false;
                toggleMode();
            }
        }

        /**
         * @brief Function that controls the hitboxes and how the player camera moves with WASD, Shift etc.
         * 
         * @param window 
         * @param dt 
         * @param renderInfo 
         */
        void updatePlayerPositions(GLFWwindow *window, float dt, renderer::renderer_t *renderInfo) {
            swingHand(dt);
            // camera angle update is reserved for player.cpp as that does not depend on the terrain
            player::updateCameraAngle(playerCamera, window, dt);

            auto originalPosition = playerCamera.pos;

            //
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && flyingMode) {
                playerCamera.yVelocity = -1.0f;
            } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && flyingMode) {
                playerCamera.yVelocity = 1.0f;
            } else if (flyingMode) {
                playerCamera.yVelocity = 0.0f;
            }

            float step = dt * CAMERA_SPEED * walkingMultiplier;
            // Controls walking multipliers
            if (runningMode && !shiftMode) {
                walkingMultiplier = 1.0f;
            } else if (shiftMode) {
                walkingMultiplier = 0.2f;
            } else {
                walkingMultiplier = 0.5f;
            }

            // Slowing player down iif in water or on slime block
            if (strcmp(blockBelowName().c_str(), "slime_block") == 0 && groundLevel == playerCamera.pos.y - 1.0f) {
                walkingMultiplier *= 0.5f;
            }

            if (isUnderwater() && !flyingMode) {
                walkingMultiplier *= 0.5f;
            }

            if (findClosestBlockAboveBelow(-1) == playerCamera.pos.y - eyeLevel) {
                // Prevent accidentally shifting off a block
                playerCamera.yVelocity = 0.0f;
            }
            
            glm::mat4 trans = glm::translate(glm::mat4(1.0), -playerCamera.pos);
            trans *= glm::rotate(glm::mat4(1.0), -glm::radians(playerCamera.yaw), glm::vec3(0, 1, 0));
            trans *= glm::rotate(glm::mat4(1.0), glm::radians(playerCamera.pitch), glm::vec3(1, 0, 0));

            auto right = glm::vec3(trans[0]);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                playerCamera.pos.z += step * -glm::cos(glm::radians(playerCamera.yaw));
                // Step backwards if player is sneaking and teetering off block, or they are inside a block
                if (checkInsideBlock() || (playerCamera.yVelocity == 0.0f && shiftMode && findClosestBlockAboveBelow(-1) < playerCamera.pos.y - eyeLevel)) {
                    playerCamera.pos.z -= step * -glm::cos(glm::radians(playerCamera.yaw));
                }
                playerCamera.pos.x += step * glm::sin(glm::radians(playerCamera.yaw));
                if (checkInsideBlock() || (playerCamera.yVelocity == 0.0f && shiftMode && findClosestBlockAboveBelow(-1) < playerCamera.pos.y - eyeLevel)) {
                    playerCamera.pos.x -= step * glm::sin(glm::radians(playerCamera.yaw));
                }
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                playerCamera.pos.z -= step * -glm::cos(glm::radians(playerCamera.yaw));
                if (checkInsideBlock() || (playerCamera.yVelocity == 0.0f && shiftMode && findClosestBlockAboveBelow(-1) < playerCamera.pos.y - eyeLevel)) {
                    playerCamera.pos.z += step * -glm::cos(glm::radians(playerCamera.yaw));
                }
                playerCamera.pos.x -= step * glm::sin(glm::radians(playerCamera.yaw));
                if (checkInsideBlock() || (playerCamera.yVelocity == 0.0f && shiftMode && findClosestBlockAboveBelow(-1) < playerCamera.pos.y - eyeLevel)) {
                    playerCamera.pos.x += step * glm::sin(glm::radians(playerCamera.yaw));
                }
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                playerCamera.pos.z += right.z * -step;
                if (checkInsideBlock() || (playerCamera.yVelocity == 0.0f && shiftMode && findClosestBlockAboveBelow(-1) < playerCamera.pos.y - eyeLevel)) {
                    playerCamera.pos.z -= right.z * -step;
                }
                playerCamera.pos.x += right.x * -step;
                if (checkInsideBlock() || (playerCamera.yVelocity == 0.0f && shiftMode && findClosestBlockAboveBelow(-1) < playerCamera.pos.y - eyeLevel)) {
                    playerCamera.pos.x -= right.x * -step;
                }
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                playerCamera.pos.z += right.z * step;
                if (checkInsideBlock() || (playerCamera.yVelocity == 0.0f && shiftMode && findClosestBlockAboveBelow(-1) < playerCamera.pos.y - eyeLevel)) {
                    playerCamera.pos.z -= right.z * step;
                }
                playerCamera.pos.x += right.x * step;
                if (checkInsideBlock() || (playerCamera.yVelocity == 0.0f && shiftMode && findClosestBlockAboveBelow(-1) < playerCamera.pos.y - eyeLevel)) {
                    playerCamera.pos.x -= right.x * step;
                }
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && (playerCamera.yVelocity == 0 || flyingMode)) {
                if (!flyingMode) {
                    playerCamera.yVelocity = JUMP_POWER;
                } else {
                    playerCamera.pos.y += step;
                    if (checkInsideBlock()) {
                        playerCamera.pos.y -= step;
                    }
                }
            } else if (glfwGetKey(window, GLFW_KEY_SPACE) && isUnderwater() && !flyingMode) {
                // Controls swimming
                playerCamera.yVelocity = (checkInsideBlock()) ? -1.5f * JUMP_POWER_UNDERWATER : JUMP_POWER_UNDERWATER;
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && playerCamera.pos.y != (float)groundLevel + eyeLevel) {
                if (flyingMode) {
                    playerCamera.pos.y -= step;
                    if (playerCamera.pos.y < (float)groundLevel + eyeLevel) {
                        playerCamera.pos.y = groundLevel + eyeLevel;
                    }
                }
            }
            updateBlocksToRender();
            // If the positions differ, then bob the hand
            if (originalPosition.x != playerCamera.pos.x || playerCamera.pos.z != originalPosition.z) {
                player.walkAnimation(dt, runningMode);
            } else {
                player.idleAnimation(dt, swingCycle);
            }
            if (originalPosition != playerCamera.pos && playerCamera.pos.y == (float)groundLevel + eyeLevel) {
                if (rand() % 2 == 0 && runningMode) {
                    // Spawn "footprint" particle with 1/2 chance
                    particle::spawnImpactParticles(
                        &listOfParticles,
                        glm::vec3(playerCamera.pos.x,(float)groundLevel - 0.5f, playerCamera.pos.z),
                        blockBelowTexID(),
                        rand() % 2 + 1
                    );
                }
                bobHand(dt);
            }
            player.tiltHead(playerCamera.pitch);
            player.rotateBody(playerCamera.yaw);

            if (glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS) {
                runningMode = false;
            }

            // Controls sneaking
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !flyingMode) {
                eyeLevel = 0.75f;
                shiftMode = true;
            } else {
                eyeLevel = 1.0f;
                shiftMode = false;
            }

            if (rand() % 10 == 0 && isUnderwater()) {
                // Spawn bubbles around player
                particle::spawnFloatingParticles(&listOfParticles, playerCamera.pos, bubble, seaSurface.translation.y, 3);
            }
            // Enacting gravity onto the camera
            if (!flyingMode) {

                playerCamera.pos.y += playerCamera.yVelocity * dt;
                if (isUnderwater()) {
                    
                    playerCamera.yVelocity += GRAVITY * dt;
                } else {
                    playerCamera.yVelocity += GRAVITY * dt;
                }
                // Enacting player terminal velocity
                auto terminalVelocity = isUnderwater() ? 2.0f : 40.0f;
                playerCamera.yVelocity = glm::clamp(playerCamera.yVelocity, -terminalVelocity, terminalVelocity);

                if (playerCamera.pos.y < (float)groundLevel + eyeLevel) {
                    // Prevents player from falling through the ground

                    if (abs(playerCamera.yVelocity) >= 7.0f) {
                        particle::spawnImpactParticles(&listOfParticles, glm::vec3(playerCamera.pos.x, (float)groundLevel - 0.5f, playerCamera.pos.z), blockBelowTexID());
                    }

                    playerCamera.pos.y = (float)groundLevel + eyeLevel;
                    // Controls the bounciness of the slime block
                    if (strcmp(blockBelowName().c_str(), "slime_block") == 0) {
                        playerCamera.yVelocity = abs(playerCamera.yVelocity) * 0.75f + GRAVITY * dt;
                        if (playerCamera.yVelocity >= 4.0f && !shiftMode) {
                            // Enacting gravity
                            playerCamera.pos.y += playerCamera.yVelocity * dt;
                            playerCamera.yVelocity += GRAVITY * dt;
                            
                        }
                    } else {
                        playerCamera.yVelocity = 0.0f;
                    }

                }
            }

            // World boundaries. Respawns when the y co-ordinate goes too low
            // || playerCamera.pos.y > WORLD_HEIGHT - 1
            if (playerCamera.pos.y - eyeLevel < VOID_LEVEL) {
                std::cout << "Respawned, you went too low!\n";

                findRespawnPosition(renderInfo);
                groundLevel = findClosestBlockAboveBelow(-1);

                if (groundLevel > 0) {
                    playerCamera.pos.y = groundLevel;
                } else {
                    playerCamera.pos.y = 4;
                }

            }

            groundLevel = findClosestBlockAboveBelow(-1);
            aboveLevel = findClosestBlockAboveBelow(1);

            // Below keeps player steady on ground
            if (playerCamera.pos.y - eyeLevel < groundLevel) {
                playerCamera.pos.y = groundLevel + eyeLevel;
                playerCamera.yVelocity = 0;
            }
            if (playerCamera.yVelocity > 0) {
                // Below keeps player under the ceiling
                if (playerCamera.pos.y + 1.0f >= aboveLevel) {
                    if (flyingMode) {
                        playerCamera.pos.y = aboveLevel - 1;
                    }
                    playerCamera.yVelocity *= -0.5;
                }
            }
        
        }

        /**
         * @brief Checks if the given x, y, z co-ordinates exists in the terrain/world
         * 
         * @param x 
         * @param y 
         * @param z 
         * @return true if out of bounds
         * @return false if inside bounds
         */
        bool isCoordOutBoundaries(int x, int y, int z) {
            try {
                terrain.at(x).at(y).at(z).air = terrain.at(x).at(y).at(z).air;
            } catch (std::out_of_range) {
                return true;
            }
            return false;
        }

        std::string blockBelowName() {
            if (!isCoordOutBoundaries(round(playerCamera.pos.x), groundLevel - 1, round(playerCamera.pos.z))) {
                return terrain.at(round(playerCamera.pos.x)).at(groundLevel - 1).at(round(playerCamera.pos.z)).name;
            } else {
                return "";
            }
        }

        /**
         * @brief Gets the texture ID of the block below the player
         * 
         * @return GLuint 
         */
        GLuint blockBelowTexID() {
            glm::vec3 blockPos = {round(playerCamera.pos.x), groundLevel - 1, round(playerCamera.pos.z)};
    
            if (!isCoordOutBoundaries(blockPos.x, blockPos.y, blockPos.z)) {
                if (terrain.at(blockPos.x).at(blockPos.y).at(blockPos.z).textureID != 0) {
                    return terrain.at(blockPos.x).at(blockPos.y).at(blockPos.z).textureID;
                }
            }
            
            for (int x = -1; x <= 1; x++) {
                for (int z = -1; z <= 1; z++) {
                    if (x == z) continue;
                    if (isCoordOutBoundaries(blockPos.x + x, blockPos.y, blockPos.z + z)) continue;
                    auto id = terrain.at(blockPos.x + x).at(blockPos.y).at(blockPos.z + z).textureID;
                    if (id != 0) {
                        return id;
                    }
                }
            }
            return 0;
        }

        /**
         * @brief Checks if the player is inside a block
         * 
         * @return true if the player is clipping into a block
         * @return false if the player is not clipping into a block
         */
        bool checkInsideBlock() {
            
            float playerPosY = playerCamera.pos.y - eyeLevel;
            float playerPosX = playerCamera.pos.x;
            float playerPosZ = playerCamera.pos.z;

            size_t xPosRd = (size_t)round(playerPosX);
            size_t yPosRd = (size_t)round(playerPosY);
            size_t zPosRd = (size_t)round(playerPosZ);

            // Checks around the player in a circle if they are touching a block or not
            for (float degree = 0; degree < 360.0f; degree += 5.0f) {
                xPosRd = (size_t)round(playerPosX + PLAYER_RADIUS * (float)glm::sin(glm::radians(degree)));
                zPosRd = (size_t)round(playerPosZ + PLAYER_RADIUS * (float)-glm::cos(glm::radians(degree)));
                if (!isCoordOutBoundaries(xPosRd, yPosRd, zPosRd) && !terrain.at(xPosRd).at(yPosRd).at(zPosRd).air) {
                    return true;
                }
                if (!isCoordOutBoundaries(xPosRd, yPosRd + 1, zPosRd) && !terrain.at(xPosRd).at(yPosRd + 1).at(zPosRd).air) {
                    return true;
                }
            }
            return false;
            
        }
        
        /**
         * @brief Find the highest or lowest level the player can go with their x and z co-ordinate
         * 
         * @param direction (Positive to find the closest block above. Negative to find the closest block below)
         * @return int 
         */
        int findClosestBlockAboveBelow(int direction, glm::vec3 pos = {-999, -999, -999}) {
            float posX, posY, posZ;
            if (pos.x == -999 && pos.y == -999 && pos.z == -999) {
                posY = playerCamera.pos.y - eyeLevel;
                posX = playerCamera.pos.x;
                posZ = playerCamera.pos.z;
            } else {
                posY = pos.y;
                posX = pos.x;
                posZ = pos.z;
            }
            

            int maxInt = (direction < 0) ? 1 : WORLD_HEIGHT;

            for (int i = (int)posY; i * direction < maxInt; i += direction) {
                int xPosRd = (int)round(posX);
                int yPosRd = (int)round(i);
                int zPosRd = (int)round(posZ);

                // Checks around the player in a circle if they are touching a block or not
                for (float degree = 0; degree < 360.0f; degree += 5.0f) {
                    xPosRd = (int)round(posX + PLAYER_RADIUS * (float)glm::sin(glm::radians(degree)));
                    zPosRd = (int)round(posZ + PLAYER_RADIUS * (float)-glm::cos(glm::radians(degree)));
                    if (isCoordOutBoundaries(xPosRd, yPosRd, zPosRd)) {
                        continue;
                    }
                    if (!terrain.at(xPosRd).at(yPosRd).at(zPosRd).air) {
                        // Correcting
                        if (direction < 0) {
                            return yPosRd - direction;
                        } else {
                            return yPosRd;
                        }
                        
                    }
                }
                
            }
            
            return (int)direction * (int)WORLD_HEIGHT * (int)WORLD_HEIGHT;
        }

        /**
         * @brief Checks if the player is under the water surface or not
         * 
         * @return true 
         * @return false 
         */
        bool isUnderwater() {
            return seaSurface.translation.y >= glm::round(playerCamera.pos.y - 0.03f);
        }

        /**
         * @brief Checks if the position given is under water or not
         * 
         * @param pos 
         * @return true 
         * @return false 
         */
        bool isUnderwater(glm::vec3 pos) {
            return seaSurface.translation.y >= glm::round(pos.y - 0.03f);
        }

        /**
         * @brief Draws all the particles inside listOfParticles. Does not draw anything if the shader given is not a particle render
         * 
         * @param particleRender 
         * @param proj 
         * @param dt 
         * @param animate 
         */
        void drawParticles(renderer::renderer_t particleRender, glm::mat4 proj, float dt, bool animate = true) {
            if (strcmp(particleRender.type.c_str(), "particle") != 0) return;
            particle::drawAllParticles(&listOfParticles, particleRender, getCurrCamera()->get_view(), proj, GRAVITY, dt, animate);
        }

        /**
         * @brief Draws the terrain, HUD, lighting effects etc.
         * 
         * @param renderInfo 
         */
        void drawWorld(renderer::renderer_t renderInfo, bool onlyIlluminating = false, bool drawHand = false) {

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (strcmp(renderInfo.type.c_str(), "default") == 0) {
                renderInfo.setBasePters(getCurrCamera()->pos);
                if (drawCelestials) {
                    glDepthRange(0.999,1);
                    drawElement(&centreOfWorld, glm::mat4(1.0f), renderInfo);
                    glDepthRange(0,1);
                }
            }
            drawTerrain(glm::mat4(1.0f), renderInfo, onlyIlluminating, getCurrCamera());

            // Draw the player if we are rendering shadow
            if (strcmp(renderInfo.type.c_str(), "shadow") == 0 && !cutsceneEnabled) {
                player.moveBody(playerCamera.pos, shiftMode, worldTime);
                drawElement(&player.positionInWorld, glm::mat4(1.0f), renderInfo);
            }

            // Draw bed if cutscene is occuring
            if (cutsceneEnabled && onlyIlluminating) {
                renderInfo.setInt("forceBlack", true);
                drawElement(&bed, glm::mat4(1.0f), renderInfo);
                renderInfo.setInt("forceBlack", false);
            }

            //drawElement(&seaSurface, glm::mat4(1.0f), renderInfo);
            return;
        }

        /**
         * @brief Draws the hand seen on the bottom right of the screen
         * 
         * @param renderInfo 
         */
        void drawHand(renderer::renderer_t renderInfo) {
            // Drawing the 3D hand on screen
            renderInfo.setInt("affectedByShadows", false);
            screenHand.translation.x = playerCamera.pos.x;
            screenHand.translation.z = playerCamera.pos.z;
            screenHand.translation.y = utility::lerp(screenHand.translation.y, playerCamera.pos.y, 0.75f);
            screenHand.rotation.x = utility::lerp(screenHand.rotation.x, playerCamera.pitch, 0.65f);
            auto newRot = -playerCamera.yaw;
            if (screenHand.rotation.y - newRot > 180.0f) {
                screenHand.rotation.y -= 360.0f;
            } else if (newRot - screenHand.rotation.y > 180.0f) {
                screenHand.rotation.y += 360.0f;
            }
            screenHand.rotation.y = utility::lerp(screenHand.rotation.y, -playerCamera.yaw, 0.65f);
            drawElement(&screenHand, glm::mat4(1.0f), renderInfo);
            renderInfo.setInt("affectedByShadows", true);
        }

        /**
         * @brief Returns the position of the centre of the world, disregarding the y values
         * 
         * @return glm::vec3 
         */
        glm::vec3 getCentreOfWorld() {
            return {terrain.size() / 2, 0, terrain.at(0).at(0).size() / 2};
        }

        /**
         * @brief Get how big the generated world is
         * 
         * @return float 
         */
        float getWorldSize() {
            return terrain.size();
        }

        /**
         * @brief Draws whatever is currently inside listOfBlocksToRender
         * Also calculates the degree between the block and the players looking vector to determine
         * if the block is within the players view point
         * 
         * @param parent_mvp 
         * @param renderInfo 
         */
        void drawTerrain(const glm::mat4 &parent_mvp, renderer::renderer_t renderInfo, bool onlyIlluminating, player::playerPOV *cam, bool ignoreFrustum = false) {

            for (size_t i = 0; i < listOfBlocksToRender.size(); i++) {
                float x = listOfBlocksToRender[i]->translation.x;
                float y = listOfBlocksToRender[i]->translation.y;
                float z = listOfBlocksToRender[i]->translation.z;
                
                if (utility::calculateDistance(glm::vec3(x, y, z), cam->pos) <= renderDistance) {
                    
                    if (!(strcmp(renderInfo.type.c_str(), "shadow") == 0 && listOfBlocksToRender[i]->transparent)) {
                        
                        if (strcmp(renderInfo.type.c_str(), "shadow") == 0) {
                            drawBlock(listOfBlocksToRender[i], parent_mvp, renderInfo, onlyIlluminating);
                        } else if (ignoreFrustum || frustum::isBlockInView(player::getLookingDirection(cam, 1), glm::vec3(x, y, z), cam->pos)) {
                            drawBlock(listOfBlocksToRender[i], parent_mvp, renderInfo, onlyIlluminating);
                        } else if (utility::calculateDistance(glm::vec3(x, y, z), cam->pos) <= 2.0f) {
                            drawBlock(listOfBlocksToRender[i], parent_mvp, renderInfo, onlyIlluminating);
                        }
                    }
                }
            }
        }

        /**
         * @brief Draws all the transparent bocks
         * 
         * @param parent_mvp 
         * @param renderInfo 
         * @param onlyIlluminating 
         */
        void drawTransTerrain(const glm::mat4 &parent_mvp, renderer::renderer_t renderInfo, bool onlyIlluminating) {

            // Draw bed if cutscene is occuring
            if (cutsceneEnabled) {
                drawElement(&bed, glm::mat4(1.0f), renderInfo);
            }

            for (size_t i = 0; i < listOfTransBlocksToRender.size(); i++) {
                float x = listOfTransBlocksToRender[i]->translation.x;
                float y = listOfTransBlocksToRender[i]->translation.y;
                float z = listOfTransBlocksToRender[i]->translation.z;
                
                if (utility::calculateDistance(glm::vec3(x, y, z), getCurrCamera()->pos) <= renderDistance) {
                    
                    if (!(strcmp(renderInfo.type.c_str(), "shadow") == 0 && listOfBlocksToRender[i]->transparent)) {

                        if (strcmp(renderInfo.type.c_str(), "shadow") == 0) {
                            drawBlock(listOfTransBlocksToRender[i], parent_mvp, renderInfo, onlyIlluminating);
                        } else if (frustum::isBlockInView(player::getLookingDirection(getCurrCamera(), 1), glm::vec3(x, y, z), getCurrCamera()->pos)) {
                            drawBlock(listOfTransBlocksToRender[i], parent_mvp, renderInfo, onlyIlluminating);
                        } else if (utility::calculateDistance(glm::vec3(x, y, z), getCurrCamera()->pos) <= 2.0f) {
                            drawBlock(listOfTransBlocksToRender[i], parent_mvp, renderInfo, onlyIlluminating);
                        }
                    }
                }
            }
            // Drawing highlight around selected block as that is transparent as well
            if (!shiftMode && strcmp(renderInfo.type.c_str(), "default") == 0) {
                // Drawing the highlighted block if shift mode is not enabled
                highlightedBlock.translation = findCursorBlock(false);
                auto pos = highlightedBlock.translation;
                if (!isCoordOutBoundaries(pos.x, pos.y, pos.z)) {
                    drawElement(&highlightedBlock, glm::mat4(1.0f), renderInfo);
                }
            }
        }

        /**
         * @brief Draws shiny terrain aas if it was a solid block
         * 
         * @param parent_mvp 
         * @param renderInfo 
         * @param onlyIlluminating 
         */
        void drawShinyTerrainNormally(const glm::mat4 &parent_mvp, renderer::renderer_t renderInfo, bool onlyIlluminating) {

            for (size_t i = 0; i < listOfShinyBlocksToRender.size(); i++) {
                float x = listOfShinyBlocksToRender[i]->translation.x;
                float y = listOfShinyBlocksToRender[i]->translation.y;
                float z = listOfShinyBlocksToRender[i]->translation.z;
                
                if (utility::calculateDistance(glm::vec3(x, y, z), getCurrCamera()->pos) <= renderDistance) {
                    
                    if (!(strcmp(renderInfo.type.c_str(), "shadow") == 0 && listOfBlocksToRender[i]->transparent)) {

                        if (strcmp(renderInfo.type.c_str(), "shadow") == 0) {
                            drawBlock(listOfShinyBlocksToRender[i], parent_mvp, renderInfo, onlyIlluminating);
                        } else if (frustum::isBlockInView(player::getLookingDirection(getCurrCamera(), 1), glm::vec3(x, y, z), getCurrCamera()->pos)) {
                            drawBlock(listOfShinyBlocksToRender[i], parent_mvp, renderInfo, onlyIlluminating);
                        } else if (utility::calculateDistance(glm::vec3(x, y, z), getCurrCamera()->pos) <= 2.0f) {
                            drawBlock(listOfShinyBlocksToRender[i], parent_mvp, renderInfo, onlyIlluminating);
                        }
                    }
                }
            }
        }

        /**
         * @brief Re-renders the environment reflection textures of all shiny terrain.
         * 
         * @param defaultRender 
         * @param skyColor 
         * @param winSize 
         */
        void updateShinyTerrain(renderer::renderer_t defaultRender, glm::vec3 skyColor,glm::vec2 winSize) {
            for (size_t i = 0; i < listOfShinyBlocksToRender.size(); i++) {

                if (listOfShinyBlocksToRender.at(i)->reflectionTexID != 0) {
                    // Delete the current texture if there was any
                    texture_2d::destroy(listOfShinyBlocksToRender.at(i)->reflectionTexID);
                }
                listOfShinyBlocksToRender.at(i)->reflectionTexID = texture_2d::createEmptyCubeMap(REFLECTION_SIZE);
                renderToEnvironmentMap(
                    listOfShinyBlocksToRender.at(i)->reflectionTexID,
                    listOfShinyBlocksToRender.at(i)->translation,
                    defaultRender,
                    skyColor,
                    winSize
                );

            }
        }

        /**
         * @brief Draws shiny terrain with the mirror greenscreen texture and reflecting its cubemap
         * 
         * @param viewProj 
         * @param renderInfo 
         * @param forceMap 
         */
        void drawShinyTerrain(const glm::mat4 &viewProj, renderer::renderer_t renderInfo, GLuint forceMap = 0) {
            for (size_t i = 0; i < listOfShinyBlocksToRender.size(); i++) {
                
                renderInfo.activate();

                // Drawing the reflection
                auto model = glm::mat4(1.0f);
                model *= glm::translate(glm::mat4(1.0), listOfShinyBlocksToRender[i]->translation);
                model *= glm::scale(glm::mat4(1.0), listOfShinyBlocksToRender[i]->scale);
                model *= glm::rotate(glm::mat4(1.0), glm::radians(listOfShinyBlocksToRender[i]->rotation.z), glm::vec3(0, 0, 1));
                model *= glm::rotate(glm::mat4(1.0), glm::radians(listOfShinyBlocksToRender[i]->rotation.y), glm::vec3(0, 1, 0));
                model *= glm::rotate(glm::mat4(1.0), glm::radians(listOfShinyBlocksToRender[i]->rotation.x), glm::vec3(1, 0, 0));

                // Rendering the shiny bits
                renderInfo.setMat4("uViewProjReflection", viewProj);
                renderInfo.setMat4("uModel", model);
                renderInfo.setInt("environmentMap", 0);
                renderInfo.setInt("uTex", 1);
                renderInfo.setVec3("cameraPos", getCurrCamera()->pos);
                glActiveTexture(GL_TEXTURE0);
                if (forceMap != 0) {
                    glBindTexture(GL_TEXTURE_CUBE_MAP, forceMap);
                } else {
                    glBindTexture(GL_TEXTURE_CUBE_MAP, listOfShinyBlocksToRender.at(i)->reflectionTexID);
                }
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mirrorGreenscreen);
                listOfShinyBlocksToRender.at(i)->ignoreCulling = true;
                
                glBindVertexArray(listOfShinyBlocksToRender[i]->mesh.vao);
                glDrawElements(GL_TRIANGLES, listOfShinyBlocksToRender[i]->mesh.indices_count, GL_UNSIGNED_INT, nullptr);
                glBindVertexArray(0);
            }
        }

        /**
         * @brief Renders the environment around the given centre to the given cubeMap
         * 
         * @param cubeMap 
         * @param centre 
         * @param basicShader 
         * @param skyColor 
         * @param winSize, with x as the width, as the height
         */
        void renderToEnvironmentMap (
            GLuint cubeMap,
            glm::vec3 centre,
            renderer::renderer_t basicShader,
            glm::vec3 skyColor,
            glm::vec2 winSize
        ) {
            // std::cout << cubeMap << "\n";
            glm::mat4 projMatrix = glm::perspective(glm::radians(90.0), 1.0, 0.1, 50.0);
            GLuint fbo, rbo;
            glGenFramebuffers(1, &fbo);
            glGenRenderbuffers(1, &rbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubeMap, 0);
            
            glBindRenderbuffer(GL_FRAMEBUFFER, rbo);
            // Texture size as REFLECTION_SIZE
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, winSize.x, winSize.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

            auto cubemapCamera = player::createCamera(glm::vec3(centre.x, centre.y, centre.z), glm::vec3(centre.x, centre.y, centre.z));

            for (int i = 0; i < 6; i++) {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    cubeMap,
                    0
                );
                glViewport(0, 0, REFLECTION_SIZE, REFLECTION_SIZE);
                cubemapCamera.roll = 0;
                switch(i) {
                    case 0: // posX
                        cubemapCamera.pitch = 0.0f;
                        cubemapCamera.yaw = 90.0f;
                        cubemapCamera.roll = 180.0f;
                        break;
                    case 1: // negX
                        cubemapCamera.pitch = 0.0f;
                        cubemapCamera.yaw = 270.0f;
                        cubemapCamera.roll = 180.0f;
                        break;
                    case 2: // posY
                        cubemapCamera.pitch = 90.0f;
                        cubemapCamera.yaw = 0.0f;
                        break;
                    case 3: // negY
                        cubemapCamera.pitch = -90.0f;
                        cubemapCamera.yaw = 0.0f;
                        break;
                    case 4: // posZ
                        cubemapCamera.pitch = 0.0f;
                        cubemapCamera.yaw = 180.0f;
                        cubemapCamera.roll = 180.0f;
                        break;
                    case 5: // negZ
                        cubemapCamera.pitch = 0.0f;
                        cubemapCamera.yaw = 0.0f;
                        cubemapCamera.roll = 180.0f;
                        break;
                }
                glm::mat4 projViewMatrix = projMatrix * cubemapCamera.get_view();
                
                basicShader.activate();

                glClearColor(skyColor.r, skyColor.g, skyColor.b, 1);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                basicShader.setMat4("uViewProj", projViewMatrix);

                drawTerrain(glm::mat4(1.0f), basicShader, false, &cubemapCamera, true);
                
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindRenderbuffer(GL_FRAMEBUFFER, 0);
            
            chicken3421::delete_framebuffer(fbo);
            chicken3421::delete_renderbuffer(rbo);
        }

        /**
         * @brief Renders the skybox to be behind everything
         * 
         * @param shader 
         * @param proj 
         * @param prevTex 
         * @param currTex 
         * @param blendValue 
         */
        void drawSkyBox(renderer::renderer_t shader, glm::mat4 proj, GLuint prevTex, GLuint currTex, GLfloat blendValue) {
            glDepthFunc(GL_LEQUAL);
            shader.activate();
            auto skyBoxView = glm::mat4(glm::mat3(getCurrCamera()->get_view()));
            shader.setMat4("uView", skyBoxView);
            shader.setMat4("uProjection", proj);
            glBindVertexArray(skyBox.mesh.vao);
            shader.setInt("prevSkybox", 0);
            shader.setInt("currSkybox", 1);
            shader.setFloat("blendFactor", blendValue);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, prevTex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, currTex);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS);
            return;
        }

        /**
         * @brief Spawns particles around all blocks that are within rendering distance if the block meets
         * a certain criteria
         */
        void spawnBlockParticles() {
            for (auto block : listOfBlocksToRender) {
                auto pos = block->translation;
                if (strcmp(block->name.c_str(), "magma") == 0) {
                    // Spawns either bubbles or smoke depending on if it's underwater or not
                    if (rand() % 15 == 0) {
                        if (isUnderwater(pos)) {
                            particle::spawnFloatingParticles(&listOfParticles, pos, bubble, seaSurface.translation.y);
                        } else {
                            particle::spawnSlowFloatingParticles(&listOfParticles, pos, smokeParticles[rand() % TOTAL_SMOKE], findClosestBlockAboveBelow(1, glm::vec3(pos.x, pos.y + 1, pos.z)) - 0.5f);
                        }
                    }
                } else if (strcmp(block->name.c_str(), "crying_obsidian") == 0) {
                    // Spawning purple obsidian tears
                    if (!isUnderwater(pos) && rand() % 30 == 0) {
                        particle::spawnDripParticles(&listOfParticles, pos, tear, findClosestBlockAboveBelow(-1, glm::vec3(pos.x, pos.y - 1, pos.z)) - 0.5f);
                    }
                } else if (strcmp(block->name.c_str(), "redstone_ore") == 0) {
                    // Spawning red dust around redstone
                    if (rand() % 30 == 0) {
                        particle::spawnBlockAmbientParticle(&listOfParticles, pos, dust[rand() % TOTAL_DUST], glm::vec3(255.0f, 0.0f, 0.0f) / 255.0f);
                    }
                } else if (strcmp(block->name.c_str(), "marccoin_block") == 0) {
                    // Spawning glitter
                    if (rand() % 30 == 0) {
                        particle::spawnBlockAmbientParticle(&listOfParticles, pos, glint, glm::vec3(255.0f, 215.0f, 0.0f) / 255.0f);
                    }
                } else if (block->name.rfind("coral", 0) == 0) {
                    // Spawning fish around coral
                    if (isUnderwater(pos)) {
                        if (rand() % 100 == 0) particle::spawnParticleAround(&listOfParticles, pos, fish[rand() % TOTAL_FISH], seaSurface.translation.y);
                        if (rand() % 100 == 0) particle::spawnParticleAround(&listOfParticles, pos, fish[rand() % TOTAL_FISH], seaSurface.translation.y);
                        if (rand() % 100 == 0) particle::spawnParticleAround(&listOfParticles, pos, fish[rand() % TOTAL_FISH], seaSurface.translation.y);
                    }
                } else if (utility::isInRange(worldTime, 0.65f, 0.9f) && rand() % 1000 == 0 && rand() % 10 == 0 && !isUnderwater(pos)) {
                    // Spawning fire flies during the night around blocks
                    particle::spawnAmbientParticle(&listOfParticles, pos, firefly[rand() % TOTAL_FIREFLY], seaSurface.translation.y);
                }
            }
        }


        /**
         * @brief Call this to update the listOfBlocks to render. Only works if the last rendered position is far away enough from
         * the player's current position. forceRender = true to ignore this
         * 
         * @param forceRender 
         */
        void updateBlocksToRender(bool forceRender = false) {

            // Don't bother to render if the last rendered position is less than (renderDistance / 10) blocks away
            // unless force render is true
            if (!forceRender && utility::calculateDistance(getCurrCamera()->pos, lastRenderedPos) <= (float)(renderDistance / 10.0f)) return;

            listOfBlocksToRender.clear();
            listOfTransBlocksToRender.clear();
            listOfShinyBlocksToRender.clear();

            std::vector<glm::vec3> transparentBlocks;
            int width = worldWidth, height = WORLD_HEIGHT;
            auto minY = (int)std::max(0, (int)(getCurrCamera()->pos.y - renderDistance));
            auto maxY = (int)std::min((int)(getCurrCamera()->pos.y + renderDistance), height);
            auto minX = (int)std::max(0, (int)(getCurrCamera()->pos.x - renderDistance));
            auto maxX = (int)std::min((int)(getCurrCamera()->pos.x + renderDistance), width);
            auto minZ = (int)std::max(0, (int)(getCurrCamera()->pos.z - renderDistance));
            auto maxZ = (int)std::min((int)(getCurrCamera()->pos.z + renderDistance), width);
            glm::vec2 yRange = glm::vec2(minY, maxY);
            glm::vec2 xRange = glm::vec2(minX, maxX);
            glm::vec2 zRange = glm::vec2(minZ, maxZ);

            for (int y = yRange.x; y < yRange.y; y++) {
                for (int z = zRange.x; z < zRange.y; z++) {
                    for (int x = xRange.x; x < xRange.y; x++) {

                        if (isCoordOutBoundaries(x, y, z) || terrain.at(x).at(y).at(z).air) {
                            continue;
                        }
                        
                        if (terrain.at(x).at(y).at(z).transparent) {
                            terrain.at(x).at(y).at(z).culledFaces = {true, true, true, true, true, true};

                            getHiddenFaces(x, y, z, terrain.at(x).at(y).at(z).culledFaces, false);

                            if (utility::countFalses(terrain.at(x).at(y).at(z).culledFaces) < 6) {
                                listOfTransBlocksToRender.push_back(&terrain.at(x).at(y).at(z));
                            }
                            continue;
                        }

                        // Figuring out which sides should be rendered or not.
                        // Only render side if it has air next to it
                        terrain.at(x).at(y).at(z).culledFaces = {true, true, true, true, true, true};

                        getHiddenFaces(x, y, z, terrain.at(x).at(y).at(z).culledFaces, true);

                        if (utility::countFalses(terrain.at(x).at(y).at(z).culledFaces) < 6) {
                            if (strcmp(terrain.at(x).at(y).at(z).name.c_str(), "mirror") == 0) {
                                listOfShinyBlocksToRender.push_back(&terrain.at(x).at(y).at(z));
                            } else {
                                listOfBlocksToRender.push_back(&terrain.at(x).at(y).at(z));
                            }
                        }

                    }
                }
            }

            lastRenderedPos = getCurrCamera()->pos;
        }

        /**
         * @brief Stores the hidden faces inside the given std::vector
         * 
         * @param x 
         * @param y 
         * @param z 
         * @param faces 
         * @param glassIncluded 
         */
        void getHiddenFaces(int x, int y, int z, std::vector<bool> &faces, bool glassIncluded) {
            if (!isCoordOutBoundaries(x, y - 1, z) && !(terrain.at(x).at(y - 1).at(z).air || strcmp(terrain.at(x).at(y - 1).at(z).name.c_str(), "mirror") == 0 || glassIncluded * terrain.at(x).at(y - 1).at(z).transparent)) {
                faces[0] = false; // 0 bottom
            }
            if (!isCoordOutBoundaries(x, y + 1, z) && !(terrain.at(x).at(y + 1).at(z).air || strcmp(terrain.at(x).at(y + 1).at(z).name.c_str(), "mirror") == 0 || glassIncluded * terrain.at(x).at(y + 1).at(z).transparent)) {
                faces[1] = false; // 1 Is top
            }
            if (!isCoordOutBoundaries(x, y, z + 1) && !(terrain.at(x).at(y).at(z + 1).air || strcmp(terrain.at(x).at(y).at(z + 1).name.c_str(), "mirror") == 0 || glassIncluded * terrain.at(x).at(y).at(z + 1).transparent)) {
                faces[2] = false;
            }
            if (!isCoordOutBoundaries(x, y, z - 1) && !(terrain.at(x).at(y).at(z - 1).air || strcmp(terrain.at(x).at(y).at(z - 1).name.c_str(), "mirror") == 0 || glassIncluded * terrain.at(x).at(y).at(z - 1).transparent)) {
                faces[3] = false;
            }
            if (!isCoordOutBoundaries(x + 1, y, z) && !(terrain.at(x + 1).at(y).at(z).air || strcmp(terrain.at(x + 1).at(y).at(z).name.c_str(), "mirror") == 0 || glassIncluded * terrain.at(x + 1).at(y).at(z).transparent)) {
                faces[4] = false;
            }
            if (!isCoordOutBoundaries(x - 1, y, z) && !(terrain.at(x - 1).at(y).at(z).air || strcmp(terrain.at(x - 1).at(y).at(z).name.c_str(), "mirror") == 0 || glassIncluded * terrain.at(x - 1).at(y).at(z).transparent)) {
                faces[5] = false;
            }
        }

        /**
         * @brief Call this to draw the HUD and other elements which rotate in respect to the player's view point
         * 
         * @param parent_mvp 
         * @param renderInfo 
         */
        void drawScreen(const glm::mat4 &parent_mvp, renderer::renderer_t renderInfo) {
            if (hideScreen) return;
            screen.translation = playerCamera.pos;
            screen.rotation.x = playerCamera.pitch;
            screen.rotation.y = -playerCamera.yaw;

            drawElement(&screen, parent_mvp, renderInfo);
        }

        /**
         * @brief Prints out a list of code lines to be used for copying and pasting into preset worlds
         * 
         */
        void convertCurrWorldIntoData() {
            for (size_t x = 0; x < terrain.size(); x++) {
                for (size_t y = 0; y < terrain.at(0).size(); y++) {
                    for (size_t z = 0; z < terrain.at(0).at(0).size(); z++) {
                        if (!terrain.at(x).at(y).at(z).air) {
                            std::string code = "listOfBlocks.emplace_back(scene::miniBlockData(\"" + terrain.at(x).at(y).at(z).name + "\", glm::vec3(";
                            std::cout << code.c_str() << x - (float)terrain.size() / 2.0f << ", " << y << ", " << z - (float)terrain.at(0).at(0).size() / 2.0f << "), true));\n";
                        }
                    }  
                }
            }
        }

        /**
         * @brief Destroys all meshes and maps used for this world
         * 
         */
        void destroyEverthing() {
            for (size_t x = 0; x < terrain.size(); x++) {
                for (size_t y = 0; y < terrain.at(0).size(); y++) {
                    for (size_t z = 0; z < terrain.at(0).at(0).size(); z++) {
                        destroy(&terrain.at(x).at(y).at(z), true);
                    }  
                }
            }
            destroy(&bed, true);
            destroy(&centreOfWorld, true);
            destroy(&screen, true);
            destroy(&highlightedBlock, true);
            texture_2d::destroy(bubble);
            texture_2d::destroy(tear);
            texture_2d::destroy(glint);

            for (auto i : moonPhases) {
                texture_2d::destroy(i);
            }
            texture_2d::destroy(mirrorGreenscreen);
            for (int i = 0; i < TOTAL_SMOKE; i++) {
                texture_2d::destroy(smokeParticles[i]);
            }
            for (int i = 0; i < TOTAL_DUST; i++) {
                texture_2d::destroy(dust[i]);
            }
            for (int i = 0; i < TOTAL_FIREFLY; i++) {
                texture_2d::destroy(firefly[i]);
            }
            for (int i = 0; i < TOTAL_FISH; i++) {
                texture_2d::destroy(fish[i]);
            }

            particle::destroyAllParticles(&listOfParticles);
        }
    };

}
#endif //COMP3421_SCENE_OBJECT_HPP