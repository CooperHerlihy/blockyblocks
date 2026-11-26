begin_vertex_shader

#version 330

uniform float u_aspect;

layout (location = 0) in vec2 vert_pos;

void main()
{
    gl_Position = vec4(vert_pos.x / u_aspect, vert_pos.y, 0.0, 1.0);
}

end_vertex_shader
begin_fragment_shader

#version 330

layout (location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(0.0, 0.0, 0.0, 1.0);
}

end_fragment_shader
