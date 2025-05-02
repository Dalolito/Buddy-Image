#include "file_io.h"
#include <iostream>
#include <stdexcept>
#include <cstring>  // Para memcpy

// Necesitarás incluir alguna biblioteca para manipulación de imágenes
// Por ejemplo: stb_image.h (una biblioteca de cabecera única)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Deberás descargar este archivo
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // Deberás descargar este archivo también

// Incluir OpenMP para paralelización
#if defined(_OPENMP)
#include <omp.h>
#endif

namespace FileIO
{
    bool loadImage(const std::string &filename, ImageProcessor::Image &image, bool useBuddySystem)
    {
        int width, height, channels;

        // Carga la imagen usando stb_image
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            std::cerr << "Error cargando la imagen: " << filename << std::endl;
            std::cerr << "Motivo: " << stbi_failure_reason() << std::endl;
            return false;
        }

        // Reserva memoria para la imagen en nuestro formato
        image.width = width;
        image.height = height;
        image.channels = channels;
        image.allocateMemory(useBuddySystem); // Pasar el modo de asignación

        // Configurar número de hilos para OpenMP
        #if defined(_OPENMP)
        omp_set_num_threads(4);
        #endif

        // Copia los datos al formato de nuestra matriz tridimensional con paralelización
        #if defined(_OPENMP)
        #pragma omp parallel for collapse(2) schedule(dynamic, 16)
        #endif
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                for (int c = 0; c < channels; c++)
                {
                    image.pixels[y][x][c] = data[(y * width + x) * channels + c];
                }
            }
        }

        // Libera la memoria utilizada por stb_image
        stbi_image_free(data);

        return true;
    }

    bool saveImage(const std::string &filename, const ImageProcessor::Image &image)
    {
        // Convertir nuestra estructura de imagen al formato lineal esperado por stb_image_write
        unsigned char *data = new unsigned char[image.width * image.height * image.channels];

        // Configurar número de hilos para OpenMP
        #if defined(_OPENMP)
        omp_set_num_threads(4);
        #endif

        // Conversión paralela de la estructura de matriz 3D a buffer lineal
        #if defined(_OPENMP)
        #pragma omp parallel for collapse(2) schedule(dynamic, 16)
        #endif
        for (int y = 0; y < image.height; y++)
        {
            for (int x = 0; x < image.width; x++)
            {
                for (int c = 0; c < image.channels; c++)
                {
                    data[(y * image.width + x) * image.channels + c] = image.pixels[y][x][c];
                }
            }
        }

        // Determinar el formato de salida basado en la extensión del archivo
        bool success = false;
        std::string ext = filename.substr(filename.find_last_of(".") + 1);

        if (ext == "jpg" || ext == "jpeg")
        {
            success = stbi_write_jpg(filename.c_str(), image.width, image.height, image.channels, data, 90);
        }
        else if (ext == "png")
        {
            // Para imágenes PNG, ajustar la compresión para equilibrar velocidad y tamaño
            int prevLevel = stbi_write_png_compression_level;
            stbi_write_png_compression_level = 3; // Menor compresión, más rápido
            
            success = stbi_write_png(filename.c_str(), 
                                    image.width, 
                                    image.height, 
                                    image.channels, 
                                    data, 
                                    image.width * image.channels);
            
            // Restaurar el nivel de compresión predeterminado
            stbi_write_png_compression_level = prevLevel;
        }
        else
        {
            std::cerr << "Formato de archivo no soportado: " << ext << std::endl;
        }

        delete[] data;
        return success;
    }

    bool isValidImageFile(const std::string &filename)
    {
        FILE *f = fopen(filename.c_str(), "rb");
        if (!f)
        {
            return false;
        }

        // Verificar los primeros bytes para identificar formatos comunes
        unsigned char header[8];
        if (fread(header, 1, 8, f) != 8)
        {
            fclose(f);
            return false;
        }

        fclose(f);

        // Verificar formato JPG
        if (header[0] == 0xFF && header[1] == 0xD8)
        {
            return true;
        }

        // Verificar formato PNG
        if (header[0] == 0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G')
        {
            return true;
        }

        return false;
    }

    // Función para procesar imágenes grandes en bloques con paralelización
    bool processBigImageInChunks(const std::string &inputFilename, const std::string &outputFilename, 
                             std::function<void(unsigned char*, int, int, int)> processor)
    {
        // Abrir la imagen para obtener su información
        int width, height, channels;
        unsigned char *data = stbi_load(inputFilename.c_str(), &width, &height, &channels, 0);
        
        if (!data) {
            std::cerr << "Error cargando la imagen: " << inputFilename << std::endl;
            return false;
        }
        
        // Configurar OpenMP para usar 4 hilos
        #if defined(_OPENMP)
        omp_set_num_threads(4);
        #endif
        
        // Definir tamaño de bloque - optimizado para mejor rendimiento de caché
        const int CHUNK_HEIGHT = 128; // Procesar 128 filas a la vez
        
        // Crear buffer para un chunk
        unsigned char *chunk = new unsigned char[width * CHUNK_HEIGHT * channels];
        
        // Procesar la imagen en chunks
        for (int y = 0; y < height; y += CHUNK_HEIGHT) {
            int chunkHeight = std::min(CHUNK_HEIGHT, height - y);
            
            // Copiar chunk actual
            memcpy(chunk, data + (y * width * channels), width * chunkHeight * channels);
            
            // Procesar chunk en paralelo
            #if defined(_OPENMP)
            #pragma omp parallel for collapse(2) schedule(dynamic, 16)
            #endif
            for (int cy = 0; cy < chunkHeight; cy++) {
                for (int x = 0; x < width; x++) {
                    processor(chunk + (cy * width + x) * channels, x, y + cy, channels);
                }
            }
            
            // Copiar chunk procesado de vuelta
            memcpy(data + (y * width * channels), chunk, width * chunkHeight * channels);
        }
        
        // Guardar la imagen procesada
        bool success = false;
        std::string ext = outputFilename.substr(outputFilename.find_last_of(".") + 1);
        
        if (ext == "jpg" || ext == "jpeg") {
            success = stbi_write_jpg(outputFilename.c_str(), width, height, channels, data, 90);
        } else if (ext == "png") {
            // Usar menor compresión para mejor rendimiento
            int prevLevel = stbi_write_png_compression_level;
            stbi_write_png_compression_level = 3;
            
            success = stbi_write_png(outputFilename.c_str(), width, height, channels, data, width * channels);
            
            stbi_write_png_compression_level = prevLevel;
        } else {
            std::cerr << "Formato de archivo no soportado: " << ext << std::endl;
        }
        
        // Liberar memoria
        delete[] chunk;
        stbi_image_free(data);
        
        return success;
    }

} // namespace FileIO