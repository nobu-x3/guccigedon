//glsl version 4.5
#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 1) uniform  SceneData{
	vec4 ambientColor;
} sceneData;


void main()
{
	outFragColor = vec4(texCoord.x,texCoord.y,0.5f,1.0f);
}
