varying vec2 vTexcoord;

void main() {
  vTexcoord = gl_MultiTexCoord0.xy;
  gl_Position = ftransform();
}