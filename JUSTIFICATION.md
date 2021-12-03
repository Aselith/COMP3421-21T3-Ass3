# Assignment 3 Justification

If you think any of the questions within the sections are not applicable, please write "N/A".
If you are answering a criteria in a different section, please describe that like: "4a. This criteria is met by the implementation described in 2b."

Video demonstration of (almost) everything in this project incase it fails to run on another machine:
https://youtu.be/PsKbE1M6UJg

CONTROLS:
- Move mouse to move view
- WASD to move
- Double tap W to run
- SPACE to jump
- LEFT CLICK to break the highlighted block
- RIGHT CLICK to place the block in your current hand
- MIDDLE CLICK to quickly put the put that you've highlighted into your hand
- MOUSE WHEEL to scroll through hotbar
- LEFT SHIFT to sneak (Same functionality as in Minecraft, where you can not fall off blocks in sneak state. Sneaking will also stop highlighting - blocks so that you can see the specular reflections better)
- TAB to maximise/minimise game window
- C to sleep and wake up (Toggles between player and cutscene camera)
- F to toggle flight mode (Toggles between player and flying camera)
- G to print debug information into terminal, such as framerate player position etc.
- O to convert the current world into lines of code so that it can be used as a preset world
- E to change from the two hotbars (general blocks -> wool blocks and vice versa)
- I to see these instructions ingame
- Left and Right brackets to change sea level
- F1 to hide/show HUD
- F2 to cycle through the different Cube Map Reflections
- F3 to cycle through the different HDR tone mappings
- F4 to cycle through the different Kernels (When underwater, the blur kernel is applied automatically)
- ESC to terminate game

## Section 1: Design Pitch. Describe a possible setting and what kind of graphics technology would be necessary to implement this. These components do not have to be implemented in this project. Suggestion: 200 words should be more than enough for this.

•	What is the setting?

The setting is a blank canvas and the player gets to decide on what it’s going to be. Using the in-game tools they are given, they can be able to craft a variety of settings by mixing and matching blocks that will give different effects depending on the combination. (Different blocks can emit light/bloom effect, emit different particles, change its behaviour based on being underwater or not etc.)

•	What is unique about it?

The player can experiment with different combinations to try and discover new effects to put into their scenery. Many other projects are static and can’t vary much (only smoke particles can emit from this specific spot in the world etc.). On the other hand, nothing is inherently hardcoded into this setting’s environment and will change depending on how the player interacts with the world.

•	What kinds of Graphics features are necessary to display this setting?

-	Bloom for the light emitting blocks
-	Shadow mapping so the player can create darker areas in their world if the setting they desire requires it
-	Particle system to add more variety to the world they are creating
-	Lighting so the player can decide between a day setting or a night setting
-	Post-processing effects such as kernels so the player can take screenshots of their world through a variety of lens for a cinematic feel, retro feel etc.
-	Reflections such as planar or Cube mapping so the player can play around with blocks that will reflect the scene. Also makes the water more believable. Will need clipping planes to prevent unnecessary parts of the scene from being reflected.
-	Lens flare to add a more cinematic feel to the player’s scene if they want it (Not implemented in this project)
-	Deferred renderer to handle more lights in the scene if the player needs more lights (Not implemented in this project)
•	How will this world be used?

The player will use this world in first person by destroying and placing elements from a selection of blocks in their hot bar. They may create particular terrain that challenges players to traverse it etc.

•	Will characters traverse and view it in a particular way?

The player can jump on blocks, bounce off certain blocks, and fly around their world. When underwater, the vision becomes blurry and distorted to simulate being underwater.

•	How will it need to be rendered to be useful and visible to a player?

The scene will be rendered through multiple framebuffers. One for the shadow mapping for the aforementioned reason, another for the bloom effect (ping-ponged between two framebuffers to apply gaussian blur), one for real-time cube mapping and two more for the reflections in the water. One final frame buffer to apply the kernels that the player chooses to add effect to their scene.


## Section 2: Advanced Lighting

### 2a: Blinn-Phong Lighting. Include code references to the implementation of lighting in your project.

Changed from phong to bling phong by implementing a half way vector between the camera to fragment and light to fragment. This can be seen in the default.frag line 107 -> 109 and water.frag line 47 -> 48

