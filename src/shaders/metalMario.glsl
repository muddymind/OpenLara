R"====(
uniform mat4 view;
uniform mat4 projection;
uniform vec4 uViewPos;
uniform vec4 uParam;
uniform samplerCube marioTex;

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
		v_color = color;
		v_normal = normal;
		v_light = transpose( mat3( view )) * normalize( vec3( 1 ));
		v_uv = uv;

		gl_Position = projection * view * vec4( position, 1. );
	}

#endif
#ifdef FRAGMENT

	out vec4 color;
	vec4 vViewVec;

	void main()
	{
		vec3 coord = vec3(1);
		vViewVec = vec4((uViewPos.xyz - coord) * 1, 0.0);
		vec3 rv = reflect(-normalize(vViewVec.xyz), normalize(v_normal.xyz));
		color = textureCube(marioTex, rv);
	}

#endif
)===="
