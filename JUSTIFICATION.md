# Assignment 3 Justification

If you think any of the questions within the sections are not applicable, please write "N/A".
If you are answering a criteria in a different section, please describe that like: "4a. This criteria is met by the implementation described in 2b."

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
-	Deferred renderer to handle more lights in the scene (Not implemented in this project)
•	How will this world be used?
The player will use this world in first person by destroying and placing elements from a selection of blocks in their hot bar.

•	Will characters traverse and view it in a particular way?
The player can jump on blocks, bounce off certain blocks, and fly around their world. When underwater, the vision becomes blurry and wavy to simulate being underwater.

•	How will it need to be rendered to be useful and visible to a player?
The scene will be rendered through multiple framebuffers. One for the shadow mapping for the aforementioned reason, another for the bloom effect (ping-ponged between two framebuffers to apply gaussian blur), one for real-time cube mapping and two more for the reflections in the water. One final frame buffer to apply the kernels that the player chooses to add effect to their scene.


## Section 2: Advanced Lighting

### 2a: Blinn-Phong Lighting. Include code references to the implementation of lighting in your project.

I did not complete part 2a. (If you would like your 2a to be marked, you will need to replace this line)

### 2b: High Dynamic Range. Show use of an intermediate frame buffer to store light values > 1.0 and how that is converted to Low Dynamic Range. Include code references.

I did not complete part 2b. (If you would like your 2b to be marked, you will need to replace this line)

## Section 3: Reflections

### 3a: Environment Mapped Reflections. Show at least one object can reflect a cube map. Cube map can be a preprepared skybox and the object doesn't need to reflect other objects in the scene. Include code references to implementation.

I did not complete part 3a. (If you would like your 3a to be marked, you will need to replace this line)

### 3b: Realtime Cube Map. Describe how the cube map is created during the frame render and then used for a reflective object. Include code references to the generation of the cube map (3a should already describe the reflection process).

I did not complete part 3b. (If you would like your 3b to be marked, you will need to replace this line)

### 3c: In-World Camera/Magic Mirror. Describe how you have placed a camera in a scene that renders to a texture. Show where both of these objects are and how they function to show a different viewpoint of the scene. Include references to code.

I did not complete part 3c. (If you would like your 3c to be marked, you will need to replace this line)

### 3d: Planar Reflections. Describe the system used to generate a planar reflection, including any techniques for perspective and/or near plane correction. Include references to code.

I did not complete part 3d. (If you would like your 3d to be marked, you will need to replace this line)

## Section 4: Post Processing

### 4a: Simple Colour Change. Describe how this project uses a framebuffer and what change is made to the colour data, including references to where this is implemented in code.

I did not complete part 4a. (If you would like your 4a to be marked, you will need to replace this line)

### 4b: Use of Kernels. Describe the kernel sampling technique in use, including code references.

I did not complete part 4b. (If you would like your 4b to be marked, you will need to replace this line)

### 4c: Multiple Intermediate Framebuffers. Show use of more than one framebuffer for post processing (like chaining two post processing effects in a row). Include references to code.

I did not complete part 4c. (If you would like your 4c to be marked, you will need to replace this line)

### 4d: Multiple Render Targets. Describe how this project uses multiple frame buffers to write information from the initial fragment shader pass and how it uses them in post processing. Include references to fragment shader output code as well as post processing code.

I did not complete part 4d. (If you would like your 4d to be marked, you will need to replace this line)

### 4e: Temporal Post Processing. Show storage of framebuffers from previous frame(s) and how they are being used in post processing. Include references to where this is happening in code.

I did not complete part 4e. (If you would like your 4e to be marked, you will need to replace this line)

## Section 5: Show Me What You Got. Implement any techniques that are not part of previous criteria for assignments in this course. This is open, so must be documented clearly with code references to show what has been created to add to the visual effects of the scene or objects in it. 

I did not complete part 5. (If you would like your 5 to be marked, you will need to replace this line)

## Section 6: Subjective Mark

### How have you fulfilled this requirement?

I did not fulfill this requirement and will not receive marks for Section 6. 
(If you would like your Section 6 to be marked, you will need to replace these lines)

### What is your vision for the scene you are showing?

I did not have a specific vision for this scene and do not need this to be marked.

### Are you referencing anything in this scene? Is this a quote or homage to any other art? Please describe it.

Write your answer here...

### Please give instructions for how a viewer/player should interact with this scene to see all the content you have created.

Write your answer here...

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
