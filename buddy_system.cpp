#include "buddy_system.h"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <iomanip>
#include <cstring> // Para memcpy/memset

// Verificar si OpenMP está disponible
#if defined(_OPENMP)
#include <omp.h>
#endif

namespace MemoryManagement 
{
    BuddySystem::BuddySystem(size_t totalSize, size_t minBlockSize) 
        : minBlockSize(minBlockSize)
    {
        // Ajustar totalSize a la siguiente potencia de 2
        this->totalSize = 1;
        while (this->totalSize < totalSize) {
            this->totalSize <<= 1;
        }
        
        // Calcular el número de niveles
        levels = 0;
        size_t size = this->totalSize;
        while (size >= minBlockSize) {
            levels++;
            size >>= 1;
        }
        
        // Inicializar las listas de bloques libres
        freeBlocks.resize(levels);
        
        // Asignar el pool de memoria e inicializarlo a cero para mejor rendimiento
        memoryPool = new unsigned char[this->totalSize];
        memset(memoryPool, 0, this->totalSize);
        
        // Añadir el bloque completo como disponible
        freeBlocks[0].push_back(memoryPool);
    }
    
    BuddySystem::~BuddySystem() 
    {
        // Verificar fugas de memoria
        if (!allocatedBlocks.empty()) {
            std::cout << "[BUDDY] ADVERTENCIA: " << allocatedBlocks.size() 
                      << " bloques no fueron liberados" << std::endl;
        }
        
        // Liberar el pool de memoria
        delete[] memoryPool;
    }
    
    int BuddySystem::getLevel(size_t size) const 
    {
        // Encontrar el nivel adecuado (potencia de 2 más pequeña >= size)
        int level = 0;
        size_t levelSize = totalSize;
        
        while (levelSize > size && level < levels - 1) {
            levelSize >>= 1;
            level++;
        }
        
        return level;
    }
    
    size_t BuddySystem::getSizeFromLevel(int level) const 
    {
        return totalSize >> level;
    }
    
    unsigned char* BuddySystem::findBlock(size_t size) 
    {
        // Calcular el nivel para este tamaño
        int level = getLevel(size);
        
        // Buscar un bloque libre en este nivel
        if (!freeBlocks[level].empty()) {
            unsigned char* block = freeBlocks[level].back();
            freeBlocks[level].pop_back();
            return block;
        }
        
        // Si no hay bloques libres en este nivel, buscar en un nivel superior y dividir
        for (int i = level - 1; i >= 0; i--) {
            if (!freeBlocks[i].empty()) {
                unsigned char* block = freeBlocks[i].back();
                freeBlocks[i].pop_back();
                
                // Dividir bloques hasta llegar al nivel requerido
                for (int j = i; j < level; j++) {
                    splitBlock(block, j);
                    // El bloque derecho va a la lista de bloques libres
                    unsigned char* rightBlock = block + getSizeFromLevel(j + 1);
                    freeBlocks[j + 1].push_back(rightBlock);
                }
                
                return block;
            }
        }
        
        // No hay suficiente memoria disponible
        return nullptr;
    }
    
    void BuddySystem::splitBlock(unsigned char* block, int level) 
    {
        // No hacemos nada físicamente para dividir, solo calculamos las direcciones
    }
    
    unsigned char* BuddySystem::getBuddy(unsigned char* block, size_t size) const 
    {
        // Calcular la dirección del buddy
        size_t offset = block - memoryPool;
        size_t buddyOffset = offset ^ size; // XOR con el tamaño
        
        return memoryPool + buddyOffset;
    }
    
    bool BuddySystem::isValidBlockAddress(unsigned char* block, int level) const 
    {
        // Calcular el offset desde el inicio del pool
        size_t offset = block - memoryPool;
        size_t blockSize = getSizeFromLevel(level);
        
        // Verificar si es un múltiplo del tamaño de bloque y está dentro del rango
        return (offset % blockSize == 0) && (offset + blockSize <= totalSize);
    }
    
