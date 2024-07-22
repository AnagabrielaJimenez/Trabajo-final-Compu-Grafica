#ifndef ARDrawingContext_HPP
#define ARDrawingContext_HPP
#include "GeometryTypes.hpp"
#include "CameraCalibration.hpp"
#include <opencv2/opencv.hpp>

void ARDrawingContextDrawCallback(void* param);
// Definición de la clase ARDrawingContext
class ARDrawingContext
{
public:
  ARDrawingContext(std::string windowName, cv::Size frameSize, const CameraCalibration& c);
  ~ARDrawingContext();

  
  bool                isPatternPresent;
  Transformation      patternPose;


  void updateBackground(const cv::Mat& frame);

  void updateWindow();

private:
    friend void ARDrawingContextDrawCallback(void* param);
    
    void draw();

 
  void drawCameraFrame();

  void drawAugmentedScene();

  void buildProjectionMatrix(const CameraCalibration& calibration, int w, int h, Matrix44& result);
  
  void drawCoordinateAxis();
  
  void drawCubeModel();

private:
  bool               m_isTextureInitialized;
  unsigned int       m_backgroundTextureId;
  CameraCalibration  m_calibration;
  cv::Mat            m_backgroundImage;
  std::string        m_windowName;
};

#endif