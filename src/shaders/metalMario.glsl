R"====(
#define MAX_LIGHTS 4

uniform mat4 view;
uniform mat4 projection;
uniform vec4 uViewPos;
uniform vec4 uParam;
uniform vec4 uLightPos[MAX_LIGHTS];
uniform vec4 uLightColor[MAX_LIGHTS]; // xyz - color, w - radius * intensity
uniform vec4 uMaterial;	// x - diffuse, y - ambient, z - specular, w - alpha
uniform samplerCube marioTex;

v2f vec3 v_position;
v2f vec3 v_color;
v2f vec3 v_normal;
v2f vec3 v_light;
v2f vec2 v_uv;

#ifdef VERTEX

	in vec3 position;
	in vec3 normal;
	in vec3 color;
	in vec2 uv;

	void main()
	{
		v_position = position;
		v_color = color;
		v_normal = normal;
		v_light = transpose( mat3( view )) * normalize( vec3( 1 ));
		v_uv = uv;

		gl_Position = projection * view * vec4( position, 1. );
	}

#endif
#ifdef FRAGMENT

	out vec4 color;

	void main()
	{
		vec3 coord = vec3(1);
		vec4 vViewVec = vec4((uViewPos.xyz - coord) * 1, 0.0);
		vec3 rv = reflect(-normalize(vViewVec.xyz), normalize(v_normal.xyz));
		color = textureCube(marioTex, rv);
	}

#endif
)===="
