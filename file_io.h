#ifndef FILE_IO_H
#define FILE_IO_H

#include "image_processor.h"
#include <string>
#include <functional>

// Funciones para manipulación de archivos de imagen
namespace FileIO
{
    // Carga una imagen desde un archivo y la convierte al formato interno
    bool loadImage(const std::string &filename, ImageProcessor::Image &image, bool useBuddySystem = false);

    // Guarda una imagen procesada a un archivo
    bool saveImage(const std::string &filename, const ImageProcessor::Image &image);

    // Verifica si un archivo es una imagen válida
    bool isValidImageFile(const std::string &filename);
    
    // Procesa una imagen grande en bloques con paralelización
    bool processBigImageInChunks(const std::string &inputFilename, const std::string &outputFilename, 
                             std::function<void(unsigned char*, int, int, int)> processor);
} // namespace FileIO

#endif // FILE_IO_H