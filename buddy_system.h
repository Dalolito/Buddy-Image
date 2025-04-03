#ifndef BUDDY_SYSTEM_H
#define BUDDY_SYSTEM_H

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <functional>

namespace MemoryManagement 
{
    class BuddySystem 
    {
    private:
        // Tamaño mínimo de un bloque
        size_t minBlockSize;
        
        // Tamaño máximo del pool de memoria
        size_t totalSize;
        
        // Representación del pool de memoria
        unsigned char* memoryPool;
        
        // Lista de bloques libres por nivel (potencia de 2)
        std::vector<std::vector<unsigned char*>> freeBlocks;
        
        // Mapa para rastrear tamaños de bloques asignados
        std::unordered_map<unsigned char*, size_t> allocatedBlocks;
        
        // Número de niveles en el sistema (basado en min y total size)
        int levels;
        
        // Obtener el nivel para un tamaño dado
        int getLevel(size_t size) const;
        
        // Obtener el tamaño en bytes para un nivel dado
        size_t getSizeFromLevel(int level) const;
        
        // Encontrar un bloque adecuado, dividiendo si es necesario
        unsigned char* findBlock(size_t size);
        
        // Dividir un bloque en dos
        void splitBlock(unsigned char* block, int level);
        
        // Unir bloques hermanos si es posible
        void mergeBlocks(unsigned char* block, int level);
        
        // Obtener el bloque hermano
        unsigned char* getBuddy(unsigned char* block, size_t size) const;
        
        // Verificar si una dirección es un inicio válido de bloque para el nivel dado
        bool isValidBlockAddress(unsigned char* block, int level) const;
        
    public:
        // Constructor
        BuddySystem(size_t totalSize, size_t minBlockSize = 64);
        
        // Destructor
        ~BuddySystem();
        
        // Asignar memoria
        unsigned char* allocate(size_t size);
        
        // Liberar memoria
        void deallocate(unsigned char* ptr);
        
        // Método para procesar bloques 2D de manera eficiente
        void process2DBlock(unsigned char* buffer, int width, int height, int channels,
                           std::function<void(unsigned char*, int, int, int)> processor);
        
        // Obtener estadísticas de uso de memoria
        struct MemoryStats {
            size_t totalMemory;
            size_t usedMemory;
            size_t freeMemory;
            float fragmentation;
        };
        
        MemoryStats getStats() const;
        
        // Desactivar operaciones de copia
        BuddySystem(const BuddySystem&) = delete;
        BuddySystem& operator=(const BuddySystem&) = delete;
    };
}

#endif // BUDDY_SYSTEM_H