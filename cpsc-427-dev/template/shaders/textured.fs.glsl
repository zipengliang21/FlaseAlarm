#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// for mask
in vec2 fragPos;
uniform bool useMask;
uniform vec2 playerPos;
uniform float rBright;
uniform float rDark;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec4 texColor=texture(sampler0, texcoord);
	float coef=1.0f;
	
	if (useMask)
	{
		// when r < rBright, coef should be 1.
		// when r >= rBright and r < rDark, coef should be 1 to 0.
		// when r >= rDark, coef should be 0.
		// r is the distance between fragment and player
	
		float r = length(fragPos - playerPos);
		coef = (r-rDark)/(rBright-rDark);
		coef = clamp(coef, 0, 1);
	}
	// if useMask = false (default), the mask will be ignored.
	
	color = vec4(fcolor, 1.0) * texColor * coef;
}
