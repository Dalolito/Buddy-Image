#ifndef FILE_IO_H
#define FILE_IO_H

#include "image_processor.h"
#include <string>

// Funciones para manipulación de archivos de imagen
namespace FileIO
{
    // Carga una imagen desde un archivo y la convierte al formato interno
    bool loadImage(const std::string &filename, ImageProcessor::Image &image);

    // Guarda una imagen procesada a un archivo
    bool saveImage(const std::string &filename, const ImageProcessor::Image &image);

    // Verifica si un archivo es una imagen válida
    bool isValidImageFile(const std::string &filename);
} // namespace FileIO

#endif // FILE_IO_H
