#ifndef COMP3421_ASS3_RENDERER_HPP
#define COMP3421_ASS3_RENDERER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <chicken3421/chicken3421.hpp>

#include <iostream>
#include <string>

#define MAX_LIGHTS 101

namespace renderer {

	struct lightSource {
		glm::vec3 position;
		glm::vec3 diffuse;
		glm::vec3 ambient;
		glm::vec3 specular = glm::vec3(1.1f);
		float intensity = 1.0f;

		GLint position_loc;
		GLint diffuse_loc;
		GLint ambient_loc;
		GLint specular_loc;
		GLint intensity_loc;

		bool occupied = false;
	};

	struct renderer_t {

		std::string type = "";

		glm::mat4 projection;
		glm::vec3 sun_light_dir = glm::normalize(glm::vec3(0) - glm::vec3(0, 10, 0));
		glm::vec3 sun_light_color = glm::vec3(1.1, 1.1, 1.1);
		float sun_light_ambient = 0.5f;

		glm::vec3 sunriseColor = glm::vec3((float)247/255, (float)205/255, (float)93/255);
		glm::vec3 dayColor = glm::vec3(1, 1, 1);
		glm::vec3 sunsetColor = glm::vec3((float)250/255, (float)214/255, (float)165/255);
		glm::vec3 nightColor = glm::vec3((float)69/255, (float)69/255, (float)133/255);

		GLuint program;
		
		bool isShadowProgram = false;
		// vertex shader uniforms
		GLint view_proj_loc;
		GLint model_loc;
		GLint light_proj_loc;

		GLint sun_direction_loc;
		GLint sun_color_loc;
		GLint sun_ambient_loc;

		GLint uTex_loc;
		GLint uSpec_loc;
		GLint uDepth_loc;
		GLuint uCube_loc;

		GLint mat_tex_factor_loc;
		GLint mat_specular_factor_loc;
		GLint mat_specular_loc;
		GLint mat_color_loc;
		GLint mat_diffuse_loc;
		GLint mat_ambient_loc;
		GLint phong_exponent_loc;
		GLint camera_loc;
		std::vector<lightSource> allLightSources;
		int totalPointLights = 0;


		/**
		 * @brief Creates the program based on name given. Will override the program currently stored
		 * 
		 */
		void createProgram(std::string programName) {
			type = programName;
			chicken3421::delete_program(program);
			std::string directory = "res/shaders/" + programName;
			auto vs = chicken3421::make_shader(directory + ".vert", GL_VERTEX_SHADER);
			auto fs = chicken3421::make_shader(directory + ".frag", GL_FRAGMENT_SHADER);
			program = chicken3421::make_program(vs, fs);
			chicken3421::delete_shader(vs);
			chicken3421::delete_shader(fs);
		}

		void setUpShadow() {
			light_proj_loc = chicken3421::get_uniform_location(program, "uLightProj");
			model_loc = chicken3421::get_uniform_location(program, "uModel");
		}

		/**
		 * @brief Creates the program with the given width and height and initialises all the different shader locations
		 * This creates basic program only
		 * @param width 
		 * @param height 
		 */
		void initialise(int width, int height) {

			// Gets MVP_Loc
			view_proj_loc = chicken3421::get_uniform_location(program, "uViewProj");
			light_proj_loc = chicken3421::get_uniform_location(program, "uLightProj");
			model_loc = chicken3421::get_uniform_location(program, "uModel");

			uTex_loc = chicken3421::get_uniform_location(program, "uTex");
			uSpec_loc = chicken3421::get_uniform_location(program, "uSpec");
			uDepth_loc = chicken3421::get_uniform_location(program, "uDepthMap");

			// Get projection
			projection = glm::perspective(glm::radians(60.0), (double) width / (double) height, 0.1, 200.0);
			// sunlight uniform locations
			sun_direction_loc = chicken3421::get_uniform_location(program, "uSun.direction");
			sun_color_loc = chicken3421::get_uniform_location(program, "uSun.color");
			sun_ambient_loc = chicken3421::get_uniform_location(program, "uSun.ambient");

			// material uniform locations
			mat_ambient_loc = chicken3421::get_uniform_location(program, "uMat.ambient");

			mat_tex_factor_loc = chicken3421::get_uniform_location(program, "uMat.texFactor");
			mat_color_loc = chicken3421::get_uniform_location(program, "uMat.color");
			mat_diffuse_loc = chicken3421::get_uniform_location(program, "uMat.diffuse");
			camera_loc =  chicken3421::get_uniform_location(program, "uCameraPos");
			phong_exponent_loc = chicken3421::get_uniform_location(program, "uMat.phongExp");
			mat_specular_factor_loc = chicken3421::get_uniform_location(program, "uMat.specularFactor");
			mat_specular_loc = chicken3421::get_uniform_location(program, "uMat.specular");

			for (int i = 0; i < MAX_LIGHTS; i++) {
				lightSource light;
				light.ambient_loc = chicken3421::get_uniform_location(program, "allLights[" + std::to_string(i) + "].ambient");
				light.diffuse_loc = chicken3421::get_uniform_location(program, "allLights[" + std::to_string(i) + "].diffuse");
				light.position_loc = chicken3421::get_uniform_location(program, "allLights[" + std::to_string(i) + "].position");
				light.specular_loc = chicken3421::get_uniform_location(program, "allLights[" + std::to_string(i) + "].specular");
				light.intensity_loc = chicken3421::get_uniform_location(program, "allLights[" + std::to_string(i) + "].intensity");
				light.position = {-1, -1, -1};
				allLightSources.push_back(light);
			}
		}