### 2b: High Dynamic Range. Show use of an intermediate frame buffer to store light values > 1.0 and how that is converted to Low Dynamic Range. Include code references.

HDR can be first seen in the render loop in main.cpp between line 796 -> 819 and is stored into the finalFrameFBO. Bloom is also calculated at this stage. When rendering this scene, the hdr shader with hdrBloom.frag and hdrBloom.vert is used. The shader defaults to using exposure tone mapping, and uses a floating point framebuffer (which is created in utility.cpp at line 73) so it has more storage to store larger light values. Next, it uses exposure tone mapping (There are more tone mappings available and can be cycled through with F3) to convert from HDR to LDR at line
33 in hdrBloom.frag by taking the exponential of the color times the exposure. If exposure tone mapping is used, the game will generate a mipmap texture of the current scene and use the highest mipmap level as a guide on adjusting the exposure value when using exposure tone mapping. (This can be found in the findAvgColor function between line 123 -> 153 in utility.cpp, which is called at line 823 of main.cpp). This way, the screen will adjust to focus on dark parts/light parts like a normal human eye. The other tonemaps follow a similar principle, but it does not take advantage of the exposure levels.

## Section 3: Reflections

### 3a: Environment Mapped Reflections. Show at least one object can reflect a cube map. Cube map can be a preprepared skybox and the object doesn't need to reflect other objects in the scene. Include code references to implementation.

When starting up the program, you can choose to have the Mirror Block to sample render from a preprepared skybox (with its textures found in res/textures/premadeSkybox). You can also select premade skybox ingame by pressing F2. Rendering a preprepared skybox can be found in the "drawShinyTerrain" function between line 1891 -> 1912 of scene.hpp, and uses the cubeReflection shader (cubeReflection.frag and cubeReflection.vert) to calculate the reflected vector of the camera to the fragment and sample the environment cube map using that vector. Then, the shaders uses the mirror green screen texture, replacing all pure green texels with a reflection of the cubemap.

### 3b: Realtime Cube Map. Describe how the cube map is created during the frame render and then used for a reflective object. Include code references to the generation of the cube map (3a should already describe the reflection process).

Realtime cube map can be seen when placing down the Mirror Block and having selecting "Realtime Cubemap" at the start (or switch to it ingame). The Mirror Block is the block next to bedrock, or the block two spaces to the left of the starting block in the hotbar. The cube map is created every third frame, and is done between lines 1843->1859 and 1896 -> 1969. It creates the cubemap by first creating an empty cubemap texture (with null data) by calling texture_2d::createEmptyCubeMap(); found in texture_2d.cpp between lines 81 -> 95. and then write to it by using a camera with a field of vision of 90 degrees. With each face, the camera is rotated to match the face i.e. when writing for the top face, the camera will be pointed up. It then renders the scene and clears the color buffer bit for the next face. Once this is done, the associated frame and render buffer are destroyed leaving the texture only which is applied to the Mirror Block's reflection. All this can be seen in the renderToEnvironmentMap(); function between line 1991 -> 2070 of scene.hpp.

### 3c: In-World Camera/Magic Mirror. Describe how you have placed a camera in a scene that renders to a texture. Show where both of these objects are and how they function to show a different viewpoint of the scene. Include references to code.

This is merged with planar reflections, as the Assignment Specifications says Planar Reflections can cover this component. A camera is placed directly beneath the player with an opposing pitch value to simulate reflection however. The distance between this camera and the current one is the distance between the current camera to the water surface multiplied by two. The placement of this reflection camera can be seen between line 757 -> line 763 of scene.hpp.

### 3d: Planar Reflections. Describe the system used to generate a planar reflection, including any techniques for perspective and/or near plane correction. Include references to code.