    void BuddySystem::mergeBlocks(unsigned char* block, int level) 
    {
        if (level <= 0) return; // No se puede fusionar el bloque de nivel superior
        
        size_t blockSize = getSizeFromLevel(level);
        unsigned char* buddy = getBuddy(block, blockSize);
        
        // Verificar si el buddy está libre
        auto buddyIt = std::find(freeBlocks[level].begin(), freeBlocks[level].end(), buddy);
        if (buddyIt == freeBlocks[level].end()) {
            return; // El buddy no está libre, no podemos fusionar
        }
        
        // Eliminar el buddy de la lista de bloques libres
        freeBlocks[level].erase(buddyIt);
        
        // Calcular el bloque fusionado (siempre es el que tiene la dirección menor)
        unsigned char* mergedBlock = (block < buddy) ? block : buddy;
        
        // Intentar fusionar recursivamente
        if (level > 0) {
            // Añadir el bloque fusionado al nivel superior
            freeBlocks[level - 1].push_back(mergedBlock);
            // Intentar fusionar en el nivel superior
            mergeBlocks(mergedBlock, level - 1);
        }
    }
    
    unsigned char* BuddySystem::allocate(size_t size) 
    {
        // Ajustar el tamaño para que sea al menos el mínimo
        size = std::max(size, minBlockSize);
        
        // Redondear al siguiente poder de 2
        size_t roundedSize = 1;
        while (roundedSize < size) {
            roundedSize <<= 1;
        }
        
        // Buscar un bloque adecuado
        unsigned char* block = findBlock(roundedSize);
        if (!block) {
            std::cerr << "[BUDDY] Error: No hay suficiente memoria para asignar " 
                      << size << " bytes" << std::endl;
            return nullptr;
        }
        
        // Registrar el bloque asignado
        int level = getLevel(roundedSize);
        size_t actualSize = getSizeFromLevel(level);
        allocatedBlocks[block] = actualSize;
        
        // Tocar la memoria para mejorar rendimiento de caché
        memset(block, 0, 64); // Tocar la primera línea de caché
        
        return block;
    }
    
    void BuddySystem::deallocate(unsigned char* ptr) 
    {
        // Verificar si el puntero es válido
        auto it = allocatedBlocks.find(ptr);
        if (it == allocatedBlocks.end()) {
            std::cerr << "[BUDDY] Error: Intento de liberar un puntero no asignado" << std::endl;
            return;
        }
        
        // Obtener el tamaño y calcular el nivel
        size_t size = it->second;
        int level = getLevel(size);
        
        // Eliminar del mapa de bloques asignados
        allocatedBlocks.erase(it);
        
        // Añadir a los bloques libres
        freeBlocks[level].push_back(ptr);
        
        // Intentar fusionar con su buddy
        mergeBlocks(ptr, level);
    }
    
    // Método para procesar bloques 2D de manera eficiente
    void BuddySystem::process2DBlock(unsigned char* buffer, int width, int height, int channels,
                                   std::function<void(unsigned char*, int, int, int)> processor) {
        const int BLOCK_SIZE = 64; // Ajustar según el tamaño de línea de caché
        
        #if defined(_OPENMP)
        #pragma omp parallel for collapse(2) schedule(dynamic)
        #endif
        for (int by = 0; by < height; by += BLOCK_SIZE) {
            for (int bx = 0; bx < width; bx += BLOCK_SIZE) {
                // Procesar un bloque
                int endY = std::min(by + BLOCK_SIZE, height);
                int endX = std::min(bx + BLOCK_SIZE, width);
                for (int y = by; y < endY; y++) {
                    for (int x = bx; x < endX; x++) {
                        processor(buffer + (y * width + x) * channels, x, y, channels);
                    }
                }
            }
        }
    }
    
    BuddySystem::MemoryStats BuddySystem::getStats() const 
    {
        MemoryStats stats;
        stats.totalMemory = totalSize;
        
        // Calcular memoria usada
        stats.usedMemory = 0;
        for (const auto& pair : allocatedBlocks) {
            stats.usedMemory += pair.second;
        }
        
        stats.freeMemory = totalSize - stats.usedMemory;
        
        // Calcular fragmentación: 1 - (mayor bloque libre / memoria libre total)
        size_t largestFreeBlock = 0;
        for (int i = 0; i < levels; i++) {
            if (!freeBlocks[i].empty()) {
                size_t blockSize = getSizeFromLevel(i);
                largestFreeBlock = std::max(largestFreeBlock, blockSize);
            }
        }
        
        if (stats.freeMemory > 0) {
            stats.fragmentation = 1.0f - (float)largestFreeBlock / stats.freeMemory;
        } else {
            stats.fragmentation = 0.0f;
        }
        
        return stats;
    }
}