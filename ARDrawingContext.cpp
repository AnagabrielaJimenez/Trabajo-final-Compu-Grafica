#include "ARDrawingContext.hpp"
#include <gl/gl.h>
#include <gl/glu.h>
// Funci�n de callback para el dibujo en OpenGL
void ARDrawingContextDrawCallback(void* param)
{
    ARDrawingContext * ctx = static_cast<ARDrawingContext*>(param);
    if (ctx)
    {
        ctx->draw();
    }
}
// Constructor de ARDrawingContext
ARDrawingContext::ARDrawingContext(std::string windowName, cv::Size frameSize, const CameraCalibration& c)
  : m_isTextureInitialized(false)
  , m_calibration(c)
  , m_windowName(windowName)
{
    cv::namedWindow(windowName, cv::WINDOW_OPENGL);

    cv::resizeWindow(windowName, frameSize.width, frameSize.height);

    cv::setOpenGlContext(windowName);
    cv::setOpenGlDrawCallback(windowName, ARDrawingContextDrawCallback, this);
}
// Destructor de ARDrawingContext
ARDrawingContext::~ARDrawingContext()
{
    cv::setOpenGlDrawCallback(m_windowName, 0, 0);
}

void ARDrawingContext::updateBackground(const cv::Mat& frame)
{
  frame.copyTo(m_backgroundImage);
}

void ARDrawingContext::updateWindow()
{
    cv::updateWindow(m_windowName);
}
// M�todo principal de dibujo que limpia el buffer y dibuja el fondo y la escena aumentada.
void ARDrawingContext::draw()
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 
  drawCameraFrame();                                  
  drawAugmentedScene();                               
  glFlush();
}
// Dibuja el frame de la c�mara en la ventana OpenGL.
void ARDrawingContext::drawCameraFrame()
{
  if (!m_isTextureInitialized)
  {
    glGenTextures(1, &m_backgroundTextureId);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_isTextureInitialized = true;
  }

  int w = m_backgroundImage.cols;
  int h = m_backgroundImage.rows;

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);
  // Configura la textura en funci�n del n�mero de canales.
  if (m_backgroundImage.channels() == 3)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, m_backgroundImage.data);
  else if(m_backgroundImage.channels() == 4)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_backgroundImage.data);
  else if (m_backgroundImage.channels()==1)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_backgroundImage.data);

  const GLfloat bgTextureVertices[] = { 0, 0, w, 0, 0, h, w, h };
  const GLfloat bgTextureCoords[]   = { 1, 0, 1, 1, 0, 0, 0, 1 };
  const GLfloat proj[]              = { 0, -2.f/w, 0, 0, -2.f/h, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1 };

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(2, GL_FLOAT, 0, bgTextureVertices);
  glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);

  glColor4f(1,1,1,1);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_TEXTURE_2D);
}
// Dibuja la escena aumentada si se detecta un patr�n.
void ARDrawingContext::drawAugmentedScene()
{
  Matrix44 projectionMatrix;
  int w = m_backgroundImage.cols;
  int h = m_backgroundImage.rows;
  buildProjectionMatrix(m_calibration, w, h, projectionMatrix);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projectionMatrix.data);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (isPatternPresent)
  {
    Matrix44 glMatrix = patternPose.getMat44();
    glLoadMatrixf(reinterpret_cast<const GLfloat*>(&glMatrix.data[0]));

    drawCoordinateAxis();
    drawCubeModel();
  }
}
// Construye la matriz de proyecci�n en funci�n de la calibraci�n de la c�mara y el tama�o de la pantalla.
void ARDrawingContext::buildProjectionMatrix(const CameraCalibration& calibration, int screen_width, int screen_height, Matrix44& projectionMatrix)
{
  float nearPlane = 0.01f;  
  float farPlane  = 100.0f;  

  float f_x = calibration.fx(); 
  float f_y = calibration.fy(); 
  float c_x = calibration.cx(); 
  float c_y = calibration.cy(); 

  projectionMatrix.data[0] = -2.0f * f_x / screen_width;
  projectionMatrix.data[1] = 0.0f;
  projectionMatrix.data[2] = 0.0f;
  projectionMatrix.data[3] = 0.0f;

  projectionMatrix.data[4] = 0.0f;
  projectionMatrix.data[5] = 2.0f * f_y / screen_height;
  projectionMatrix.data[6] = 0.0f;
  projectionMatrix.data[7] = 0.0f;

  projectionMatrix.data[8] = 2.0f * c_x / screen_width - 1.0f;
  projectionMatrix.data[9] = 2.0f * c_y / screen_height - 1.0f;    
  projectionMatrix.data[10] = -( farPlane + nearPlane) / ( farPlane - nearPlane );
  projectionMatrix.data[11] = -1.0f;

  projectionMatrix.data[12] = 0.0f;
  projectionMatrix.data[13] = 0.0f;
  projectionMatrix.data[14] = -2.0f * farPlane * nearPlane / ( farPlane - nearPlane );        
  projectionMatrix.data[15] = 0.0f;
}

// Dibuja un eje de coordenadas RGB.
void ARDrawingContext::drawCoordinateAxis()
{
  static float lineX[] = {0,0,0,1,0,0};
  static float lineY[] = {0,0,0,0,1,0};
  static float lineZ[] = {0,0,0,0,0,1};

  glLineWidth(2);

  glBegin(GL_LINES);

  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3fv(lineX);
  glVertex3fv(lineX + 3);

  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3fv(lineY);
  glVertex3fv(lineY + 3);

  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3fv(lineZ);
  glVertex3fv(lineZ + 3);

  glEnd();
}