Planar reflection is used to generate the reflection off the water surface ingame. This generation starts in the render loop in main.cpp between line 614 -> 733. It first renders the scene via the reflection camera, which is positioned below in the way mentioned in the previous component. It makes use of a clipping plane so that anything below the water surface will not be rendered by this camera as it doesn't make sense for things below the water surface to be seen reflected on the surface. The same principle is applied for the refraction render, where it simply renders the scene from the player's point of view, but the clipping plane is set in a way that anything above the water surface will not be rendered (clipping plane is set between line 624 -> 638). Next, in the untamperedFBO, the scene is rendered normally, and the water is textured using water.frag and water.vert, which takes in the refraction and reflection texture and mixes it together with the water texture (If the player is looking directly at the water, the refraction texture will be prioritised. If looking perpendicularly, the reflection texture will be prioritised instead). This shader also uses a DuDvMap and a Normal map to distort the reflection/refraction and also to reflect any light coming from light-emitting blocks. The normal map is pixelated on purpose to fit the Minecraft style. The water surface can change by pressing Left Bracket to decrease, and Right Bracket to increase.

## Section 4: Post Processing

### 4a: Simple Colour Change. Describe how this project uses a framebuffer and what change is made to the colour data, including references to where this is implemented in code.

Framebuffers are used in this project for post processing effects by storing a scene into a texture to be edited/distorted by a shader based on the palyer's current environment. The player also has some freedom on how they want their scene to be post processed with the F4 key. The player can press F4 to cycle through the kernels until they get to the last two, which will pixelate the screen or invert all the colors on the screen. This is done in the finalFrame shader which uses finalFrame.vert and finalFrame.frag by first rendering the hdrBloom scene to the finalFrameFBO, and then passing the associated texture into the finalFrame shader. The neg and pixelate filters can be found between line 32 -> 43 in finalFrame.frag. Pixelate rounds the texture co-ordinate to a nearest floor co-ordinate to get a pixelated/retro feel, while neg subtracts the color from white rgb color.

Simple colour change also gets automatically applied when the player is underwater by tinting the screen with a slight blue color seen in hdrBloom.frag between line 99 -> 102. Additionally, it also distorts the screen using a sin curve between line 74 -> 84 and a value called "cycle", which is pointing to the waterWaveCycle variable found in main.cpp between line 110 -> 139.

### 4b: Use of Kernels. Describe the kernel sampling technique in use, including code references.

Kernels can be toggled at anytime by pressing F4. There are many kernels: edge, emboss, sharpen, right sobel and blur. All these implementation can be found in finalFrame.frag between line 48 -> 103. The blur kernel is automatically applied in the hdrBloom shader when the player is beneath the water surface to immerse them into the new environment of being underwater. This can be seen hdrBloom.frag between line 80 -> 84 and line 25 -> 35. The kernel sampling technique uses a float array with "weighted" values stored within which dictates the amount to sample around the neighbouring pixels and the centre pixel when determining the centre pixel's color.

### 4c: Multiple Intermediate Framebuffers. Show use of more than one framebuffer for post processing (like chaining two post processing effects in a row). Include references to code.

Through out one cycle of the render loop of main.cpp, many framebuffers are used.
- The scene is first passed through the depthMapFBO. This renders the depth values from the perspective of the sun, which is later on used to calculate the shadows. [line 582 -> 585, using shadow.vert and shadow.frag]
- Next, if realtime cubemapping is enabled, the scene is rendered 6 times per Mirror Block into a framebuffer and written into a texture as previously mentioned in the Real time cubemapping component. [Line 599 -> 609, using default.vert and default.frag]
- The scene is then rendered 3 times again, once for reflection into the reflectionFBO from the reflection camera, then once for the normal view of the player with a clipping plane positioned at the water surface into the refractionFBO, and another for a normal view with no clipping plane into the untamperedFBO. When rendering into the untamperedFBO, the game uses the scene rendered into reflectionFBO and refractionFBO to texture the water surface as mentioned in the Planar Reflection Component. [Line 614 -> 733, using default.vert/default.frag and water.vert/water.frag]
- The world is drawn again from the player's perspective but this time only drawing the the bloom texture of each element in the scene. This scene is then passed into two framebuffers named pingpong, where it passes the bloom scene into each other while applying gaussian blur. [Line 736 -> 771 for rendering only the bloom textures, and then line 781 -> 791 for the pingpong framebuffers which blurs bloom render with blur.frag/blur.vert]
- The HDR effect and the bloom blur scene is combined into the finalFrameFBO, and the HUD is also applied over the scene after clearing the depth buffer bit. This shader also applies any underwater effects if the player is submerged in water. The underwater effects are done here to not disturb/disrupt the HUD [Line 796 -> 819 using hdrBloom.frag/hdrBloom.vert]
- Finally, the scene in finalFrameFBO is outputted onto the screen using the finalFrame shader which will apply any kernals or filters the player has selected. This shader also deals with creating the slow motion effect using the temporal FBO. [Line 832 -> 851]
- The scene in the finalFrameFBO is also copied into temporalFrameAFBO and the scene in temporalFrameAFBO is copied into temporalFrameBFBO for the slow motion effect used in the finalFrame shader.

