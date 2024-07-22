#ifndef DEBUG_HELPERS_HPP
#define DEBUG_HELPERS_HPP

#include <string>
#include <sstream>

//Convierte cualquier tipo de dato a string. Ideal para debugging.
template <typename T>
std::string ToString(const T& value)
{
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

namespace cv
{
    //Muestra una imagen en una ventana y también la guarda como un archivo PNG
    inline void showAndSave(std::string name, const cv::Mat& m)
    {
        cv::imshow(name, m);
        cv::imwrite(name + ".png", m);
    }
    //dibuja las coincidencias entre dos imágenes utilizando los puntos clave y las coincidencias proporcionadas.
    inline cv::Mat getMatchesImage(cv::Mat query, cv::Mat pattern, const std::vector<cv::KeyPoint>& queryKp, const std::vector<cv::KeyPoint>& trainKp, std::vector<cv::DMatch> matches, int maxMatchesDrawn)
    {
        cv::Mat outImg;
        //limita el número de coincidencias dibujadas para evitar sobrecargar la imagen
        if (matches.size() > maxMatchesDrawn)
        {
            matches.resize(maxMatchesDrawn);
        }
        // Dibuja las coincidencias entre los puntos clave de las dos imágenes.
        cv::drawMatches
            (
            query, 
            queryKp, 
            pattern,  
            trainKp,
            matches, 
            outImg, 
            cv::Scalar(0,200,0,255), 
            cv::Scalar::all(-1),
            std::vector<char>(), 
            cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS
            );

        return outImg;
    }
}

#endif