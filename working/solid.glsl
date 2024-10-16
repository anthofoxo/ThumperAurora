#inject
#include "common.glsl"

INPUT(vec3, iPosition, 0);
INPUT(vec3, iNormal, 1);
INPUT(vec2, iTexCoord, 2);
INPUT(uint, iColor, 3);

VARYING(vec3, vNormal);

OUTPUT(vec4, oColor, 0);

uniform mat4 uProjection;
uniform mat4 uView;
uniform float uFlip;
uniform vec3 uColor;

#ifdef VERT
void main(void) {
	vec3 pos = iPosition;
	if(uFlip > 0.5) pos.y *= -1.0;
	
	vec3 norm = iNormal;
	if(uFlip > 0.5) norm.y *= -1.0;

	gl_Position = uProjection * uView * vec4(pos, 1.0);
	vNormal = norm;
}
#endif

#ifdef FRAG
void main(void) {
	oColor = vec4(uColor, 1.0);

	vec3 sufaceNorm = normalize(vNormal);

	if(!gl_FrontFacing) {
		sufaceNorm *= -1.0;
		oColor.rgb = mix(oColor.rgb, vec3(0.0, 0.0, 1.0), 0.5);
	}

	oColor.rgb *= max(0.3, dot(sufaceNorm, normalize(vec3(0.0, 0.4, 1.0))));
}
#endif