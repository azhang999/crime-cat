#pragma once

#include "GL.hpp"
#include "Load.hpp"

//Shader program that draws transformed, vertices tinted with vertex colors:
struct TextTextureProgram {
	TextTextureProgram();
	~TextTextureProgram();

	GLuint program = 0;

	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	
	//Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;
    GLuint TextSampler = -1U;
    GLuint Color_vec3 = -1U;

	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord
};

extern Load< TextTextureProgram > text_texture_program;
