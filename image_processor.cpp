#include "image_processor.h"
#include <sstream>
#include <iostream>

namespace ImageProcessor {

Image::Image() : width(0), height(0), channels(0), pixels(nullptr) {
}

Image::~Image() {
    freeMemory();
}

// Implementación del constructor de copia
Image::Image(const Image& other) : width(0), height(0), channels(0), pixels(nullptr) {
    width = other.width;
    height = other.height;
    channels = other.channels;
    
    if (width > 0 && height > 0 && channels > 0) {
        allocateMemory();
        
        // Copiar los datos de píxeles
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                for (int c = 0; c < channels; c++) {
                    pixels[y][x][c] = other.pixels[y][x][c];
                }
            }
        }
    }
}

// Implementación del operador de asignación
Image& Image::operator=(const Image& other) {
    if (this != &other) {
        freeMemory();
        
        width = other.width;
        height = other.height;
        channels = other.channels;
        
        if (width > 0 && height > 0 && channels > 0) {
            allocateMemory();
            
            // Copiar los datos de píxeles
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    for (int c = 0; c < channels; c++) {
                        pixels[y][x][c] = other.pixels[y][x][c];
                    }
                }
            }
        }
    }
    return *this;
}

void Image::allocateMemory() {
    // Liberar memoria previa si existe
    freeMemory();
    
    std::cout << "Asignando memoria usando new/delete convencional..." << std::endl;
    
    // Asignar memoria para la matriz tridimensional usando new explícitamente
    pixels = new unsigned char**[height];
    for (int y = 0; y < height; y++) {
        pixels[y] = new unsigned char*[width];
        for (int x = 0; x < width; x++) {
            pixels[y][x] = new unsigned char[channels];
            
            // Inicializar píxeles a 0
            for (int c = 0; c < channels; c++) {
                pixels[y][x][c] = 0;
            }
        }
    }
    
    std::cout << "Memoria asignada: " << (width * height * channels) << " bytes." << std::endl;
}

void Image::freeMemory() {
    if (pixels) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                delete[] pixels[y][x];  // Liberar memoria con delete[]
            }
            delete[] pixels[y];  // Liberar memoria con delete[]
        }
        delete[] pixels;  // Liberar memoria con delete[]
        pixels = nullptr;
    }
}

std::string Image::getInfo() const {
    std::stringstream ss;
    ss << "Dimensiones originales: " << width << " x " << height << std::endl;
    ss << "Canales: " << channels << " (";
    
    if (channels == 1) {
        ss << "Escala de grises";
    } else if (channels == 3) {
        ss << "RGB";
    } else if (channels == 4) {
        ss << "RGBA";
    } else {
        ss << "Desconocido";
    }
    
    ss << ")" << std::endl;
    ss << "Tamaño en memoria: " << (width * height * channels / 1024.0) << " KB";
    
    return ss.str();
}

} // namespace ImageProcessor