#version 330 

in vec3 pos;
out vec4 color;
uniform sampler2D water;

void main()
{
    vec4 texColor = texture(water,gl_PointCoord);
    if(texColor.r < 0.3)discard;
    color = vec4(0.00f,0.30f,1.00f,0.5);
}
