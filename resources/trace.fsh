uniform sampler2D uTexture;
uniform vec3 uEyePoint, uXAxis, uYAxis, uNegViewDir;
uniform float uViewDistance, uAspectRatio, uVolumeDim, uTilesDim, uTime;

varying vec2 vTexcoord;

const int kMaxSteps = 100;

vec4 getVoxelColor(vec3 pos) {
  vec2 uv = pos.xy / 64.0;
  uv.x += mod(pos.z, 8.0);
  uv.y += floor(pos.z / 8.0);
  uv /= 8.0;
  return texture2D(uTexture, uv);
}

void main() {
  float s = (vTexcoord.x - .5) * uAspectRatio;
	float t = (vTexcoord.y - .5);
  
  vec3 ori = uEyePoint;
  vec3 dir = normalize(uXAxis * s + uYAxis * t - (uNegViewDir * uViewDistance));
  
  vec3 pos = floor(ori);
  vec3 prev;
  
  vec3 step = sign(dir);
  vec3 tdelta = 1.0 / abs(dir);
  
  vec3 tmax = (vec3(
    step.x < 0.0 ? pos.x : pos.x + 1.0,
    step.y < 0.0 ? pos.y : pos.y + 1.0,
    step.z < 0.0 ? pos.z : pos.z + 1.0
  ) - ori) / dir;
  
  for(int i = 0; i < kMaxSteps; i++) {
    prev = pos;
    
    if(tmax.x < tmax.y) {
      if(tmax.x < tmax.z) {
        pos.x += step.x;
        tmax.x += tdelta.x;
      }
      else {
        pos.z += step.z;
        tmax.z += tdelta.z;
      }
    }
    else {
      if(tmax.y < tmax.z) {
        pos.y += step.y;
        tmax.y += tdelta.y;
      }
      else {
        pos.z += step.z;
        tmax.z += tdelta.z;
      }
    }
    
    vec4 color = getVoxelColor(pos);
    
    if(color.r > 0.0 || color.g > 0.0 || color.b > 0.0) {
      float fog = float(i) / float(kMaxSteps);
      color *= 1.0 - fog * fog;
      color.a = 1.0;
      gl_FragColor = color;
      return;
    }
  }
  
  gl_FragColor = vec4(0.0);
}