#ifndef COMP3421_ASS3_PARTICLE_HPP
#define COMP3421_ASS3_PARTICLE_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ass3/static_mesh.hpp>
#include <ass3/static_mesh.hpp>
#include <ass3/renderer.hpp>

namespace particle {
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

    void spawnBlockBreakParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID);

    void spawnImpactParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, int totalParticles = 18);

    void spawnFloatingParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float maxHeight = 999.0f, int totalParticles = -1);

    void spawnSlowFloatingParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float maxHeight = 999.0f);

    void spawnDripParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float minHeight = -999.0f);

    void spawnBlockAmbientParticle(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, glm::vec3 color);

    void spawnParticleAround(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float maxHeight = 999.0f);

    void spawnAmbientParticle(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float minHeight = -999.0f);

    void drawAllParticles(std::vector<particle_t *> *list, renderer::renderer_t defaultRender, glm::mat4 view, glm::mat4 proj, GLfloat gravity, GLfloat dt, bool animate = true);

    void animateParticle(particle_t* particlePointer, GLfloat gravity, GLfloat dt);

    void destroyAllParticles(std::vector<particle_t *> *list);
}

#endif //COMP3421_ASS3_PARTICLE_HPP
    