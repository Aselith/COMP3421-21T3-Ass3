#include <chicken3421/chicken3421.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <ass3/particle.hpp>
#include <ass3/shapes.hpp>
#include <ass3/utility.hpp>
#include <iostream>

namespace particle {

	const float RADIUS = 0.5f;

	void spawnBlockBreakParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID) {
		int totalParticles = (rand() % 10) + 8;
		for (int i = 0; i < totalParticles; i++) {
			particle_t* newParticle = (struct particle_t*) malloc(sizeof(struct particle_t));

			newParticle->mesh = shapes::createParticle(true);
			newParticle->color = glm::vec3(-1.0f, -1.0f, -1.0f);
			newParticle->textureID = texID;
			newParticle->affectedByGravity = true;
			newParticle->affectedByLight = true;
			newParticle->lifeTimer = utility::genRandFloat(0.5f, 1.5f);
			newParticle->translation = position;
			newParticle->minimumYvalue = -999.0f;
			newParticle->maximumYvalue = 999.0f;
			newParticle->bounceCount = 0;

			newParticle->yVelocity = utility::genRandFloat(3.0f, 6.5f);
			newParticle->xVelocity = utility::genRandFloat(-1.5f, 1.5f);
			newParticle->zVelocity = utility::genRandFloat(-1.5f, 1.5f);

			// Assigning a random position
			newParticle->translation.x += utility::genRandFloat(-0.3f, 0.3f);
			newParticle->translation.y += utility::genRandFloat(-0.3f, 0.3f);
			newParticle->translation.z += utility::genRandFloat(-0.3f, 0.3f);

			newParticle->scale = glm::vec3(0.1875f, 0.1875f, 0.1875f);
			
			list->push_back(newParticle);
		}

		return;
	}

	void spawnFloatingParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float maxHeight, int totalParticles) {
		if (totalParticles < 0) {
			totalParticles = (rand() % 10) + 5;
		}
		for (int i = 0; i < totalParticles; i++) {
			particle_t* newParticle = (struct particle_t*) malloc(sizeof(struct particle_t));

			newParticle->mesh = shapes::createParticle(false);
			newParticle->color = glm::vec3(-1.0f, -1.0f, -1.0f);
			newParticle->textureID = texID;
			newParticle->affectedByGravity = false;
			newParticle->affectedByLight = true;
			newParticle->lifeTimer = utility::genRandFloat(5.0f, 12.0f);
			newParticle->translation = position;
			newParticle->minimumYvalue = -999.0f;
			newParticle->maximumYvalue = maxHeight;
			newParticle->bounceCount = 0;

			newParticle->yVelocity = utility::genRandFloat(0.2f, 1.0f);
			newParticle->xVelocity = utility::genRandFloat(-0.5f, 0.5f);
			newParticle->zVelocity = utility::genRandFloat(-0.5f, 0.5f);

			// Assigning a random position
			newParticle->translation.x += utility::genRandFloat(-0.4f, 0.4f);
			newParticle->translation.y += utility::genRandFloat(-0.4f, 0.4f);
			newParticle->translation.z += utility::genRandFloat(-0.4f, 0.4f);

			newParticle->scale = glm::vec3(0.1875f, 0.1875f, 0.1875f);
			
			list->push_back(newParticle);
		}

		return;
	}

	void spawnSlowFloatingParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float maxHeight) {

		particle_t* newParticle = (struct particle_t*) malloc(sizeof(struct particle_t));

		newParticle->mesh = shapes::createParticle(false);
		newParticle->color = glm::vec3(-1.0f, -1.0f, -1.0f);
		newParticle->textureID = texID;
		newParticle->affectedByGravity = false;
		newParticle->affectedByLight = true;
		newParticle->lifeTimer = utility::genRandFloat(20.0f, 30.0f);
		newParticle->translation = position;
		newParticle->minimumYvalue = -999.0f;
		newParticle->maximumYvalue = maxHeight;
		newParticle->bounceCount = 0;

		newParticle->yVelocity = utility::genRandFloat(0.2f, 1.0f);
		newParticle->xVelocity = 0.0f;
		newParticle->zVelocity = 0.0f;

		// Assigning a random position
		newParticle->translation.x += utility::genRandFloat(-0.4f, 0.4f);
		newParticle->translation.y += utility::genRandFloat(-0.4f, 0.4f);
		newParticle->translation.z += utility::genRandFloat(-0.4f, 0.4f);

		newParticle->scale = glm::vec3(1.0f, 1.0f, 1.0f);
		
		list->push_back(newParticle);

		return;
	}

	void spawnDripParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float minHeight) {
		// Error checking. Don't spawn any particles if texture is invalid
		if (texID == 0) return;

		int totalParticles = (rand() % 2) + 1;
		for (int i = 0; i < totalParticles; i++) {
			particle_t* newParticle = (struct particle_t*) malloc(sizeof(struct particle_t));

			newParticle->mesh = shapes::createParticle(true);
			newParticle->color = glm::vec3(-1.0f, -1.0f, -1.0f);
			newParticle->textureID = texID;
			newParticle->affectedByGravity = true;
			newParticle->affectedByLight = true;
			newParticle->lifeTimer = utility::genRandFloat(3.5f, 25.0f);
			newParticle->translation.x = position.x;
			newParticle->translation.y = position.y;
			newParticle->translation.z = position.z;
			newParticle->translation += utility::genRandomPointOnCube(0.505f);
			newParticle->minimumYvalue = minHeight;
			newParticle->maximumYvalue = newParticle->translation.y;
			newParticle->bounceCount = 0;

			newParticle->yVelocity = utility::genRandFloat(0.0f, 100.0f);
			newParticle->xVelocity = 0.0f;
			newParticle->zVelocity = 0.0f;

			newParticle->scale = glm::vec3(0.09375f, 0.09375f, 0.09375f);
			
			list->push_back(newParticle);
		}

		return;
	}

	void spawnBlockAmbientParticle(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, glm::vec3 color) {
		// Error checking. Don't spawn any particles if texture is invalid
		if (texID == 0) return;

		int totalParticles = (rand() % 3) + 1;
		for (int i = 0; i < totalParticles; i++) {
			particle_t* newParticle = (struct particle_t*) malloc(sizeof(struct particle_t));

			newParticle->mesh = shapes::createParticle(false);
			newParticle->color = color;
			newParticle->textureID = texID;
			newParticle->affectedByGravity = false;
			newParticle->affectedByLight = false;
			newParticle->lifeTimer = utility::genRandFloat(3.5f, 25.0f);
			newParticle->translation.x = position.x;
			newParticle->translation.y = position.y;
			newParticle->translation.z = position.z;
			newParticle->translation += utility::genRandomPointOnCube(0.515f);
			newParticle->minimumYvalue = -999.0f;
			newParticle->maximumYvalue = 999.0f;
			newParticle->bounceCount = 0;

			newParticle->yVelocity = 0.0f;
			newParticle->xVelocity = 0.0f;
			newParticle->zVelocity = 0.0f;

			newParticle->scale = glm::vec3(0.1875f, 0.1875f, 0.1875f);
			
			list->push_back(newParticle);
		}

		return;
	}

	void spawnParticleAround(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float maxHeight) {
		// Error checking. Don't spawn any particles if texture is invalid
		if (texID == 0) return;

		particle_t* newParticle = (struct particle_t*) malloc(sizeof(struct particle_t));

		newParticle->mesh = shapes::createParticle(false);
		newParticle->color = glm::vec3(-1.0f, -1.0f, -1.0f);
		newParticle->textureID = texID;
		newParticle->affectedByGravity = false;
		newParticle->affectedByLight = true;
		newParticle->lifeTimer = utility::genRandFloat(1.5f, 25.0f);
		newParticle->translation.x = position.x + utility::genRandFloat(-3.0f, 3.0f);
		newParticle->translation.y = position.y + utility::genRandFloat(-3.0f, 3.0f);
		newParticle->translation.z = position.z + utility::genRandFloat(-3.0f, 3.0f);

		newParticle->minimumYvalue = 0;
		newParticle->maximumYvalue = maxHeight;
		newParticle->bounceCount = 0;

		newParticle->yVelocity = utility::genRandFloat(-0.01f, 0.01f);
		newParticle->xVelocity = utility::genRandFloat(-0.2f, 0.2f);
		newParticle->zVelocity = utility::genRandFloat(-0.2f, 0.2f);

		newParticle->scale = glm::vec3(1.0f, 1.0f, 1.0f);
		
		list->push_back(newParticle);

		return;
	}

	void spawnAmbientParticle(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, float minHeight) {
		// Error checking. Don't spawn any particles if texture is invalid
		if (texID == 0) return;

		int totalParticles = (rand() % 2) + 1;
		for (int i = 0; i < totalParticles; i++) {
			particle_t* newParticle = (struct particle_t*) malloc(sizeof(struct particle_t));

			newParticle->mesh = shapes::createParticle(false);
			newParticle->color = glm::vec3(-1.0f, -1.0f, -1.0f);
			newParticle->textureID = texID;
			newParticle->affectedByGravity = false;
			newParticle->affectedByLight = false;
			newParticle->lifeTimer = utility::genRandFloat(10.0f, 120.0f);
			newParticle->translation.x = position.x + utility::genRandFloat(-3.0f, 3.0f);
			newParticle->translation.y = position.y + utility::genRandFloat( 0.0f, 5.0f);
			newParticle->translation.z = position.z + utility::genRandFloat(-3.0f, 3.0f);

			newParticle->minimumYvalue = minHeight;
			newParticle->maximumYvalue = 9999;
			newParticle->bounceCount = 0;

			newParticle->yVelocity = utility::genRandFloat(-0.02f, 0.02f);
			newParticle->xVelocity = utility::genRandFloat(-0.2f, 0.2f);
			newParticle->zVelocity = utility::genRandFloat(-0.2f, 0.2f);

			newParticle->scale = glm::vec3(0.5f, 0.5f, 0.5f);
			
			list->push_back(newParticle);
		}

		return;
	}


	void spawnImpactParticles(std::vector<particle_t *> *list, glm::vec3 position, GLuint texID, int totalParticles) {
		// Error checking. Don't spawn any particles if texture is invalid
		if (texID == 0) return;
		GLfloat sign = 0;
		for (int i = 0; i < totalParticles; i++) {
			particle_t* newParticle = (struct particle_t*) malloc(sizeof(struct particle_t));

			newParticle->mesh = shapes::createParticle(true);
			newParticle->color = glm::vec3(-1.0f, -1.0f, -1.0f);
			newParticle->textureID = texID;
			newParticle->affectedByGravity = true;
			newParticle->affectedByLight = true;

			newParticle->lifeTimer = utility::genRandFloat(3.5f, 25.0f);
			newParticle->translation.x = position.x;
			newParticle->translation.y = position.y;
			newParticle->translation.z = position.z;
			newParticle->minimumYvalue = position.y;
			newParticle->maximumYvalue = 999.0f;
			newParticle->bounceCount = 3;
			newParticle->yVelocity = utility::genRandFloat(4.0f, 5.5f);

			sign = (rand() % 2 == 0) ? -1.0f : 1.0f;
			auto xPoint = utility::genRandFloat(-0.5f, 0.5f);
			auto zPoint = sign * glm::sqrt(abs((xPoint * xPoint) - (RADIUS * RADIUS)));
	
			newParticle->xVelocity = xPoint * utility::genRandFloat(1.0f, 3.0f);
			newParticle->zVelocity = zPoint * utility::genRandFloat(1.0f, 3.0f);

			newParticle->scale = glm::vec3(0.1875f, 0.1875f, 0.1875f);
			
			list->push_back(newParticle);
		}

		return;
	}

    void drawAllParticles(std::vector<particle_t *> *list, renderer::renderer_t particleRender, glm::mat4 view, glm::mat4 proj, GLfloat gravity, GLfloat dt, bool animate) {
		size_t index = 0;
		while (index < list->size()) {

			particle_t* particlePointer = list->at(index);
	
			auto model = glm::mat4(1.0f);
			model *= glm::translate(glm::mat4(1.0), particlePointer->translation);

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
			particleRender.setVec3("uColor", particlePointer->color);
			particleRender.setInt("affectedByLight", particlePointer->affectedByLight);

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
				if (animate) animateParticle(particlePointer, gravity, dt);
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
		particlePointer->translation.y = glm::clamp(particlePointer->translation.y, particlePointer->minimumYvalue, particlePointer->maximumYvalue);
		if (particlePointer->translation.y == particlePointer->minimumYvalue || particlePointer->translation.y == particlePointer->maximumYvalue) {
			if (particlePointer->bounceCount > 0) {
				particlePointer->yVelocity = -particlePointer->yVelocity * 0.4f;
				particlePointer->xVelocity /= 2.0f;
				particlePointer->zVelocity /= 2.0f;
				particlePointer->bounceCount--;
			} else {
				// particlePointer->yVelocity = 0.0f;
				particlePointer->xVelocity = 0.0f;
				particlePointer->zVelocity = 0.0f;
			}
			
		}
		particlePointer->lifeTimer -= dt;

	}

	void destroyAllParticles(std::vector<particle_t *> *list) {
		size_t index = 0;
		while (index < list->size()) {
			static_mesh::destroy(list->at(index)->mesh);
			free(list->at(index));
			list->erase(list->begin() + (int)index);
		}
	}

}