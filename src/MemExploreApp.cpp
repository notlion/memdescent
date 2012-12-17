#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Surface.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include "cinder/Matrix.h"
#include "cinder/Vector.h"
#include "cinder/Arcball.h"
#include <tr1/unordered_set>

using namespace ci;
using namespace ci::app;
using namespace std;

class MemExploreApp : public AppBasic {
public:
  void prepareSettings(Settings *settings);
  void setup();
  void update();
  void draw();
  void keyDown(KeyEvent e);
  void keyUp(KeyEvent e);
  void resize(ResizeEvent e);
private:
  int      mVolumeDim, mTilesDim;
  size_t   mDataLength;
  uint8_t *mDataPointer;
  
  Surface      mTestSurface;
  gl::Texture  mTexture;
  gl::GlslProg mProgram;
  gl::Fbo      mFbo;
  
  CameraPersp mCamera;
  Vec3f       mCameraVel, mCameraAcc;
  Arcball     mCameraArcball;
  
  tr1::unordered_set<char> mKeysDown;
};

void MemExploreApp::prepareSettings(Settings *settings)
{
  settings->setFullScreen();
  settings->setWindowSize(1440, 900);
}

void MemExploreApp::setup()
{
  mVolumeDim = 64; // Must have integer sqrt.
  mTilesDim = sqrtf(mVolumeDim);
  mDataLength = mVolumeDim * mVolumeDim * mVolumeDim * 4;
//  mDataPointer = (uint8_t*)malloc(mDataLength);
  mDataPointer = new uint8_t;
  
  try {
    mProgram = gl::GlslProg(loadResource("trace.vsh"), loadResource("trace.fsh"));
  }
  catch(gl::GlslProgCompileExc e) {
    console() << e.what() << endl;
  }
  
  int pixelSize = 4;
  mFbo = gl::Fbo(getWindowWidth() / pixelSize, getWindowHeight() / pixelSize);
  
  mCamera.setPerspective(60.0f, getWindowAspectRatio(), 0.01f, 100.0f);
  
  mCameraArcball.setCenter(getWindowCenter());
  mCameraArcball.setRadius(Vec2f(getWindowSize()).length() * 10.0f);
  
  hideCursor();
}

void MemExploreApp::resize(ResizeEvent e)
{
}

void MemExploreApp::keyDown(KeyEvent e)
{
  switch(e.getChar()) {
    case ' ':
      delete mDataPointer;
      mDataPointer = new uint8_t;
      break;
  }
  mKeysDown.insert(e.getChar());
}

void MemExploreApp::keyUp(KeyEvent e)
{
  mKeysDown.erase(e.getChar());
}

void MemExploreApp::update()
{
  mCameraArcball.resetQuat();
  mCameraArcball.mouseDown(getWindowCenter());
  mCameraArcball.mouseDrag(getWindowSize() - getMousePos());
  mCamera.setOrientation(mCameraArcball.getQuat() * mCamera.getOrientation());

  float speed = 0.01f;
  Vec3f camX = mCamera.getOrientation() * Vec3f::xAxis() * speed;
  Vec3f camY = mCamera.getOrientation() * Vec3f::yAxis() * speed;
  Vec3f camZ = mCamera.getOrientation() * Vec3f::zAxis() * speed;
  
  if(mKeysDown.count('w')) mCameraAcc -= camZ;
  if(mKeysDown.count('a')) mCameraAcc -= camX;
  if(mKeysDown.count('s')) mCameraAcc += camZ;
  if(mKeysDown.count('d')) mCameraAcc += camX;
  if(mKeysDown.count('q')) mCameraAcc += camY;
  if(mKeysDown.count('e')) mCameraAcc -= camY;

  mCameraVel += mCameraAcc;
  mCamera.setEyePoint(mCamera.getEyePoint() + mCameraVel);
  mCameraVel *= 0.975f;
  mCameraAcc *= 0.8f;
}

void MemExploreApp::draw()
{
  mTexture = gl::Texture(mDataPointer, GL_RGBA, mVolumeDim * mTilesDim, mVolumeDim * mTilesDim);
  mTexture.setWrap(GL_REPEAT, GL_REPEAT);
  mTexture.setMinFilter(GL_NEAREST);
  mTexture.setMagFilter(GL_NEAREST);
  
  float frustum[6];
  mCamera.getFrustum(&frustum[0], &frustum[1], &frustum[2], &frustum[3], &frustum[4], &frustum[5]);

  mFbo.bindFramebuffer();
  gl::setMatricesWindow(mFbo.getSize(), false);

  mProgram.bind();
  mProgram.uniform("uTexture", 0);
  mProgram.uniform("uVolumeDim", mVolumeDim);
  mProgram.uniform("uTilesDim", mTilesDim);
  mProgram.uniform("uTime", (float)getElapsedSeconds());
  mProgram.uniform("uEyePoint", mCamera.getEyePoint());
  mProgram.uniform("uXAxis", mCamera.getOrientation() * Vec3f::xAxis());
  mProgram.uniform("uYAxis", mCamera.getOrientation() * Vec3f::yAxis());
  mProgram.uniform("uViewDistance", mCamera.getAspectRatio() / abs(frustum[2] - frustum[0]) * mCamera.getNearClip());
  mProgram.uniform("uNegViewDir", -mCamera.getViewDirection().normalized());
  mProgram.uniform("uAspectRatio", mCamera.getAspectRatio());
  mTexture.enableAndBind();
  gl::drawSolidRect(mFbo.getBounds());
  mTexture.unbind();
  mProgram.unbind();
  
  mFbo.unbindFramebuffer();
  
  gl::setMatricesWindow(getWindowSize());
  gl::draw(mFbo.getTexture(), getWindowBounds());
}

CINDER_APP_BASIC(MemExploreApp, RendererGl)
