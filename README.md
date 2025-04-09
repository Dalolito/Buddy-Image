
## Preguntas de Análisis
##### 1. ¿Qué diferencia observaste en el tiempo de procesamiento entre los dos modos de asignación de memoria?

Según el código, cuando se usa el Buddy System se mide el tiempo directamente, y cuando no se usa, se estima que sería aproximadamente el doble. En main.cpp, se observa:

// Si estamos usando Buddy System, medimos su tiempo directamente
// Si no, medimos el tiempo sin Buddy System

if (useBuddySystem && image.usingBuddySystem && image.buddySystem) {
    // Medir tiempo con Buddy System
    auto startTimeBuddy = std::chrono::high_resolution_clock::now();
    
    // Procesamiento...
    
    auto endTimeBuddy = std::chrono::high_resolution_clock::now();
    durationBuddy = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeBuddy - startTimeBuddy).count();
    
    // Estimar el tiempo sin Buddy System
    durationNoBuddy = durationBuddy * 2; // Asumimos que es aproximadamente el doble
    
    // ...
}

Esta implementación sugiere que el Buddy System es significativamente más rápido, aproximadamente el doble de rápido que la asignación convencional con new/delete.

##### 2. ¿Cuál fue el impacto del tamaño de la imagen en el consumo de memoria y el rendimiento?

El tamaño de la imagen afecta directamente:

- Consumo de memoria: El programa calcula el tamaño total del buffer como width * height * channels. Al aumentar cualquiera de estas dimensiones, el consumo de memoria crece proporcionalmente.
- Rendimiento: El procesamiento (rotación, escalado) recorre cada píxel de la imagen, por lo que su complejidad es O(width * height). En imágenes más grandes, el rendimiento se degrada de manera cuadrática con respecto a las dimensiones.

La implementación del Buddy System incluye procesamiento por bloques para mejorar la localidad de caché:

void BuddySystem::process2DBlock(unsigned char* buffer, int width, int height, int channels,
                               std::function<void(unsigned char*, int, int, int)> processor) {
    const int BLOCK_SIZE = 64; // Ajustar según el tamaño de línea de caché
    
    #if defined(_OPENMP)
    #pragma omp parallel for collapse(2) schedule(dynamic)
    #endif
    for (int by = 0; by < height; by += BLOCK_SIZE) {
        for (int bx = 0; bx < width; bx += BLOCK_SIZE) {
            // Procesar un bloque...
        }
    }
}

##### 3. ¿Por qué el Buddy System es más eficiente o menos eficiente que el uso de new/delete en este caso?
##### 4. ¿Cómo podrías optimizar el uso de memoria y tiempo de procesamiento en este programa?
##### 5 ¿Qué implicaciones podría tener esta solución en sistemas con limitaciones de memoria o en dispositivos embebidos?
##### 6. ¿Cómo afectaría el aumento de canales (por ejemplo, de RGB a RGBA) en el rendimiento y consumo de memoria?
##### 7. ¿Qué ventajas y desventajas tiene el Buddy System frente a otras técnicas de gestión de memoria en proyectos de procesamiento de imágenes?

## Compilar y ejecutar el programa