void ARDrawingContext::drawCubeModel()
{
  static const GLfloat LightAmbient[]=  { 0.25f, 0.25f, 0.25f, 1.0f };   
  static const GLfloat LightDiffuse[]=  { 0.1f, 0.1f, 0.1f, 1.0f };    
  static const GLfloat LightPosition[]= { 0.0f, 0.0f, 2.0f, 1.0f };    
  
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT);

  glColor4f(0.2f,0.35f,0.3f,0.75f);        
  glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);     
  glEnable(GL_BLEND); 

  glShadeModel(GL_SMOOTH);

  glEnable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
  glEnable(GL_COLOR_MATERIAL);

  glScalef(0.25,0.25, 0.25);
  glTranslatef(0,0, 1);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glBegin(GL_QUADS); 
  glNormal3f( 0.0f, 0.0f, 1.0f);    
  glVertex3f(-1.0f, -1.0f,  1.0f);  
  glVertex3f( 1.0f, -1.0f,  1.0f);  
  glVertex3f( 1.0f,  1.0f,  1.0f);  
  glVertex3f(-1.0f,  1.0f,  1.0f);  
  
  glNormal3f( 0.0f, 0.0f,-1.0f);   
  glVertex3f(-1.0f, -1.0f, -1.0f);  
  glVertex3f(-1.0f,  1.0f, -1.0f);  
  glVertex3f( 1.0f,  1.0f, -1.0f);  
  glVertex3f( 1.0f, -1.0f, -1.0f);  
  
  glNormal3f( 0.0f, 1.0f, 0.0f);    
  glVertex3f(-1.0f,  1.0f, -1.0f);  
  glVertex3f(-1.0f,  1.0f,  1.0f);  
  glVertex3f( 1.0f,  1.0f,  1.0f);  
  glVertex3f( 1.0f,  1.0f, -1.0f);  
  
  glNormal3f( 0.0f,-1.0f, 0.0f);    
  glVertex3f(-1.0f, -1.0f, -1.0f);  
  glVertex3f( 1.0f, -1.0f, -1.0f);  
  glVertex3f( 1.0f, -1.0f,  1.0f);  
  glVertex3f(-1.0f, -1.0f,  1.0f);  
  
  glNormal3f( 1.0f, 0.0f, 0.0f);    
  glVertex3f( 1.0f, -1.0f, -1.0f); 
  glVertex3f( 1.0f,  1.0f, -1.0f);  
  glVertex3f( 1.0f,  1.0f,  1.0f);  
  glVertex3f( 1.0f, -1.0f,  1.0f);  
  
  glNormal3f(-1.0f, 0.0f, 0.0f);    
  glVertex3f(-1.0f, -1.0f, -1.0f);  
  glVertex3f(-1.0f, -1.0f,  1.0f);  
  glVertex3f(-1.0f,  1.0f,  1.0f);  
  glVertex3f(-1.0f,  1.0f, -1.0f); 
  glEnd();
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glColor4f(0.2f,0.65f,0.3f,0.35f); 
  glBegin(GL_QUADS); 
 
  glNormal3f( 0.0f, 0.0f, 1.0f);    
  glVertex3f(-1.0f, -1.0f,  1.0f);  
  glVertex3f( 1.0f, -1.0f,  1.0f);  
  glVertex3f( 1.0f,  1.0f,  1.0f);  
  glVertex3f(-1.0f,  1.0f,  1.0f);  
  
  glNormal3f( 0.0f, 0.0f,-1.0f);    
  glVertex3f(-1.0f, -1.0f, -1.0f);  
  glVertex3f(-1.0f,  1.0f, -1.0f);  
  glVertex3f( 1.0f,  1.0f, -1.0f);  
  glVertex3f( 1.0f, -1.0f, -1.0f);  
  
  glNormal3f( 0.0f, 1.0f, 0.0f);    
  glVertex3f(-1.0f,  1.0f, -1.0f);  
  glVertex3f(-1.0f,  1.0f,  1.0f);  
  glVertex3f( 1.0f,  1.0f,  1.0f);  
  glVertex3f( 1.0f,  1.0f, -1.0f);  
  
  glNormal3f( 0.0f,-1.0f, 0.0f);    
  glVertex3f(-1.0f, -1.0f, -1.0f);  
  glVertex3f( 1.0f, -1.0f, -1.0f);  
  glVertex3f( 1.0f, -1.0f,  1.0f);  
  glVertex3f(-1.0f, -1.0f,  1.0f);  
  
  glNormal3f( 1.0f, 0.0f, 0.0f);    
  glVertex3f( 1.0f, -1.0f, -1.0f);  
  glVertex3f( 1.0f,  1.0f, -1.0f);  
  glVertex3f( 1.0f,  1.0f,  1.0f);  
  glVertex3f( 1.0f, -1.0f,  1.0f);  
  
  glNormal3f(-1.0f, 0.0f, 0.0f);    
  glVertex3f(-1.0f, -1.0f, -1.0f);  
  glVertex3f(-1.0f, -1.0f,  1.0f);  
  glVertex3f(-1.0f,  1.0f,  1.0f);  
  glVertex3f(-1.0f,  1.0f, -1.0f);  
  glEnd();
 
  glPopAttrib();
}
