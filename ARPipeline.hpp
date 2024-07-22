#ifndef ARPIPELINE_HPP
#define ARPIPELINE_HPP

#include "PatternDetector.hpp"
#include "CameraCalibration.hpp"
#include "GeometryTypes.hpp"
// ARPipeline gestiona la detección de patrones y calcula su pose en el espacio 3D utilizando una imagen de patrón 
// y una calibración de cámara.

class ARPipeline
{
public:
  ARPipeline(const cv::Mat& patternImage, const CameraCalibration& calibration);

  bool processFrame(const cv::Mat& inputFrame);

  const Transformation& getPatternLocation() const;

  PatternDetector     m_patternDetector;
private:

private:
  CameraCalibration   m_calibration;
  Pattern             m_pattern;
  PatternTrackingInfo m_patternInfo;
};

#endif