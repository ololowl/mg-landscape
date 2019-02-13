#version 330 core
in vec3 vFragPosition;
in vec2 vTexCoords;
in vec3 vNormal;

out vec4 color;

uniform sampler2D WaterTexture;
uniform sampler2D GrassTexture;
uniform sampler2D RockTexture;
uniform sampler2D ForestTexture;
uniform sampler2D SandTexture;
uniform sampler2D SnowTexture;

uniform int mode;
uniform int fog;


//heights
float waterMax = 0.85f;
float sandMin = 0.95f;
float sandMax = 1.4f;
float grassMin = 1.7f;
float grassMax = 3.75f;
float forestMin = 5.5f;
float forestMax = 6.0f;
float rockMin = 7.5f;
float rockMax = 8.5f;
float globalMax = 10.0f;
//fog
float fogDensity = 0.04;

void main()
{
	vec3 lightDir = vec3(1.0f, 1.0f, 0.0f); 
	vec3 col, col1, col2;
	vec4 color1, color2;
	float kh;
	float kd = max(dot(vNormal, lightDir), 0.0);
	if (mode == 0) {
		if (vFragPosition.y < waterMax) {
			kh = vFragPosition.y / waterMax + 0.3;
			col = vec3(0.2f, 0.15f, kh * 0.9f); 		// water
			color1 = mix(texture(WaterTexture, vTexCoords), vec4(col, 1.0f), 0.5);
			color = color1 * kd;
		} else if (vFragPosition.y < sandMin) {
			kh = (vFragPosition.y - waterMax) / (sandMin - waterMax);
			col1 = vec3(0.2f + kh * 0.58f, 0.15f + kh * 0.35f, 1.2f - kh * 0.9f);
			col2 = vec3(0.78f, 0.5f, 0.3f);
			color1 = mix(texture(WaterTexture, vTexCoords), vec4(col1, 1.0f), 0.5);
			color2 = mix(texture(SandTexture, vTexCoords), vec4(col2, 1.0f), 0.5);
			color = kd * mix(color1, color2, kh);
		} else if (vFragPosition.y < sandMax) {
			kh = 1.3f - (vFragPosition.y - sandMin) / (sandMax - sandMin);
			col = vec3(kh * 0.6f, 0.5f, 0.3f); 		// sand
			color1 = mix(texture(SandTexture, vTexCoords), vec4(col, 1.0f), 0.5);
			color = kd * color1;
		} else if (vFragPosition.y < grassMin) {
			kh = (vFragPosition.y - sandMax) / (grassMin - sandMax);
			col1 = vec3(0.18f + kh * 0.02f, 0.5f + kh * 0.54f, 0.3f);
			col2 = vec3(0.2f, 1.04f, 0.3f);
			color1 = mix(texture(SandTexture, vTexCoords), vec4(col1, 1.0f), 0.5);
			color2 = mix(texture(GrassTexture, vTexCoords), vec4(col2, 1.0f), 0.5);
			color = kd * mix(color1, color2, kh);
		} else if (vFragPosition.y < grassMax){
			kh = 1.3f - (vFragPosition.y - grassMin) / (grassMax - grassMin);
			col = vec3(0.2f, kh * 0.8f, 0.3f); 		// grass
			color1 = mix(texture(GrassTexture, vTexCoords), vec4(col, 1.0f), 0.5);
			color = kd * color1;
		} else if (vFragPosition.y < forestMin){
			kh = (vFragPosition.y - grassMax) / (forestMin - grassMax);
			col1 = vec3(0.2f + kh * 0.1f, 0.24f + kh * 0.28f, 0.3f - kh * 0.2f);
			col2 = vec3(0.3f, 0.52f, 0.1f);
			color1 = mix(texture(GrassTexture, vTexCoords), vec4(col1, 1.0f), 0.5);
			color2 = mix(texture(ForestTexture, vTexCoords), vec4(col2, 1.0f), 0.5);
			color = kd * mix(color1, color2, kh);
		} else if (vFragPosition.y < forestMax){
			kh = 1.3f - (vFragPosition.y - forestMin) / (forestMax - forestMin);
			col = vec3(0.3f, kh * 0.4f, 0.1f); 		// forest
			color1 = mix(texture(ForestTexture, vTexCoords), vec4(col, 1.0f), 0.5);
			color = kd * color1;
		} else if (vFragPosition.y < rockMin){
			kh = (vFragPosition.y - forestMax) / (rockMin - forestMax);
			col1 = vec3(0.3f + kh * 0.2f, 0.12f + kh * 0.38f, 0.1f + kh * 0.4f);
			col2 = vec3(0.5f, 0.5f, 0.5f);
			color1 = mix(texture(ForestTexture, vTexCoords), vec4(col1, 1.0f), 0.5);
			color2 = mix(texture(RockTexture, vTexCoords), vec4(col2, 1.0f), 0.5);
			color = kd * mix(color1, color2, kh);
		} else if (vFragPosition.y < rockMax) {
			kh = 1.3f - (vFragPosition.y - rockMin) / (rockMax - rockMin);
			col = vec3(0.5f, 0.5f, 0.5f); 		// grey
			color1 = mix(texture(RockTexture, vTexCoords), vec4(col, 1.0f), 0.5);
			color = kd * color1;
		} else {
			kh = min((vFragPosition.y - rockMax) / (globalMax - rockMax) + 0.2f, 1.0f);
			col1 = vec3(0.5f, 0.5f, 0.5f);	
			col2 = vec3(1.0f, 1.0f, 1.0f);
			color1 = mix(texture(RockTexture, vTexCoords), vec4(col1, 1.0f), 0.5);
			color2 = mix(texture(SnowTexture, vTexCoords), vec4(col2, 1.0f), 0.5);
			color = kd * mix(color1, color2, kh);
		}
	} else {
		color = vec4(vNormal, 1.0f);
	}
	if (fog == 1) {
		float fogCoord = gl_FragCoord.z / gl_FragCoord.w;
		if (fogCoord < 0) {
			fogCoord *= -1;
		}
		color = mix(color, vec4(1.0f, 0.7255f, 0.855f, 1.0f), exp(-pow(1.0f / (fogDensity * fogCoord), 2.0f)));
	}
}