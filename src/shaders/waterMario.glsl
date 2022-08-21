R"====(
#define UNDERWATER_COLOR	vec3(0.6, 0.9, 0.9)
#define MAX_LIGHTS 4

uniform mat4 view;
uniform mat4 projection;
uniform vec4 uViewPos;
uniform vec4 uParam;
uniform vec4 uLightPos[MAX_LIGHTS];
uniform vec4 uLightColor[MAX_LIGHTS]; // xyz - color, w - radius * intensity
uniform vec4 uMaterial;	// x - diffuse, y - ambient, z - specular, w - alpha
uniform sampler2D marioTex;

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
		float light = .5 + .5 * clamp( dot( v_normal, v_light ), 0., 1. );
		vec4 texColor = texture2D( marioTex, v_uv );
		vec3 mainColor = mix( v_color, texColor.rgb, texColor.a ); // v_uv.x >= 0. ? texColor.a : 0. );
		color = vec4( mainColor * light, 1 );

		float uwSign = step(uParam.y, v_position.y);
		color.xyz = mix(color.xyz, color.xyz * UNDERWATER_COLOR, uwSign);
		color.xyz = mix(UNDERWATER_COLOR * 0.2, color.xyz, 1);
	}

#endif
)===="
