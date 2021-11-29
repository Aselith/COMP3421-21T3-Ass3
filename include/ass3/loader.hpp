
#ifndef COMP3421_ASS3_LOADEAR_HPP
#define COMP3421_ASS3_LOADEAR_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ass3/scene.hpp>

namespace loader {

	void printOutPresets();

	void loadWorld(std::vector<scene::miniBlockData> *list, int worldType);
}

#endif //COMP3421_ASS3_LOADEAR_HPP