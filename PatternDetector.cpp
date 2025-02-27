#include "PatternDetector.hpp"
#include "DebugHelpers.hpp"

#include <cmath>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <cassert>
// Constructor de PatternDetector, inicializa los detectores, extractores y comparadores.
PatternDetector::PatternDetector(cv::Ptr<cv::FeatureDetector> detector, 
    cv::Ptr<cv::DescriptorExtractor> extractor, 
    cv::Ptr<cv::DescriptorMatcher> matcher, 
    bool ratioTest)
    : m_detector(detector)
    , m_extractor(extractor)
    , m_matcher(matcher)
    , enableRatioTest(ratioTest)
    , enableHomographyRefinement(true)
    , homographyReprojectionThreshold(3)
{
}

// Entrena el detector con un patr�n espec�fico.
void PatternDetector::train(const Pattern& pattern)
{
    m_pattern = pattern;

    m_matcher->clear();

    std::vector<cv::Mat> descriptors(1);
    descriptors[0] = pattern.descriptors.clone(); 
    m_matcher->add(descriptors);

    m_matcher->train();
}
// Construye un patr�n a partir de una imagen dada, definiendo puntos 2D y 3D.
void PatternDetector::buildPatternFromImage(const cv::Mat& image, Pattern& pattern) const
{
    int numImages = 4;
    float step = sqrtf(2.0f);

    pattern.size = cv::Size(image.cols, image.rows);
    pattern.frame = image.clone();
    getGray(image, pattern.grayImg);
    
    pattern.points2d.resize(4);
    pattern.points3d.resize(4);
    const float w = image.cols;
    const float h = image.rows;

    const float maxSize = std::max(w,h);
    const float unitW = w / maxSize;
    const float unitH = h / maxSize;

    pattern.points2d[0] = cv::Point2f(0,0);
    pattern.points2d[1] = cv::Point2f(w,0);
    pattern.points2d[2] = cv::Point2f(w,h);
    pattern.points2d[3] = cv::Point2f(0,h);

    pattern.points3d[0] = cv::Point3f(-unitW, -unitH, 0);
    pattern.points3d[1] = cv::Point3f( unitW, -unitH, 0);
    pattern.points3d[2] = cv::Point3f( unitW,  unitH, 0);
    pattern.points3d[3] = cv::Point3f(-unitW,  unitH, 0);

    extractFeatures(pattern.grayImg, pattern.keypoints, pattern.descriptors);
}

