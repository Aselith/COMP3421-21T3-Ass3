#include <glad/glad.h>
#include <iostream>
#include <stb/stb_image.h>
#include <chicken3421/chicken3421.hpp>

#include <ass3/texture_2d.hpp>

namespace texture_2d {
    GLuint init(std::string file_name, params_t const &params) {
        GLuint tex;
        glGenTextures(1, &tex);

        // load the image using stb lib
        void *data = 0;
        int width, height, n_channels;
        data = stbi_load(file_name.data(), &width, &height, &n_channels, 0);

        chicken3421::expect(data, "Could not read " + file_name);

        glBindTexture(GL_TEXTURE_2D, tex);

        GLenum format = n_channels == 3 ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        // generate mipmap if filter_min is a mipmap filter
        switch (params.filter_min) {
            case GL_LINEAR_MIPMAP_LINEAR:
            case GL_NEAREST_MIPMAP_LINEAR:
            case GL_LINEAR_MIPMAP_NEAREST:
            case GL_NEAREST_MIPMAP_NEAREST:
                glGenerateMipmap(GL_TEXTURE_2D);
                break;
            default:
                break;
        }

        // wrap options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.wrap_t);
        // mag/min options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.filter_min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.filter_max);

        glBindTexture(GL_TEXTURE_2D, 0);

        return tex;
    }

    GLuint loadCubemap(std::string filePath) {

        std::vector<std::string> faces;
        faces.push_back(filePath + "/right.jpg");
        faces.push_back(filePath + "/left.jpg");
        faces.push_back(filePath + "/top.jpg");
        faces.push_back(filePath + "/bottom.jpg");
        faces.push_back(filePath + "/front.jpg");
        faces.push_back(filePath + "/back.jpg");



        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (GLuint i = 0; i < faces.size(); i++) {
            void *data = 0;
            data = stbi_load(faces[i].data(), &width, &height, &nrChannels, 0);
            chicken3421::expect(data, "Could not read " + faces[i]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }

    void destroy(GLuint tex) {
        glDeleteTextures(1, &tex);
    }
}

