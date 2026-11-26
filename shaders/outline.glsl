begin_vertex_shader

#version 330

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

layout (location = 0) in vec3 vert_pos;

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(vert_pos, 1.0);
}

end_vertex_shader
begin_fragment_shader

#version 330

layout (location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
}

end_fragment_shader
