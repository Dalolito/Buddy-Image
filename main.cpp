#include "file_io.h"
#include "image_processor.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

void printUsage(const char *programName)
{
    std::cout << "Uso: " << programName
              << " entrada.jpg salida.jpg [-angulo grados] [-escalar factor] [-buddy]" << std::endl;
    std::cout << "Parámetros:" << std::endl;
    std::cout << "  entrada.jpg: archivo de imagen de entrada" << std::endl;
    std::cout << "  salida.jpg: archivo donde se guarda la imagen procesada" << std::endl;
    std::cout << "  -angulo: define el ángulo de rotación (opcional)" << std::endl;
    std::cout << "  -escalar: define el factor de escalado (opcional)" << std::endl;
    std::cout << "  -buddy: activa el modo Buddy System (opcional)" << std::endl;
}

int main(int argc, char *argv[])
{
    // Verificar número mínimo de argumentos
    if (argc < 3)
    {
        printUsage(argv[0]);
        return 1;
    }

    // Obtener archivos de entrada y salida
    std::string inputFile  = argv[1];
    std::string outputFile = argv[2];

    // Configuración predeterminada
    float rotationAngle  = 0.0f;
    float scaleFactor    = 1.0f;
    bool  useBuddySystem = false;

    // Procesar parámetros opcionales
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-angulo") == 0 && i + 1 < argc)
        {
            rotationAngle = static_cast<float>(atof(argv[i + 1]));
            i++; // Saltar el siguiente argumento
        }
        else if (strcmp(argv[i], "-escalar") == 0 && i + 1 < argc)
        {
            scaleFactor = static_cast<float>(atof(argv[i + 1]));
            i++; // Saltar el siguiente argumento
        }
        else if (strcmp(argv[i], "-buddy") == 0)
        {
            useBuddySystem = true;
        }
    }

    // Verificar si el archivo de entrada existe y es una imagen válida
    if (!FileIO::isValidImageFile(inputFile))
    {
        std::cerr << "Error: El archivo " << inputFile << " no existe o no es una imagen válida."
                  << std::endl;
        return 1;
    }

    // Crear objeto de imagen
    ImageProcessor::Image image;

    // Cargar la imagen
    std::cout << "Cargando imagen: " << inputFile << std::endl;
    if (!FileIO::loadImage(inputFile, image))
    {
        std::cerr << "Error al cargar la imagen." << std::endl;
        return 1;
    }

    // Mostrar información de la imagen
    std::cout << "=== PROCESAMIENTO DE IMAGEN ===" << std::endl;
    std::cout << "Archivo de entrada: " << inputFile << std::endl;
    std::cout << "Archivo de salida: " << outputFile << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << image.getInfo() << std::endl;
    std::cout << "------------------------" << std::endl;

    // Aplicar rotación si se especificó
    if (rotationAngle != 0.0f)
    {
        std::cout << "Ángulo de rotación: " << rotationAngle << " grados" << std::endl;
        image.rotateImage(rotationAngle);
    }

    // Por ahora solo implementamos la rotación, el escalado lo dejaremos para la siguiente parte
    std::cout << "Guardando imagen en: " << outputFile << std::endl;
    if (!FileIO::saveImage(outputFile, image))
    {
        std::cerr << "Error al guardar la imagen." << std::endl;
        return 1;
    }

    std::cout << "[INFO] Imagen guardada correctamente en " << outputFile << std::endl;

    return 0;
}
