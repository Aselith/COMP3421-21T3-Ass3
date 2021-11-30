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
        glm::vec3 translation = glm::vec3(0.0);
        glm::vec3 rotation = glm::vec3(0.0);
        glm::vec3 scale = glm::vec3(1.0);
        std::string name;
        GLfloat yVelocity, zVelocity, xVelocity;
        bool affectedByGravity = true;
        GLfloat lifeTimer = 0.0f;
    };

    void spawnBlockBreakParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID);

    void drawAllParticles(std::vector<particle_t *> *list, renderer::renderer_t defaultRender, glm::mat4 view, glm::mat4 proj, GLfloat gravity, GLfloat dt, bool animate = true);

    void animateParticle(particle_t* particlePointer, GLfloat gravity, GLfloat dt);
}

#endif //COMP3421_ASS3_PARTICLE_HPP
    