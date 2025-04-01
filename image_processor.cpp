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
          pixels(nullptr)
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
          pixels(nullptr)
    {
        width    = other.width;
        height   = other.height;
        channels = other.channels;

        if (width > 0 && height > 0 && channels > 0)
        {
            allocateMemory();

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

            if (width > 0 && height > 0 && channels > 0)
            {
                allocateMemory();

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

    void Image::allocateMemory()
    {
        // Liberar memoria previa si existe
        freeMemory();

        std::cout << "Asignando memoria usando new/delete convencional..." << std::endl;

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

        std::cout << "Memoria asignada: " << (width * height * channels) << " bytes." << std::endl;
    }

    void Image::freeMemory()
    {
        if (pixels)
        {
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    delete[] pixels[y][x]; // Liberar memoria con delete[]
                }
                delete[] pixels[y]; // Liberar memoria con delete[]
            }
            delete[] pixels; // Liberar memoria con delete[]
            pixels = nullptr;
        }
    }

    std::string Image::getInfo() const
    {
        std::stringstream ss;
        ss << "Dimensiones originales: " << width << " x " << height << std::endl;
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
        ss << "Tamaño en memoria: " << (width * height * channels / 1024.0) << " KB";

        return ss.str();
    }

    unsigned char Image::bilinearInterpolation(float x, float y, int channel)
    {
        // Si las coordenadas están fuera de la imagen, devolver 0 (negro)
        if (x < 0 || y < 0 || x >= width - 1 || y >= height - 1)
        {
            return 0;
        }

        // Obtener los índices de los píxeles vecinos
        int x1 = static_cast<int>(x);
        int y1 = static_cast<int>(y);
        int x2 = x1 + 1;
        int y2 = y1 + 1;

        // Calcular los pesos de interpolación
        float dx = x - x1;
        float dy = y - y1;

        // Realizar la interpolación bilineal
        float value = (1.0f - dx) * (1.0f - dy) * pixels[y1][x1][channel] +
                      dx * (1.0f - dy) * pixels[y1][x2][channel] +
                      (1.0f - dx) * dy * pixels[y2][x1][channel] +
                      dx * dy * pixels[y2][x2][channel];

        // Asegurar que el valor esté dentro del rango [0, 255]
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
        rotatedImage.allocateMemory();

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

        std::cout << "[INFO] Imagen rotada correctamente " << angleDegrees << " grados."
                  << std::endl;
    }

} // namespace ImageProcessor
