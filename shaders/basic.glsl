begin_vertex_shader

#version 330

layout (location = 0) in vec2 vertPos;

uniform vec2 uPos;

void main()
{
    gl_Position = vec4(vertPos + uPos, 0.0, 1.0);
}


end_vertex_shader
begin_fragment_shader

#version 330

layout (location = 0) out vec4 outColor;

uniform vec4 uColor;

void main()
{
    outColor = uColor;
}

end_fragment_shader
