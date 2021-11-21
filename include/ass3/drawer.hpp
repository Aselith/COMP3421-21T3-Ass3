
#ifndef COMP3421_ASS3_DRAWER_HPP
#define COMP3421_ASS3_DRAWER_HPP

#include <ass3/scene.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace drawer {

	/**
     * @brief Render the given node and its children recursively. Renders selected sides based on
     * the node's culled faces
     * 
     * @param node 
     * @param model 
     * @param renderInfo 
     * @param defaultSpecular 
     */
    void drawBlock(const scene::node_t *node, glm::mat4 model, renderer::renderer_t renderInfo, GLuint defaultSpecular);

    /**
     * @brief Render the given node and its children recursively. Renders all sides regardless
     * of culled faces
     * 
     * @param node 
     * @param model 
     * @param renderInfo 
     * @param defaultSpecular 
     */
    void drawElement(const scene::node_t *node, glm::mat4 model, renderer::renderer_t renderInfo, GLuint defaultSpecular);
}

#endif //COMP3421_ASS3_FRUSTUM_HPP