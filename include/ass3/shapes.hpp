#ifndef COMP3421_ASS3_SHAPES_HPP
#define COMP3421_ASS3_SHAPES_HPP

#include <glm/glm.hpp>
#include <ass3/static_mesh.hpp>

namespace shapes {

    /**
     * @brief Create a static mesh of a cube
     * 
     * @param invertNormals 
     * @param affectedByLight 
     * @return static_mesh::mesh_t 
     */
    static_mesh::mesh_t createCube(bool invertNormals, bool affectedByLight);

    /**
     * @brief Create a static mesh of a one dimensional square
     * 
     * @param invert 
     * @return static_mesh::mesh_t 
     */
    static_mesh::mesh_t createFlatSquare(bool invert);

    static_mesh::mesh_t createParticle(bool sampleTexture);

    static_mesh::mesh_t createSkybox();

    static_mesh::mesh_t createFlatSurface(int width);

    /**
     * @brief Create a mesh for a Minecraft bed
     * 
     * @return static_mesh::mesh_t 
     */
    static_mesh::mesh_t createBed();

    /**
     * @brief Create a mesh for a Minecraft player head
     * 
     * @return static_mesh::mesh_t 
     */
    static_mesh::mesh_t createPlayerHead();

    /**
     * @brief Create a mesh for a Minecraft player torso
     * 
     * @return static_mesh::mesh_t 
     */
    static_mesh::mesh_t createPlayerTorso();

    /**
     * @brief Create a mesh for a Minecraft player right leg
     * 
     * @return static_mesh::mesh_t 
     */
    static_mesh::mesh_t createPlayerLegRight();

    /**
     * @brief Create a mesh for a Minecraft player left leg
     * 
     * @return static_mesh::mesh_t 
     */
    static_mesh::mesh_t createPlayerLegLeft();

    /**
     * @brief Create a mesh for a Minecraft player right arm
     * 
     * @return static_mesh::mesh_t 
     */
    static_mesh::mesh_t createPlayerArmRight();

    /**
     * @brief Create a mesh for a Minecraft player left arm
     * 
     * @return static_mesh::mesh_t 
     */
    static_mesh::mesh_t createPlayerArmLeft();

}

#endif //COMP3421_SHAPES_HPP
