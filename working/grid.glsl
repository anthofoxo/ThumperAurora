#inject
#include "common.glsl"

VARYING(vec3, vWorldPos);
OUTPUT(vec4, oColor, 0);

uniform mat4 uProjection;
uniform mat4 uView;

const float kGridSize = 1000.0;

#ifdef VERT
const vec3 kPos[4] = vec3[4](
    vec3(-1.0, 0.0,  1.0),
    vec3(-1.0, 0.0, -1.0),
    vec3( 1.0, 0.0,  1.0),
    vec3( 1.0, 0.0, -1.0)
);

void main(void) {
	vWorldPos = kPos[gl_VertexID] * kGridSize;
    vec3 cameraWorldPos = (inverse(uView) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vWorldPos.x += cameraWorldPos.x;
    vWorldPos.z += cameraWorldPos.z;
    gl_Position = uProjection * uView * vec4(vWorldPos, 1.0);
}
#endif

#ifdef FRAG
const float kGridMinPixelsBetweenCells = 2.0;
const float kGridCellSize = 1.0;
const vec4 kGridColorThin = vec4(0.5, 0.5, 0.5, 1.0);
const vec4 kGridColorThick = vec4(0.0, 0.0, 0.0, 1.0);

float log10(float x) {
    float f = log(x) / log(10.0);
    return f;
}


float satf(float x) {
    float f = clamp(x, 0.0, 1.0);
    return f;
}


vec2 satv(vec2 x) {
    vec2 v = clamp(x, vec2(0.0), vec2(1.0));
    return v;
}


float max2(vec2 v) {
    float f = max(v.x, v.y);
    return f;
}

void main(void) {
	vec2 dvx = vec2(dFdx(vWorldPos.x), dFdy(vWorldPos.x));
    vec2 dvy = vec2(dFdx(vWorldPos.z), dFdy(vWorldPos.z));

    float lx = length(dvx);
    float ly = length(dvy);

    vec2 dudv = vec2(lx, ly);
    float l = length(dudv);

    float LOD = max(0.0, log10(l * kGridMinPixelsBetweenCells / kGridCellSize) + 1.0);

    float GridCellSizeLod0 = kGridCellSize * pow(10.0, floor(LOD));
    float GridCellSizeLod1 = GridCellSizeLod0 * 10.0;
    float GridCellSizeLod2 = GridCellSizeLod1 * 10.0;

    dudv *= 4.0;

    vec2 mod_div_dudv = mod(vWorldPos.xz, GridCellSizeLod0) / dudv;
    float Lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

    mod_div_dudv = mod(vWorldPos.xz, GridCellSizeLod1) / dudv;
    float Lod1a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

    mod_div_dudv = mod(vWorldPos.xz, GridCellSizeLod2) / dudv;
    float Lod2a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

    float LOD_fade = fract(LOD);
    vec4 Color;

    if (Lod2a > 0.0) {
        Color = kGridColorThick;
        Color.a *= Lod2a;
    } else {
        if (Lod1a > 0.0) {
            Color = mix(kGridColorThick, kGridColorThin, LOD_fade);
            Color.a *= Lod1a;
        } else {
            Color = kGridColorThin;
            Color.a *= (Lod0a * (1.0 - LOD_fade));
        }
    }

    vec3 cameraWorldPos = (inverse(uView) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    float OpacityFalloff = (1.0 - satf(length(vWorldPos.xz - cameraWorldPos.xz) / kGridSize));

    Color.a *= OpacityFalloff * 0.5;
    oColor = Color;
}
#endif