		/**
		 * @brief Uses this shader
		 * 
		 */
		void activate() {
			glUseProgram(program);
		}

		void setInt(const std::string &name, int value) const {
			glUniform1i(chicken3421::get_uniform_location(program, name.c_str()), value); 
		}

		void setFloat(const std::string &name, float value) const { 
			glUniform1fv(chicken3421::get_uniform_location(program, name.c_str()), 1, &value); 
		}

		void setMat4(const std::string &name, const glm::mat4 &mat) const {
			glUniformMatrix4fv(chicken3421::get_uniform_location(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}

		void setVec2(const std::string &name, const glm::vec2 &value) const { 
			glUniform2fv(chicken3421::get_uniform_location(program, name.c_str()), 1, &value[0]); 
		}

		void setVec3(const std::string &name, const glm::vec3 &value) const { 
			glUniform3fv(chicken3421::get_uniform_location(program, name.c_str()), 1, &value[0]); 
		}

		void setVec4(const std::string &name, const glm::vec4 &value) const { 
			glUniform4fv(chicken3421::get_uniform_location(program, name.c_str()), 1, &value[0]);
		}

		/**
		 * @brief Changes the sunlight color based on the given degree.
		 * 
		 * @param degree 
		 */
		void changeSunlight(float degree) {

			if (degree >= 0.0f && degree < 90.0f) {
				sun_light_color = sunriseColor + ((dayColor - sunriseColor) * glm::vec3(degree / 90.0f)); 
			} else if (degree >= 90.0f && degree < 180.0f) {
				sun_light_color = dayColor + ((sunsetColor - dayColor) * glm::vec3((degree - 90) / 90.0f)); 
			} else if (degree >= 180.0f && degree < 270.0f) {
				sun_light_color = sunsetColor + ((nightColor - sunsetColor) * glm::vec3((degree - 180) / 90.0f)); 
			} else {
				sun_light_color = nightColor + ((sunriseColor - nightColor) * glm::vec3((degree - 270) / 90.0f)); 
			}
			if (degree >= 180.0f && degree < 360.0f) {
				sun_light_ambient = -glm::sin(glm::radians(degree)) * 0.4f + 0.25f;
			} else {
				sun_light_ambient = 0.25f;
			}
		}

		/**
		 * @brief Gets the max lights allowed by this program
		 * @warning must match with the MAX_LIGHTS in the shader frag
		 * 
		 * @return int 
		 */
		int getMaxLights() {
			return MAX_LIGHTS;
		}

		/**
		 * @brief Sets the base shader pointers
		 * 
		 * @param pos 
		 */
		void setBasePters(glm::vec3 pos) {
			glUniform3fv(sun_direction_loc, 1, glm::value_ptr(sun_light_dir));
            glUniform3fv(sun_color_loc, 1, glm::value_ptr(sun_light_color));
            glUniform3fv(camera_loc, 1, glm::value_ptr(pos));
            glUniform1f(sun_ambient_loc, sun_light_ambient);
			for (auto light : allLightSources) {
				glUniform3fv(light.ambient_loc, 1, glm::value_ptr(light.ambient));
				glUniform3fv(light.diffuse_loc, 1, glm::value_ptr(light.diffuse));
				glUniform3fv(light.specular_loc, 1, glm::value_ptr(light.specular));
				glUniform3fv(light.position_loc, 1, glm::value_ptr(light.position));
				glUniform1f(light.intensity_loc, light.intensity);
			}
            glUniform1i(uTex_loc, 0);
			glUniform1i(uSpec_loc, 1);
			glUniform1i(uDepth_loc, 2);
			// Pointing to the different textures
		}

		/**
		 * @brief Add a light source with the given parameters and returns the light ID used to create this light
		 * 
		 * @param pos 
		 * @param color 
		 * @param intensity 
		 * @return int 
		 */
		int addLightSource(glm::vec3 pos, glm::vec3 color, float intensity) {
			for (size_t i = 0; i < allLightSources.size(); i++) {
				if (!allLightSources[i].occupied) {
					totalPointLights++;
					std::cout << "\u001b[32mLight added.   " << MAX_LIGHTS - totalPointLights << " lights left.\n\033[0m";
					allLightSources[i].occupied = true;
					allLightSources[i].position = pos;
					allLightSources[i].diffuse = color;
					allLightSources[i].ambient = color;
					allLightSources[i].intensity = intensity;
					allLightSources[i].specular = color * 1.5f;
					if (allLightSources[i].specular.r >= 1.0f) {
						allLightSources[i].specular.r = 1.0f;
					}
					if (allLightSources[i].specular.g >= 1.0f) {
						allLightSources[i].specular.g = 1.0f;
					}
					if (allLightSources[i].specular.b >= 1.0f) {
						allLightSources[i].specular.b = 1.0f;
					}
					return (int)i;
				}
			}
			return -1;
		}

		/**
		 * @brief Remove the light source with the given ID
		 * 
		 * @param lightID 
		 */
		void removeLightSource(int lightID) {
			if (lightID < 0) return;
			totalPointLights--;
			std::cout << "\u001b[31mLight removed. " << MAX_LIGHTS - totalPointLights << " lights left.\033[0m\n";
			allLightSources[(size_t)lightID].occupied = false;
			allLightSources[(size_t)lightID].position = {-1, -1, 1};
			return;
		}

		/**
		 * @brief Deletes the program that was originally initialised
		 * 
		 */
		void deleteProgram() {
			chicken3421::delete_program(program);
		}
	};
}

#endif // COMP3421_ASS3_RENDERER_HPP