// Busca un patr�n en una imagen y refina los resultados usando homograf�a.
bool PatternDetector::findPattern(const cv::Mat& image, PatternTrackingInfo& info)
{
    getGray(image, m_grayImg);
    
    extractFeatures(m_grayImg, m_queryKeypoints, m_queryDescriptors);
    
    getMatches(m_queryDescriptors, m_matches);

#if _DEBUG
    cv::showAndSave("Raw matches", getMatchesImage(image, m_pattern.frame, m_queryKeypoints, m_pattern.keypoints, m_matches, 100));
#endif

#if _DEBUG
    cv::Mat tmp = image.clone();
#endif

    bool homographyFound = refineMatchesWithHomography(
        m_queryKeypoints, 
        m_pattern.keypoints, 
        homographyReprojectionThreshold, 
        m_matches, 
        m_roughHomography);

    if (homographyFound)
    {
#if _DEBUG
        cv::showAndSave("Refined matches using RANSAC", getMatchesImage(image, m_pattern.frame, m_queryKeypoints, m_pattern.keypoints, m_matches, 100));
#endif
        
        if (enableHomographyRefinement)
        {
            // Aplica una transformaci�n para mejorar la precisi�n de la homograf�a.
            cv::warpPerspective(m_grayImg, m_warpedImg, m_roughHomography, m_pattern.size, cv::WARP_INVERSE_MAP | cv::INTER_CUBIC);
#if _DEBUG
            cv::showAndSave("Warped image",m_warpedImg);
#endif
            std::vector<cv::KeyPoint> warpedKeypoints;
            std::vector<cv::DMatch> refinedMatches;

            extractFeatures(m_warpedImg, warpedKeypoints, m_queryDescriptors);

            getMatches(m_queryDescriptors, refinedMatches);
            // Refina los matches utilizando la homograf�a ajustada.
            homographyFound = refineMatchesWithHomography(
                warpedKeypoints, 
                m_pattern.keypoints, 
                homographyReprojectionThreshold, 
                refinedMatches, 
                m_refinedHomography);
#if _DEBUG
            cv::showAndSave("MatchesWithRefinedPose", getMatchesImage(m_warpedImg, m_pattern.grayImg, warpedKeypoints, m_pattern.keypoints, refinedMatches, 100));
#endif
            
            info.homography = m_roughHomography * m_refinedHomography;

            
#if _DEBUG
            cv::perspectiveTransform(m_pattern.points2d, info.points2d, m_roughHomography);
            info.draw2dContour(tmp, CV_RGB(0,200,0));
#endif

            
            cv::perspectiveTransform(m_pattern.points2d, info.points2d, info.homography);
#if _DEBUG
            info.draw2dContour(tmp, CV_RGB(200,0,0));
#endif
        }
        else
        {
            info.homography = m_roughHomography;

            
            cv::perspectiveTransform(m_pattern.points2d, info.points2d, m_roughHomography);
#if _DEBUG
            info.draw2dContour(tmp, CV_RGB(0,200,0));
#endif
        }
    }

#if _DEBUG
    if (1)
    {
        cv::showAndSave("Final matches", getMatchesImage(tmp, m_pattern.frame, m_queryKeypoints, m_pattern.keypoints, m_matches, 100));
    }
    std::cout << "Features:" << std::setw(4) << m_queryKeypoints.size() << " Matches: " << std::setw(4) << m_matches.size() << std::endl;
#endif

    return homographyFound;
}
// Convierte una imagen a escala de grises si es necesario.
void PatternDetector::getGray(const cv::Mat& image, cv::Mat& gray)
{
    if (image.channels()  == 3)
        cv::cvtColor(image, gray, CV_BGR2GRAY);
    else if (image.channels() == 4)
        cv::cvtColor(image, gray, CV_BGRA2GRAY);
    else if (image.channels() == 1)
        gray = image;
}
// Extrae caracter�sticas de una imagen en escala de grises.
bool PatternDetector::extractFeatures(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors) const
{
    assert(!image.empty());
    assert(image.channels() == 1);

    m_detector->detect(image, keypoints);
    if (keypoints.empty())
        return false;

    m_extractor->compute(image, keypoints, descriptors);
    if (keypoints.empty())
        return false;

    return true;
}
// Obtiene matches entre los descriptores de consulta y los descriptores del patr�n.
void PatternDetector::getMatches(const cv::Mat& queryDescriptors, std::vector<cv::DMatch>& matches)
{
    matches.clear();

    if (enableRatioTest)
    { 
        const float minRatio = 1.f / 1.5f;
        
        m_matcher->knnMatch(queryDescriptors, m_knnMatches, 2);

        for (size_t i=0; i<m_knnMatches.size(); i++)
        {
            const cv::DMatch& bestMatch   = m_knnMatches[i][0];
            const cv::DMatch& betterMatch = m_knnMatches[i][1];

            float distanceRatio = bestMatch.distance / betterMatch.distance;
            
            if (distanceRatio < minRatio)
            {
                matches.push_back(bestMatch);
            }
        }
    }
    else
    {
        m_matcher->match(queryDescriptors, matches);
    }
}
// Refina los matches usando una homograf�a calculada mediante RANSAC.
bool PatternDetector::refineMatchesWithHomography
    (
    const std::vector<cv::KeyPoint>& queryKeypoints,
    const std::vector<cv::KeyPoint>& trainKeypoints, 
    float reprojectionThreshold,
    std::vector<cv::DMatch>& matches,
    cv::Mat& homography
    )
{
    const int minNumberMatchesAllowed = 8;

    if (matches.size() < minNumberMatchesAllowed)
        return false;

    std::vector<cv::Point2f> srcPoints(matches.size());
    std::vector<cv::Point2f> dstPoints(matches.size());

    for (size_t i = 0; i < matches.size(); i++)
    {
        srcPoints[i] = trainKeypoints[matches[i].trainIdx].pt;
        dstPoints[i] = queryKeypoints[matches[i].queryIdx].pt;
    }

    std::vector<unsigned char> inliersMask(srcPoints.size());
    homography = cv::findHomography(srcPoints, 
                                    dstPoints, 
                                    CV_FM_RANSAC, 
                                    reprojectionThreshold, 
                                    inliersMask);

    std::vector<cv::DMatch> inliers;
    for (size_t i=0; i<inliersMask.size(); i++)
    {
        if (inliersMask[i])
            inliers.push_back(matches[i]);
    }

    matches.swap(inliers);
    return matches.size() > minNumberMatchesAllowed;
}
