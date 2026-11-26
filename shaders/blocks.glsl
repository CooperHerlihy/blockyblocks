begin_vertex_shader

#version 330

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec2 vert_uv;

out vec2 frag_uv;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(vert_pos, 1.0);
    frag_uv = vert_uv;
}

end_vertex_shader
begin_fragment_shader

#version 330

in vec2 frag_uv;

layout (location = 0) out vec4 outColor;

uniform sampler2D uTex;

void main()
{
    outColor = texture(uTex, frag_uv);
}

end_fragment_shader
