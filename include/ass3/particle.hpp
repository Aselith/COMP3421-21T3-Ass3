#ifndef COMP3421_ASS3_PARTICLE_HPP
#define COMP3421_ASS3_PARTICLE_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ass3/static_mesh.hpp>
#include <ass3/renderer.hpp>

namespace particle {
    // Stores all data about a single particle
    struct particle_t {
        static_mesh::mesh_t mesh;
        GLuint textureID = 0;
        glm::vec3 translation = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
        glm::vec3 color = glm::vec3(0.0f);
        std::string name;
        GLfloat yVelocity, zVelocity, xVelocity;
        GLfloat minimumYvalue;
        GLfloat maximumYvalue;
        bool affectedByGravity = true, affectedByLight = true;
        GLuint bounceCount = 0.0f;
        GLfloat lifeTimer = 0.0f;
        
    };

    /**
     * @brief Spawns particles that have short lifespan and bursts out in all directions. Does not have any collision
     * detection. 3x3 sampling of the given texture is used here.
     * 
     * @param list 
     * @param position 
     * @param texID 
     */
    void spawnBlockBreakParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID);

    /**
     * @brief Spawns particles that bursts in all direction but has very little vertical velocity. Particles also bound
     * when they hit their minimum y value. 3x3 sampling of the given texture is used here.
     * @param list 
     * @param position 
     * @param texID 
     * @param totalParticles 
     */
    void spawnImpactParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, int totalParticles = 18);

    /**
     * @brief Spawns many particles that are not affected by gravity. 3x3 sampling of the given texture is not used here.
     * 
     * @param list 
     * @param position 
     * @param texID 
     * @param maxHeight 
     * @param totalParticles 
     */
    void spawnFloatingParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float maxHeight = 999.0f, int totalParticles = -1);

    /**
     * @brief Spawns one or two particles that are not affect by gravity. 3x3 sampling of the given texture is not used here.
     * 
     * @param list 
     * @param position 
     * @param texID 
     * @param maxHeight 
     */
    void spawnSlowFloatingParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float maxHeight = 999.0f);

    /**
     * @brief Spawns particles that stays in the same position for a little while before dropping down onto the minimum y value.
     * 3x3 sampling of the given texture is used here.
     * 
     * @param list 
     * @param position 
     * @param texID 
     * @param minHeight 
     */
    void spawnDripParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float minHeight = -999.0f);

    /**
     * @brief Spawns particles on the block surface that does not move at all. 3x3 sampling of the given texture is not used here.
     * 
     * @param list 
     * @param position 
     * @param texID 
     * @param color 
     */
    void spawnBlockAmbientParticle(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, glm::vec3 color);

    /**
     * @brief Spawns particles in a wide range around the given position. 3x3 sampling of the given texture is not used here.
     * 
     * @param list 
     * @param position 
     * @param texID 
     * @param maxHeight 
     */
    void spawnParticleAround(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float maxHeight = 999.0f);

    /**
     * @brief Spawns particles in a wide range above the given position. 3x3 sampling of the given texture is not used here.
     * 
     * @param list 
     * @param position 
     * @param texID 
     * @param minHeight 
     */
    void spawnAmbientParticle(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float minHeight = -999.0f);

    /**
     * @brief All particles in the given list will be drawn with the given render. The gravity given here determines how fast the particles
     * will fall. Set animate to true to also animate the particle, false if otherwise. Particles out of time will be freed and removed
     * from the list
     * 
     * @param list 
     * @param defaultRender 
     * @param view 
     * @param proj 
     * @param gravity 
     * @param dt 
     * @param animate 
     */
    void drawAllParticles(std::vector<particle_t *> *list, renderer::renderer_t defaultRender, glm::mat4 view, glm::mat4 proj, GLfloat gravity, GLfloat dt, bool animate = true);

    /**
     * @brief Animates the praticles and also subtracts the deltatime from the particle's lifespan
     * 
     * @param particlePointer 
     * @param gravity 
     * @param dt 
     */
    void animateParticle(particle_t* particlePointer, GLfloat gravity, GLfloat dt);

    /**
     * @brief All particles in the given particle list will be freed and removed
     * 
     * @param list 
     */
    void destroyAllParticles(std::vector<particle_t *> *list);
}

#endif //COMP3421_ASS3_PARTICLE_HPP
    