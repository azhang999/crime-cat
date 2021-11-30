#include "TextTextureProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Load< TextTextureProgram > text_texture_program(LoadTagEarly);

// TextTextureProgram is equivalent to ColorTextureProgram, except adapted to accomodate text
// using the shader definitions in https://learnopengl.com/In-Practice/Text-Rendering
TextTextureProgram::TextTextureProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		// ---------------- vertex shader ----------------
		"#version 330\n"

		"in vec4 vertex;\n"       // *** INPUT: a vertex position
		"out vec2 TexCoords;\n"     // output of vertex shader -> input fragment shader so give same name?
        
        "uniform mat4 projection;\n"
    
		"void main() {\n"
        "   gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
		"   TexCoords = vertex.zw;\n"
		"}\n"

        // **** Since the color of the text of the game is going to dynamically change, 
        // and we don't have access to screen coordinates here, 
        // we're going to set the color of the text and the object-to-clip matrix later,
        // binding them as uniforms
	,
		// ---------------- fragment shader ----------------
		"#version 330\n"

		"in vec2 TexCoords;\n"
		"out vec4 color;\n"

		"uniform sampler2D tex;\n"
		"uniform vec3 textColor;\n"

		"void main() {\n"
        "   vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, TexCoords).r);\n"
        "   color = vec4(textColor, 1.0) * sampled;\n"
		"}\n"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.

	//look up the locations of vertex attributes:
	Position_vec4 = glGetAttribLocation(program, "vertex");

	//look up the locations of uniforms:
	OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "projection");
	TextSampler = glGetUniformLocation(program, "tex");
    Color_vec3 = glGetUniformLocation(program, "textColor");

	//set TextSampler to always refer to texture binding zero:
	glUseProgram(program); //bind program -- glUniform* calls refer to this program now

	// glUniform1i(TextSampler, 0); //set TEX to sample from GL_TEXTURE0

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

TextTextureProgram::~TextTextureProgram() {
	glDeleteProgram(program);
	program = 0;
}