### 4d: Multiple Render Targets. Describe how this project uses multiple frame buffers to write information from the initial fragment shader pass and how it uses them in post processing. Include references to fragment shader output code as well as post processing code.

Multiple Render Targets is achieved when doing the bloom effect. The game first renders a scene with the target of all normal textures in view, and then in another framebuffer targets the bloom texture information, which is later to be blurred and applied to the final scene with additive blending. The targeting of the bloom information and the fragment shader output code can be seen between line 737 -> 771, which then used in the pingpong frame buffers in post processing to blur using gaussian blur in the blur shader at lines 781 -> 791 (Using blur.vert). Bloom is only emitted from light emitting objects.

Another example would be when rendering the shadows. The game renders the depth of the current scene into a separate buffer first, seen in main.cpp between line 566 -> 585. This depth buffer is used in the rendering of the final scene, where the depthMap is used to determine if an object is in a shadow or not (Post processing code of this depthmap in default.frag line 71 -> 94) This depth information is then passed into the default shader. More about shadows is explained below in the "Show me what you got".

### 4e: Temporal Post Processing. Show storage of framebuffers from previous frame(s) and how they are being used in post processing. Include references to where this is happening in code.

The storage of framebuffers from previous frames can be found at the end of the render loop between line 867 -> 881, where the current frame is rendered into temporalFrameAFBO, and the previous frame is rendered into temporalFrameBFBO using the copyFrame shader, which is a shader that simply draws the scene onto a quad with no differences. These framebuffers are used in the next loop between line 841 -> 851 where it is processed by the finalFrame shader in finalFrame.vert between line 126 -> 130, where it mixes the current, previous and older frame together to create a slow motion/sluggish effect. This only occurs when the player is beneath the water surface however.

## Section 5: Show Me What You Got. Implement any techniques that are not part of previous criteria for assignments in this course. This is open, so must be documented clearly with code references to show what has been created to add to the visual effects of the scene or objects in it. 

Extra techniques implemented: Shadow mapping and a Particle System

* Shadow mapping
Shadow mapping implementation starts with the frame buffer creation between main.cpp lines 481 -> 499 which reads in the depth component. Then at line 520, the game creates the light projection matrix, which is an orthogonal camera so that the shadows casted will be parallel with one another. At line 563 -> 584, the game calculates where the sun is, which will be the same as where this orthogonal camera will be and will be looking at where the player is. The game then calculates the view-projection matrix of the sun camera and renders the world + the player model with the shadow shader, which only cares about the gl_position of fragments. This will create a depth map in the texture binded to the depthmap fbo, which is then sampled in default.frag between line 647 -> 648 in main.cpp and in default.frag, that texture is seen in calcShadow() function between line 71 -> 94. Here, bias correcting is applied to the shadows, where the values seen on line 79 have been trial and errored such that there won't be any shadow acne at any time during the day. Percentage Closer Filtering is then applied after so that the shadows can look more smooth and less jagged/pixelated. It performs PCF by sampling the neighbouring tex co-ordinates on the shadow map to determine whether those co-ordinates are in shadow, and then averages the total texels sampled to determine how much of the directional light shines onto the surface (At line 150. If the average is 9/9, then it is completely in shadow, 0/9 means it is not in shadow). The issue with peter panning was not apparent on blocks so not much has been done to fix that. However, with the player shadow, I've offsetted the player model depending on the time of day so that it's feet will always seem like it's at the bottom of the player (Done between line 195 -> 200 in scene.hpp).

