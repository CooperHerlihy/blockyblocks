SHADER_BEGIN(VERTEX)

#version 330

layout (location = 0) in vec3 inPos;

void main()
{
    gl_Position = vec4(inPos, 1.0);
}

SHADER_END(VERTEX)
SHADER_BEGIN(FRAGMENT)

#version 330

layout (location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(0.6f, 0.2f, 0.1f, 1.0f);
}

SHADER_END(FRAGMENT)
