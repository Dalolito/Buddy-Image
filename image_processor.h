#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <string>

namespace ImageProcessor {

    // Estructura para representar una imagen como matriz tridimensional
    class Image {
    public:
        int width;
        int height;
        int channels;
        
        // Matriz tridimensional para los píxeles
        unsigned char*** pixels;
        
        Image();
        ~Image();
        
        // Constructor de copia y operador de asignación (prevenir fugas de memoria)
        Image(const Image& other);
        Image& operator=(const Image& other);
        
        // Asigna memoria para la matriz tridimensional de píxeles usando new
        void allocateMemory();
        
        // Libera la memoria asignada usando delete
        void freeMemory();
        
        // Devuelve información sobre la imagen
        std::string getInfo() const;
    };

} // namespace ImageProcessor

#endif // IMAGE_PROCESSOR_H