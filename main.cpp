#include <iostream>
#include <string>
#include "image_processor.h"
#include "file_io.h"

void printUsage(const char* programName) {
    std::cout << "Uso: " << programName << " entrada.jpg salida.jpg [-angulo grados] [-escalar factor] [-buddy]" << std::endl;
    std::cout << "Parámetros:" << std::endl;
    std::cout << "  entrada.jpg: archivo de imagen de entrada" << std::endl;
    std::cout << "  salida.jpg: archivo donde se guarda la imagen procesada" << std::endl;
    std::cout << "  -angulo: define el ángulo de rotación (opcional)" << std::endl;
    std::cout << "  -escalar: define el factor de escalado (opcional)" << std::endl;
    std::cout << "  -buddy: activa el modo Buddy System (opcional)" << std::endl;
}

int main(int argc, char* argv[]) {
    // Verificar número mínimo de argumentos
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Obtener archivos de entrada y salida
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    
    // Por ahora, ignoramos los demás parámetros (se implementarán en partes posteriores)
    
    // Verificar si el archivo de entrada existe y es una imagen válida
    if (!FileIO::isValidImageFile(inputFile)) {
        std::cerr << "Error: El archivo " << inputFile << " no existe o no es una imagen válida." << std::endl;
        return 1;
    }
    
    // Crear objeto de imagen
    ImageProcessor::Image image;
    
    // Cargar la imagen
    std::cout << "Cargando imagen: " << inputFile << std::endl;
    if (!FileIO::loadImage(inputFile, image)) {
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
    
    // Por ahora, solo guardaremos la imagen sin procesar
    std::cout << "Guardando imagen en: " << outputFile << std::endl;
    if (!FileIO::saveImage(outputFile, image)) {
        std::cerr << "Error al guardar la imagen." << std::endl;
        return 1;
    }
    
    std::cout << "[INFO] Imagen guardada correctamente en " << outputFile << std::endl;
    
    return 0;
}