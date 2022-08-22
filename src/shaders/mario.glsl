R"====(
#define MAX_LIGHTS 4

uniform mat4 view;
uniform mat4 projection;
uniform vec4 uViewPos;
uniform vec4 uParam; // x - time, y - water height, z - clip plane sign, w - clip plane height
uniform vec4 uLightPos[MAX_LIGHTS];
uniform vec4 uLightColor[MAX_LIGHTS]; // xyz - color, w - radius * intensity
uniform vec4 uMaterial;	// x - diffuse, y - ambient, z - specular, w - alpha
uniform vec4 uAmbient[6];
uniform sampler2D marioTex;

v2f vec3 v_position;
v2f vec3 v_color;
v2f vec3 v_normal;
v2f vec4 v_light;
v2f vec2 v_uv;
v2f vec4 vViewVec;	// xyz - dir * dist, w - coord.y * clipPlaneSign
v2f vec4 vDiffuse;
v2f vec3 vLightVec;

#ifdef VERTEX

	in vec3 position;
	in vec3 normal;
	in vec3 color;
	in vec2 uv;

	vec3 calcAmbient(vec3 n)
	{
		vec3 sqr = n * n;
		vec3 pos = step(0.0, n);
		return	sqr.x * mix(uAmbient[1].xyz, uAmbient[0].xyz, pos.x) +
				sqr.y * mix(uAmbient[3].xyz, uAmbient[2].xyz, pos.y) +
				sqr.z * mix(uAmbient[5].xyz, uAmbient[4].xyz, pos.z);
	}

	void main()
	{
		v_position = position;
		v_color = color;
		v_normal = normal;
		v_uv = uv;

		// code from compose.glsl
		vViewVec = vec4((uViewPos.xyz - position) * 1, 0.0);
		vDiffuse = vec4(v_color * uMaterial.x, 1.0);
		vDiffuse.xyz *= 2.0;
		vDiffuse *= uMaterial.w;

		vec3 lv0 = (uLightPos[0].xyz - position) * uLightColor[0].w;
		vec3 lv1 = (uLightPos[1].xyz - position) * uLightColor[1].w;
		vec3 lv2 = (uLightPos[2].xyz - position) * uLightColor[2].w;
		vec3 lv3 = (uLightPos[3].xyz - position) * uLightColor[3].w;
		vLightVec = lv0;

		vec4 lum, att;
		lum.x = dot(normal, normalize(lv0)); att.x = dot(lv0, lv0);
		lum.y = dot(normal, normalize(lv1)); att.y = dot(lv1, lv1);
		lum.z = dot(normal, normalize(lv2)); att.z = dot(lv2, lv2);
		lum.w = dot(normal, normalize(lv3)); att.w = dot(lv3, lv3);
		vec4 light = max(vec4(0.0), lum) * max(vec4(0.0), vec4(1.0) - att);

		vec3 ambient = calcAmbient(v_normal);

		v_light.xyz = uLightColor[1].xyz * light.y + uLightColor[2].xyz * light.z + uLightColor[3].xyz * light.w;
		v_light.w = 0.0;
		v_light.xyz += ambient + uLightColor[0].xyz * light.x;
		v_light *= 1.5;

		gl_Position = projection * view * vec4( position, 1. );
	}

#endif
#ifdef FRAGMENT

	out vec4 color;

	void main()
	{
		vec4 texColor = texture2D( marioTex, v_uv );
		vec3 light = v_light.xyz;

		vec3 mainColor = mix( v_color, texColor.rgb, texColor.a ); // v_uv.x >= 0. ? texColor.a : 0. );
		color = vec4( mainColor * light, 1 );
	}
#endif
)===="
