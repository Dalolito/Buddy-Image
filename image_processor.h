#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <string>
#include "buddy_system.h"

namespace ImageProcessor
{
    // Estructura para representar una imagen como matriz tridimensional
    class Image
    {
        public:
            int width;
            int height;
            int channels;

            // Matriz tridimensional para los píxeles
            unsigned char ***pixels;
            
            // Flag para indicar si se está usando Buddy System
            bool usingBuddySystem;
            
            // Sistema Buddy para gestión de memoria
            MemoryManagement::BuddySystem* buddySystem;
            
            // Buffer utilizado cuando se usa Buddy System
            unsigned char* buddyBuffer;
            
            // Tamaño total del buffer
            size_t totalBufferSize;

            Image();
            ~Image();

            // Constructor de copia y operador de asignación (prevenir fugas de memoria)
            Image(const Image &other);
            Image &operator=(const Image &other);

            // Asigna memoria para la matriz tridimensional de píxeles usando el método seleccionado
            void allocateMemory(bool useBuddySystem = false);

            // Libera la memoria asignada
            void freeMemory();

            // Devuelve información sobre la imagen
            std::string getInfo() const;

            // Rota la imagen alrededor de su centro por un ángulo dado en grados
            void rotateImage(float angleDegrees);

            // Escala la imagen por un factor dado
            void scaleImage(float factor);

            // Método auxiliar para la interpolación bilineal
            unsigned char bilinearInterpolation(float x, float y, int channel);
            
            // Obtener estadísticas de memoria del Buddy System
            std::string getMemoryStats() const;
    };

} // namespace ImageProcessor

#endif // IMAGE_PROCESSOR_H