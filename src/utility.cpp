#include <glad/glad.h>

#include <iostream>
#include <chicken3421/chicken3421.hpp>

#include <ass3/utility.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include <algorithm>

GLuint quadrangleVAO = 0;
GLuint quadrangleVBO;

namespace utility {

    float calculateDistance(glm::vec3 posA, glm::vec3 posB) {
        return sqrt( pow(posB.x - posA.x, 2) + pow(posB.y - posA.y, 2) + pow(posB.z - posA.z, 2) );
    }

    float time_delta() {
        static float then = time_now();
        float now = time_now();
        float dt = now - then;
        then = now;
        return dt;
    }

	GLfloat findIlluminance(int screenWidth, int screenHeight) {
		glm::vec3 luminescene;
        glm::vec3 totalLuminescene;
        GLfloat totalSamples = 0;
        for (GLuint x = 0; x < screenWidth; x += 240) {
            for (GLuint y = 0; y < screenHeight; y += 240) {
                totalSamples += 1.5f;
                glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &luminescene);
                totalLuminescene += luminescene;
            }
        }
        totalLuminescene /= totalSamples;
		return 0.2126 * totalLuminescene.r + 0.7152 * totalLuminescene.g + 0.0722 * totalLuminescene.b;
	}

	GLfloat lerp(GLfloat posA, GLfloat posB, GLfloat by) {
		return posA * (1 - by) + posB * by;
	}

	double roundUp(double value, int decimal_places) {
		const double multiplier = std::pow(10.0, decimal_places);
		return std::ceil(value * multiplier) / multiplier;
	}

	void renderQuad() {
		if (quadrangleVAO == 0) {
			float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays(1, &quadrangleVAO);
			glGenBuffers(1, &quadrangleVBO);
			glBindVertexArray(quadrangleVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadrangleVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		glBindVertexArray(quadrangleVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}


    float time_now() {
        return (float)glfwGetTime();
    }

    void calcVertNormals(static_mesh::mesh_template_t& mesh_template) {
		mesh_template.normals = std::vector<glm::vec3>(mesh_template.positions.size(), glm::vec3(0));

		const auto& pos = mesh_template.positions;
		for (auto i = size_t{0}; i < mesh_template.indices.size() - 2; i += 3) {
			GLuint i1 = mesh_template.indices[i];
			GLuint i2 = mesh_template.indices[i + 1];
			GLuint i3 = mesh_template.indices[i + 2];
			auto a = pos[i2] - pos[i1];
			auto b = pos[i3] - pos[i1];
			auto face_normal = glm::normalize(glm::cross(a, b));
			mesh_template.normals[i1] += face_normal;
			mesh_template.normals[i2] += face_normal;
			mesh_template.normals[i3] += face_normal;
			/*
			std::cout << "Normals: " <<  face_normal.x << " " << face_normal.y << " " << face_normal.z << "\n";
			std::cout <<  mesh_template.normals[i1].x << " " << mesh_template.normals[i1].y << " " << mesh_template.normals[i1].z << "\n";
			std::cout <<  mesh_template.normals[i2].x << " " << mesh_template.normals[i2].y << " " << mesh_template.normals[i2].z << "\n";
			std::cout <<  mesh_template.normals[i3].x << " " << mesh_template.normals[i3].y << " " << mesh_template.normals[i3].z << "\n";
			*/
		}
		// normalise all the normals
		// std::cout << "Normal size: " << mesh_template.normals.size() << "\n";
		for (auto i = size_t{0}; i < mesh_template.normals.size(); i++) {
			
			mesh_template.normals[i] = glm::normalize(mesh_template.normals[i]);
			// std::cout << mesh_template.normals[i].x << " " << mesh_template.normals[i].y << " " << mesh_template.normals[i].z << "\n";
		}
		// std::cout << "\n";
	}

	void invertShape(static_mesh::mesh_template_t &meshTemplate) {
		for (size_t i = 0; i < meshTemplate.indices.size(); i += 3) {
			int tempVariable = meshTemplate.indices[i + 1];
			meshTemplate.indices[i + 1] = meshTemplate.indices[i + 2];
			meshTemplate.indices[i + 2] = tempVariable;
		}
	}

	int countFalses(std::vector<bool> vector) {
		int total = 0;
		for (auto i : vector) {
			if (!i) {
				total++;
			}
		}
		return total;
	}

	glm::vec3 cubicBezier(const std::vector<glm::vec3> &controlPoints, float t) {
		glm::vec3 pos = (1 - t) * (1 - t) * (1 - t) * controlPoints[0]
						+ 3 * t * (1 - t) * (1 - t) * controlPoints[1]
						+ 3 * t * t * (1 - t) * controlPoints[2]
						+ t * t * t * controlPoints[3];
		return pos;
	}

	int getDirection(float yaw) {
		yaw = fmod(yaw, 360.0f);
		if (yaw >= 315.0f || yaw < 45.0f) {
			return 0;
		} else if (yaw >= 45.0f && yaw < 135.0f) {
			return 1;
		} else if (yaw >= 135.0f && yaw < 225.0f) {
			return 2;
		} else {
			return 3;
		}
	}

}
