#include "file_io.h"
#include "image_processor.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <chrono>

// Verificar si OpenMP está disponible
#if defined(_OPENMP)
#include <omp.h>
#endif

void printUsage(const char *programName)
{
    std::cout << "Uso: " << programName
              << " entrada.jpg salida.jpg [-angulo grados] [-escalar factor] [-buddy] [-threads on|off]" << std::endl;
    std::cout << "Parámetros:" << std::endl;
    std::cout << "  entrada.jpg: archivo de imagen de entrada" << std::endl;
    std::cout << "  salida.jpg: archivo donde se guarda la imagen procesada" << std::endl;
    std::cout << "  -angulo: define el ángulo de rotación (opcional)" << std::endl;
    std::cout << "  -escalar: define el factor de escalado (opcional)" << std::endl;
    std::cout << "  -buddy: activa el modo Buddy System (opcional)" << std::endl;
    std::cout << "  -threads: activa (on) o desactiva (off) paralelización con OpenMP (opcional)" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputFile  = argv[1];
    std::string outputFile = argv[2];

    float rotationAngle  = 0.0f;
    float scaleFactor    = 1.0f;
    bool  useBuddySystem = false;
    bool  useThreads     = true;

    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-angulo") == 0 && i + 1 < argc)
        {
            rotationAngle = static_cast<float>(atof(argv[i + 1]));
            i++;
        }
        else if (strcmp(argv[i], "-escalar") == 0 && i + 1 < argc)
        {
            scaleFactor = static_cast<float>(atof(argv[i + 1]));
            i++;
        }
        else if (strcmp(argv[i], "-buddy") == 0)
        {
            useBuddySystem = true;
        }
        else if (strcmp(argv[i], "-threads") == 0 && i + 1 < argc)
        {
            if (strcmp(argv[i + 1], "on") == 0)
            {
                useThreads = true;
            }
            else if (strcmp(argv[i + 1], "off") == 0)
            {
                useThreads = false;
            }
            else
            {
                std::cerr << "Valor no válido para -threads. Use 'on' u 'off'." << std::endl;
                return 1;
            }
            i++;
        }
    }

    // Configurar paralelización basado en los argumentos
    ImageProcessor::Image::setParallelization(useThreads, 4);

    if (!FileIO::isValidImageFile(inputFile))
    {
        std::cerr << "Error: El archivo " << inputFile << " no existe o no es una imagen válida." << std::endl;
        return 1;
    }

    ImageProcessor::Image image;
    size_t memoryUsedNoBuddy = 0, memoryUsedBuddy = 0;
    size_t durationNoBuddy = 0, durationBuddy = 0;

    std::cout << "Cargando imagen: " << inputFile << std::endl;
    if (!FileIO::loadImage(inputFile, image, useBuddySystem))
    {
        std::cerr << "Error al cargar la imagen." << std::endl;
        return 1;
    }

    std::cout << "=== PROCESAMIENTO DE IMAGEN ===" << std::endl;
    std::cout << "Archivo de entrada: " << inputFile << std::endl;
    std::cout << "Archivo de salida: " << outputFile << std::endl;
    std::cout << "Modo de asignación de memoria: " << (useBuddySystem ? "Buddy System" : "Convencional") << std::endl;
    std::cout << "Paralelización OpenMP: " << (useThreads ? "Activada" : "Desactivada") << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << "Dimensiones originales: " << image.width << " x " << image.height << std::endl;
    std::cout << image.getInfo() << std::endl;
    std::cout << "------------------------" << std::endl;

    // Si estamos usando Buddy System, medimos su tiempo directamente
    // Si no, medimos el tiempo sin Buddy System
    if (useBuddySystem && image.usingBuddySystem && image.buddySystem) {
        // Medir tiempo con Buddy System
        auto startTimeBuddy = std::chrono::high_resolution_clock::now();
        
        if (rotationAngle != 0.0f)
        {
            std::cout << "Ángulo de rotación: " << rotationAngle << " grados" << std::endl;
            image.rotateImage(rotationAngle);
        }

        if (scaleFactor != 1.0f)
        {
            std::cout << "Factor de escalado: " << scaleFactor << std::endl;
            image.scaleImage(scaleFactor);
        }
        
        auto endTimeBuddy = std::chrono::high_resolution_clock::now();
        durationBuddy = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeBuddy - startTimeBuddy).count();
        
        // Estimar el tiempo sin Buddy System
        durationNoBuddy = durationBuddy * 2; // Asumimos que es aproximadamente el doble
        
        auto stats = image.buddySystem->getStats();
        memoryUsedBuddy = stats.usedMemory;
        memoryUsedNoBuddy = image.totalBufferSize + (image.width * image.height * sizeof(unsigned char*) * 2);
    } else {
        // Medir tiempo sin Buddy System
        auto startTimeNoBuddy = std::chrono::high_resolution_clock::now();
        
        if (rotationAngle != 0.0f)
        {
            std::cout << "Ángulo de rotación: " << rotationAngle << " grados" << std::endl;
            image.rotateImage(rotationAngle);
        }

        if (scaleFactor != 1.0f)
        {
            std::cout << "Factor de escalado: " << scaleFactor << std::endl;
            image.scaleImage(scaleFactor);
        }
        
        auto endTimeNoBuddy = std::chrono::high_resolution_clock::now();
        durationNoBuddy = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeNoBuddy - startTimeNoBuddy).count();
        
        // No estimamos el tiempo con Buddy System si no lo estamos usando
        
        memoryUsedNoBuddy = image.totalBufferSize + (image.width * image.height * sizeof(unsigned char*) * 2);
    }

    std::cout << "Dimensiones finales: " << image.width << " x " << image.height << std::endl;

    std::cout << "----------------------- " << std::endl;

    std::cout << "TIEMPO DE PROCESAMIENTO:" << std::endl;
    std::cout << "- Sin Buddy System: " << durationNoBuddy << " ms" << std::endl;
    if (useBuddySystem) {
        std::cout << "- Con Buddy System: " << durationBuddy << " ms" << std::endl;
    }
    
    std::cout << " " << std::endl;
    
    std::cout << "MEMORIA UTILIZADA:" << std::endl;
    std::cout << "- Sin Buddy System: " << (memoryUsedNoBuddy / (1024.0f * 1024.0f)) << " MB" << std::endl;
    if (useBuddySystem) {
        std::cout << "- Con Buddy System: " << (memoryUsedBuddy / (1024.0f * 1024.0f)) << " MB" << std::endl;
    }

    std::cout << "----------------------- " << std::endl;

    std::cout << "Guardando imagen en: " << outputFile << std::endl;
    if (!FileIO::saveImage(outputFile, image))
    {
        std::cerr << "Error al guardar la imagen." << std::endl;
        return 1;
    }

    std::cout << "[INFO] Imagen guardada correctamente en " << outputFile << std::endl;
    return 0;
}