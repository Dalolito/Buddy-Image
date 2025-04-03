#include "image_processor.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

namespace ImageProcessor
{

    Image::Image()
        : width(0),
          height(0),
          channels(0),
          pixels(nullptr),
          usingBuddySystem(false),
          buddySystem(nullptr),
          buddyBuffer(nullptr),
          totalBufferSize(0)
    {
    }

    Image::~Image()
    {
        freeMemory();
    }

    // Implementación del constructor de copia
    Image::Image(const Image &other)
        : width(0),
          height(0),
          channels(0),
          pixels(nullptr),
          usingBuddySystem(false),
          buddySystem(nullptr),
          buddyBuffer(nullptr),
          totalBufferSize(0)
    {
        width    = other.width;
        height   = other.height;
        channels = other.channels;
        usingBuddySystem = other.usingBuddySystem;

        if (width > 0 && height > 0 && channels > 0)
        {
            // Usar el mismo método de asignación de memoria que la imagen original
            allocateMemory(other.usingBuddySystem);

            // Copiar los datos de píxeles
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    for (int c = 0; c < channels; c++)
                    {
                        pixels[y][x][c] = other.pixels[y][x][c];
                    }
                }
            }
        }
    }

    // Implementación del operador de asignación
    Image &Image::operator=(const Image &other)
    {
        if (this != &other)
        {
            freeMemory();

            width    = other.width;
            height   = other.height;
            channels = other.channels;
            usingBuddySystem = other.usingBuddySystem;

            if (width > 0 && height > 0 && channels > 0)
            {
                // Usar el mismo método de asignación de memoria que la imagen original
                allocateMemory(other.usingBuddySystem);

                // Copiar los datos de píxeles
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        for (int c = 0; c < channels; c++)
                        {
                            pixels[y][x][c] = other.pixels[y][x][c];
                        }
                    }
                }
            }
        }
        return *this;
    }

    void Image::allocateMemory(bool useBuddySystem)
    {
        // Liberar memoria previa si existe
        freeMemory();
        
        // Establecer el flag de uso de Buddy System
        usingBuddySystem = useBuddySystem;
        
        // Calcular el tamaño total necesario
        totalBufferSize = (size_t)height * width * channels;
        
        // Tamaño estimado para punteros de la matriz
        size_t pointerSize = height * sizeof(unsigned char**) + height * width * sizeof(unsigned char*);
        
        if (usingBuddySystem)
        {
            std::cout << "Asignando memoria usando Buddy System (" << totalBufferSize << " bytes)..." << std::endl;
            
            // Crear el sistema Buddy con un tamaño apropiado para la imagen
            // El tamaño mínimo de bloque es 64 bytes por defecto
            size_t requiredMemory = totalBufferSize + pointerSize;
            size_t buddyPoolSize = requiredMemory * 2; // Dar margen extra
            
            buddySystem = new MemoryManagement::BuddySystem(buddyPoolSize);
            
            // Asignar memoria para la matriz de punteros
            pixels = (unsigned char***)buddySystem->allocate(height * sizeof(unsigned char**));
            
            for (int y = 0; y < height; y++)
            {
                pixels[y] = (unsigned char**)buddySystem->allocate(width * sizeof(unsigned char*));
            }
            
            // Asignar un gran bloque para todos los datos de píxeles
            buddyBuffer = (unsigned char*)buddySystem->allocate(totalBufferSize);
            
            // Configurar la matriz 3D para apuntar a las secciones correctas del buffer
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    pixels[y][x] = &buddyBuffer[(y * width + x) * channels];
                    
                    // Inicializar píxeles a 0
                    for (int c = 0; c < channels; c++)
                    {
                        pixels[y][x][c] = 0;
                    }
                }
            }
        }
        else
        {
            std::cout << "Asignando memoria convencional (" << totalBufferSize << " bytes)..." << std::endl;

            // Asignar memoria para la matriz tridimensional usando new explícitamente
            pixels = new unsigned char **[height];
            for (int y = 0; y < height; y++)
            {
                pixels[y] = new unsigned char *[width];
                for (int x = 0; x < width; x++)
                {
                    pixels[y][x] = new unsigned char[channels];

                    // Inicializar píxeles a 0
                    for (int c = 0; c < channels; c++)
                    {
                        pixels[y][x][c] = 0;
                    }
                }
            }
        }
    }

    void Image::freeMemory()
    {
        if (pixels)
        {
            if (usingBuddySystem)
            {
                if (buddySystem)
                {
                    // Liberar el buffer principal
                    if (buddyBuffer)
                    {
                        buddySystem->deallocate(buddyBuffer);
                        buddyBuffer = nullptr;
                    }
                    
                    // Liberar memoria para los punteros
                    for (int y = 0; y < height; y++)
                    {
                        buddySystem->deallocate((unsigned char*)pixels[y]);
                    }
                    
                    buddySystem->deallocate((unsigned char*)pixels);
                    
                    delete buddySystem;
                    buddySystem = nullptr;
                }
            }
            else
            {
                // Método convencional con new/delete
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        delete[] pixels[y][x];
                    }
                    delete[] pixels[y];
                }
                delete[] pixels;
            }
            
            pixels = nullptr;
        }
    }

    std::string Image::getInfo() const
    {
        std::stringstream ss;
        ss << "Dimensiones: " << width << " x " << height << std::endl;
        ss << "Canales: " << channels << " (";

        if (channels == 1)
        {
            ss << "Escala de grises";
        }
        else if (channels == 3)
        {
            ss << "RGB";
        }
        else if (channels == 4)
        {
            ss << "RGBA";
        }
        else
        {
            ss << "Desconocido";
        }

        ss << ")" << std::endl;
        ss << "Tamaño en memoria: " << (width * height * channels / 1024.0) << " KB" << std::endl;
        ss << "Método de asignación: " << (usingBuddySystem ? "Buddy System" : "Convencional");

        return ss.str();
    }

    std::string Image::getMemoryStats() const
    {
        std::stringstream ss;
        
        if (usingBuddySystem && buddySystem)
        {
            MemoryManagement::BuddySystem::MemoryStats stats = buddySystem->getStats();
            
            //ss << "Estadísticas Buddy System:" << std::endl;
           ss << "  Memoria total: " << stats.totalMemory << " bytes" << std::endl;
           // ss << "  Memoria usada: " << stats.usedMemory << " bytes" << std::endl;
            // ss << "  Memoria libre: " << stats.freeMemory << " bytes" << std::endl;
            //ss << "  Fragmentación: " << (stats.fragmentation * 100.0f) << "%" << std::endl;
        }
        
        return ss.str();
    }

    // Función optimizada para bilinearInterpolation (reemplazar en image_processor.cpp)
    unsigned char Image::bilinearInterpolation(float x, float y, int channel) {
    // Si las coordenadas están fuera de la imagen, devolver 0 (negro)
       if (x < 0 || y < 0 || x >= width - 1 || y >= height - 1) {
          return 0;
       }  

    // Obtener los índices de los píxeles vecinos
      int x1 = static_cast<int>(x);
      int y1 = static_cast<int>(y);
      int x2 = x1 + 1;
      int y2 = y1 + 1;

    // Pre-calcular todos los pesos de una vez
      float dx = x - x1;
      float dy = y - y1;
      float w1 = (1.0f - dx) * (1.0f - dy);
      float w2 = dx * (1.0f - dy);
      float w3 = (1.0f - dx) * dy;
      float w4 = dx * dy;

    // Acceso a memoria más eficiente
      unsigned char p1 = pixels[y1][x1][channel];
      unsigned char p2 = pixels[y1][x2][channel];
      unsigned char p3 = pixels[y2][x1][channel];
      unsigned char p4 = pixels[y2][x2][channel];

    // Cálculo más eficiente
      float value = w1 * p1 + w2 * p2 + w3 * p3 + w4 * p4;
    
      return static_cast<unsigned char>(std::max(0.0f, std::min(255.0f, value)));
    }

    void Image::rotateImage(float angleDegrees)
    {
        // Convertir ángulo de grados a radianes
        float angleRadians = angleDegrees * M_PI / 180.0f;

        // Crear una nueva imagen con las mismas dimensiones para almacenar el resultado
        Image rotatedImage;
        rotatedImage.width    = width;
        rotatedImage.height   = height;
        rotatedImage.channels = channels;
        rotatedImage.allocateMemory(usingBuddySystem); // Usar el mismo método de memoria

        // Calcular el centro de la imagen
        float centerX = width / 2.0f;
        float centerY = height / 2.0f;

        // Calcular la matriz de rotación inversa
        float cosAngle = cos(angleRadians);
        float sinAngle = sin(angleRadians);

        // Para cada píxel en la imagen de salida
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // Trasladar al origen (centro de la imagen)
                float xOffset = x - centerX;
                float yOffset = y - centerY;

                // Aplicar la rotación inversa
                float srcX = xOffset * cosAngle + yOffset * sinAngle + centerX;
                float srcY = -xOffset * sinAngle + yOffset * cosAngle + centerY;

                // Aplicar interpolación bilineal para cada canal
                for (int c = 0; c < channels; c++)
                {
                    rotatedImage.pixels[y][x][c] = bilinearInterpolation(srcX, srcY, c);
                }
            }
        }

        // Copiar la imagen rotada de vuelta a la imagen original
        *this = rotatedImage;

        std::cout << "[INFO] Imagen rotada " << angleDegrees << " grados." << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

    void Image::scaleImage(float factor)
    {
        // Calcular las nuevas dimensiones
        int newWidth  = static_cast<int>(width * factor);
        int newHeight = static_cast<int>(height * factor);

        // Crear una nueva imagen con las dimensiones escaladas
        Image scaledImage;
        scaledImage.width    = newWidth;
        scaledImage.height   = newHeight;
        scaledImage.channels = channels;
        scaledImage.allocateMemory(usingBuddySystem); // Usar el mismo método de memoria

        // Para cada píxel en la imagen de salida
        for (int y = 0; y < newHeight; y++)
        {
            for (int x = 0; x < newWidth; x++)
            {
                // Calcular las coordenadas en la imagen original
                float srcX = x / factor;
                float srcY = y / factor;

                // Aplicar interpolación bilineal para cada canal
                for (int c = 0; c < channels; c++)
                {
                    scaledImage.pixels[y][x][c] = bilinearInterpolation(srcX, srcY, c);
                }
            }
        }

        // Copiar la imagen escalada de vuelta a la imagen original
        *this = scaledImage;

        std::cout << "[INFO] Imagen escalada con factor " << factor << ". ";
        std::cout << "Nuevas dimensiones: " << newWidth << " x " << newHeight << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

} // namespace ImageProcessor