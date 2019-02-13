#version 330 core
in vec3 vFragPosition;
in vec2 vTexCoords;
in vec3 vNormal;

out vec4 color;

uniform float max_height;
uniform sampler2D water_texture;
uniform sampler2D sand_texture;

void main()
{
  vec3 lightDir = vec3(1.0f, 1.0f, 0.0f); 

  vec3 col;
  if (vFragPosition.y < max_height * 0.25f) {
      col = vec3(64.f / 255.f, 164.f / 255.f, 223.f / 255.f);
      float kd = max(dot(vNormal, lightDir), 0.0);
      color = mix(texture(water_texture, vTexCoords), vec4(kd * col, 1.0), 0.5);
  } else if (vFragPosition.y < max_height * 0.3f) {
      col = vec3(194.f / 255.f, 178.f / 255.f, 128.f / 255.f);
      float kd = max(dot(vNormal, lightDir), 0.0);
      color = mix(texture(sand_texture, vTexCoords), vec4(kd * col, 1.0), 0.5);
  } else if (vFragPosition.y < max_height * 0.4f) {
      col = vec3(44.f / 255.f, 176.f / 255.f, 55.f / 255.f);
      float kd = max(dot(vNormal, lightDir), 0.0);
      color = vec4(kd * col, 1.0f);
  } else if (vFragPosition.y < max_height * 0.65f) {
      col = vec3(64.f / 255.f, 103.f / 255.f, 13.f / 255.f);
      float kd = max(dot(vNormal, lightDir), 0.0);
      color = vec4(kd * col, 1.0f);
  } else if (vFragPosition.y < max_height * 0.8f) {
      col = vec3(138.f / 255.f, 128.f / 255.f, 124.f / 255.f);
      float kd = max(dot(vNormal, lightDir), 0.0);
      color = vec4(kd * col, 1.0f);
  } else {
      col = vec3(255.f / 255.f, 250.f / 255.f, 250.f / 255.f);
      float kd = max(dot(vNormal, lightDir), 0.0);
      color = vec4(kd * col, 1.0f);
  }
}