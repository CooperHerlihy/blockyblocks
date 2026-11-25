begin_vertex_shader

#version 330

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertUV;

out vec2 fragUV;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

void main()
{
    gl_Position = uProj * uView * uModel * vec4(vertPos, 1.0);
    fragUV = vertUV;
}

end_vertex_shader
begin_fragment_shader

#version 330

in vec2 fragUV;

layout (location = 0) out vec4 outColor;

uniform sampler2D uTex;

void main()
{
    outColor = texture(uTex, fragUV);
}

end_fragment_shader