* Particle System
As particles play an important role in the original Minecraft to breathe life into their game, I decided to replicate their particle system as well. All the particles are stored in scene.hpp at line 416, and all the functions related to it are found in particle.hpp and particle.cpp. The particle struct in particle.hpp between line 12->26 contains all the data required to animate the particle, such as how much time it has left before it despawns, its velocities, its texture etc. Then, in all the functions that spawn particles inside particle.cpp, a particle is malloc'd with the required stats (and varied floats to make their animation more random and varied) and then stored inside the vector in scene.hpp. When drawing all the particles between line 285 -> 335, the model matrix first translates the particle to it's rightful position, and then it removes the top left 3x3 cells in the matrix and replaces it with a 3x3 identity matrix. This causes the particle to always face the camera no matter where the camera is. It then scales the particle appropriately. If it detects that the particle's life timer has reached below 0, it will free the memory associated with it, destroys the mesh and removes it from the vector list. If not, then it will animate the particle with the function between line 337 -> 360, which takes into account all the data that's stored inside the particle struct given. The particle shape is also created in shapes.cpp between line 194 -> 244. If a true boolean is passed into the function found between those lines, it will sample a random 3x3 in the provided texture.

Particles emit from different sources:
- Marccoin Block
- Redstone Ore
- Crying Obsidian
- Magma Block (Bubbles when underwater, smoke when above water)
- Brain, Bubble, Fire, Horn and Tube coral blocks emits fish particles when under water
- During night time, firefly particles will spawn
- Player landing on a solid block at a certain velocity will create particles + Player running
- Breaking Blocks
- Changing the terrain under water

## Section 6: Subjective Mark

### How have you fulfilled this requirement?

Building off from Assignment 2, I furthered my recreation of Minecraft by trying to incorporate the Graphic technologies required of this assignment; technologies that aren't present in Modern Minecraft. As such, the goal of this Assignment for me was to not perfectly replicate Minecraft, but to add a modern twist to it.

### What is your vision for the scene you are showing?

In addition to the previous vision of Assignment 2, I hope to show players what Minecraft may have looked like if it was created 10 years later with all the new graphical technologies such as shadow mapping and blooming.

### Are you referencing anything in this scene? Is this a quote or homage to any other art? Please describe it.

This is a remake of Notch Pearson's Minecraft, using textures by Jasper "JAPPA" Boerstra. The Block of Marc uses the Marccoin found in the tutorials and the Chicken emoji found on the course discord. The cloud texture was pulled from this Minecraft Resource Pack [https://www.planetminecraft.com/texture-pack/items-in-the-clouds/] and was edited a bit to suit the project. The fish and firefly particles was drawn together by me with reference to Minecraft fish/firefly. The preset map name "Marccoin in the Tower" was meant to be a reference to the Super Mario 64 levels titled wtih "Bowser in the (...)". The pre-rendered cubemap is a map of the iconic "pack.png" texture in the Minecraft folder and was taken from this video [https://www.youtube.com/watch?v=ea6py9q46QU]. The Mirror Block texture and concept was an entirely original creation.

### Please give instructions for how a viewer/player should interact with this scene to see all the content you have created.

Instructions (including the new ones) can be seen when the player first boots up the game. Just like with Assignment 2, the player needs to input their desired world size, the world preset, and their render distance. In addition now, there is an option to toggle which cubemap reflection to choose from before the game starts, as real time cubemapping may slow the game down substantially. If the player wishes to play around with the new particles, they may place down any of the particle-emitting blocks mentioned earlier. All controls can be seen at the top of this page as well.

When underwater, the player can hold space to swim upwards. Their movement and vision is also restricted.

Thank you for playing through this recreation of Minecraft!

## Section 7 (optional): Code Style Feedback

### There are no marks assigned to code style in this assignment. However, if you would like to receive some feedback on your coding style, please point out a section of your code (up to 100 lines) and your tutor will give you some written feedback on the structure and readability of that code.

Write your answer here (lines numbers or function names that you'd like reviewed)

## Section 8 (optional): Use of External Libraries

### What does the library do?

Write your answer here...

### Why did you decide to use it?

Write your answer here...

### How does this integrate with the assignment project build system? Please include any instructions and requirements if a marker needs to integrate an external library.

Write your answer here...
