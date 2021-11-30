#include <chicken3421/chicken3421.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <ass3/particle.hpp>
#include <ass3/shapes.hpp>
#include <ass3/utility.hpp>
#include <iostream>

namespace particle {

	const int TOTAL_BREAK_PARTICLES = 16;

	void spawnBlockBreakParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID) {
		GLfloat sign = 0;
		for (int i = 0; i < TOTAL_BREAK_PARTICLES; i++) {
			particle_t* newParticle = (struct particle_t*) malloc(sizeof(struct particle_t));

			newParticle->mesh = shapes::createParticle(true);
			newParticle->textureID = texID;
			newParticle->affectedByGravity = true;
			newParticle->lifeTimer = 5.0f; // Only lasts for 5 seconds
			newParticle->translation = position;

			sign = (rand() % 2 == 0) ? -1.0f : 1.0f;
			newParticle->yVelocity = sign * utility::genRandFloat(4.0f, 6.5f);
			sign = (rand() % 2 == 0) ? -1.0f : 1.0f;
			newParticle->xVelocity = sign * utility::genRandFloat(0.5f, 1.5f);
			sign = (rand() % 2 == 0) ? -1.0f : 1.0f;
			newParticle->zVelocity = sign * utility::genRandFloat(0.5f, 1.5f);

			// Assigning a random position
			sign = (rand() % 2 == 0) ? -1.0f : 1.0f;
			newParticle->translation.x += sign * utility::genRandFloat(0.0f, 0.3f);
			sign = (rand() % 2 == 0) ? -1.0f : 1.0f;
			newParticle->translation.y += sign * utility::genRandFloat(0.0f, 0.3f);
			sign = (rand() % 2 == 0) ? -1.0f : 1.0f;
			newParticle->translation.z += sign * utility::genRandFloat(0.0f, 0.3f);

			newParticle->rotation = glm::vec3(0.0f);
			newParticle->scale = glm::vec3(0.1875f, 0.1875f, 0.1875f);
			
			list->push_back(newParticle);
		}

		return;
	}

    void drawAllParticles(std::vector<particle_t *> *list, renderer::renderer_t particleRender, glm::mat4 view, glm::mat4 proj, GLfloat gravity, GLfloat dt, bool animate) {
		size_t index = 0;
		while (index < list->size()) {
			particle_t* particlePointer = list->at(index);

			if (animate) animateParticle(particlePointer, gravity, dt);
	
			auto model = glm::mat4(1.0f);
			model *= glm::translate(glm::mat4(1.0), particlePointer->translation);
			model *= glm::rotate(glm::mat4(1.0), glm::radians(particlePointer->rotation.z), glm::vec3(0, 0, 1));
			model *= glm::rotate(glm::mat4(1.0), glm::radians(particlePointer->rotation.y), glm::vec3(0, 1, 0));
			model *= glm::rotate(glm::mat4(1.0), glm::radians(particlePointer->rotation.x), glm::vec3(1, 0, 0));

			particleRender.setMat4("uProj", proj);
			auto viewModel = view * model;
			// Removing the rotational component from the view model by setting 3x3 top left into identity
			// Billboarding
			viewModel[0][0] = 1.0f;
			viewModel[1][1] = 1.0f;
			viewModel[2][2] = 1.0f;
			viewModel[0][1] = 0.0f;
			viewModel[0][2] = 0.0f;
			viewModel[1][2] = 0.0f;
			viewModel[1][0] = 0.0f;
			viewModel[2][0] = 0.0f;
			viewModel[2][1] = 0.0f;

			viewModel *= glm::scale(glm::mat4(1.0), particlePointer->scale);

			particleRender.setMat4("uViewModel", viewModel);
			particleRender.setMat4("uModel", model);

			if (particlePointer->mesh.vbo) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, particlePointer->textureID);

				glBindVertexArray(particlePointer->mesh.vao);
				glDrawElements(GL_TRIANGLES, particlePointer->mesh.indices_count, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);
			}

			if (particlePointer->lifeTimer <= 0) {
				// Destory particle when the timer reaches below 0
				static_mesh::destroy(list->at(index)->mesh);
				free(list->at(index));
				list->erase(list->begin() + (int)index);
			} else {
				index++;
			}
		}
	}

	void animateParticle(particle_t* particlePointer, GLfloat gravity, GLfloat dt) {
		particlePointer->translation.y += particlePointer->yVelocity * dt * 0.5f;
		particlePointer->translation.x += particlePointer->xVelocity * dt * 0.5f;
		particlePointer->translation.z += particlePointer->zVelocity * dt * 0.5f;
		if (particlePointer->affectedByGravity) {
			particlePointer->yVelocity += gravity * dt * 0.5f;
		}
		particlePointer->lifeTimer -= dt;

	}

}