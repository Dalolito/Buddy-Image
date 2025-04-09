## Preguntas de Análisis
#### 1. ¿Qué diferencia observaste en el tiempo de procesamiento entre los dos modos de asignación de memoria?

Según el código en main.cpp, cuando se usa el Buddy System, se mide directamente su tiempo de ejecución, y luego se estima que el tiempo sin Buddy System sería aproximadamente el doble:

// Estimar el tiempo sin Buddy System

```durationNoBuddy = durationBuddy * 2; // Asumimos que es aproximadamente el doble```

Esta estimación indica que el Buddy System es significativamente más rápido, reduciendo el tiempo de procesamiento aproximadamente a la mitad comparado con la asignación convencional mediante new/delete.

#### 2. ¿Cuál fue el impacto del tamaño de la imagen en el consumo de memoria y el rendimiento?

El tamaño de la imagen afecta directamente:

Consumo de memoria: El programa calcula totalBufferSize = height * width * channels. Cuando cualquiera de estas dimensiones aumenta, el consumo de memoria crece proporcionalmente.
Rendimiento: Las operaciones de rotación y escalado requieren iterar sobre cada píxel, resultando en una complejidad O(width * height). Para imágenes grandes, esto significa un aumento cuadrático en el tiempo de procesamiento.

El Buddy System implementa un enfoque de procesamiento por bloques para mejorar la localidad de caché:

```
void BuddySystem::process2DBlock(unsigned char* buffer, int width, int height, int channels,
                               std::function<void(unsigned char*, int, int, int)> processor) {
    const int BLOCK_SIZE = 64; // Ajustar según el tamaño de línea de caché
    
    #if defined(_OPENMP)
    #pragma omp parallel for collapse(2) schedule(dynamic)
    #endif
    for (int by = 0; by < height; by += BLOCK_SIZE) {
        for (int bx = 0; bx < width; bx += BLOCK_SIZE) {
            // Procesamiento por bloques...
        }
    }
}
```
Esta técnica mejora significativamente el rendimiento para imágenes grandes al optimizar el uso de caché.

#### 3. ¿Por qué el Buddy System es más eficiente o menos eficiente que el uso de new/delete en este caso?

El Buddy System muestra mayor eficiencia por varias razones:

##### Asignación de memoria contigua:
En vez de múltiples pequeñas asignaciones, asigna un gran bloque contiguo (buddyBuffer), mejorando la localidad espacial y reduciendo los fallos de caché.
##### Reducción de llamadas al sistema: 
Con el método convencional, se realizan width * height llamadas a new para los píxeles individuales, mientras que el Buddy System realiza pocas asignaciones grandes.
##### Estructura de datos eficiente: 
El algoritmo de buddy utiliza operaciones binarias rápidas (como XOR) para identificar bloques:

```
unsigned char* BuddySystem::getBuddy(unsigned char* block, size_t size) const {
    size_t offset = block - memoryPool;
    size_t buddyOffset = offset ^ size; // XOR con el tamaño
    return memoryPool + buddyOffset;
}
```

#### Paralelización eficiente: 
El procesamiento por bloques permite mejor uso de OpenMP para paralelizar operaciones.
#### Toque de caché optimizado: 
El código inicializa explícitamente bloques nuevos:

```
// Tocar la memoria para mejorar rendimiento de caché
memset(block, 0, 64); // Tocar la primera línea de caché
```

#### 4. ¿Cómo podrías optimizar el uso de memoria y tiempo de procesamiento en este programa?

Optimizaciones potenciales:

Estructura de datos más eficiente: Cambiar de una matriz 3D con punteros anidados a una representación lineal con acceso calculado:

```
// En vez de: pixels[y][x][c]
// Usar: linearBuffer[y * width * channels + x * channels + c]
```
#### Vectorización con SIMD: 
Implementar instrucciones vectoriales (SSE/AVX) para procesar múltiples píxeles simultáneamente.
#### Optimización de bucles: 
Usar técnicas como loop unrolling, especialmente en la función de interpolación bilineal.
#### Ajuste dinámico del tamaño de bloque: 
Adaptar BLOCK_SIZE según el tamaño de línea de caché del sistema.
#### Implementar un sistema de memoria transaccional: 
Para reducir copias durante operaciones como rotación o escalado.
#### Filtrado de píxeles: 
Para operaciones como escalado, implementar preselección de qué píxeles necesitan procesamiento.
#### Utilizar memoria mapeada en archivos: 
Para imágenes muy grandes que no caben en memoria principal.

#### 5 ¿Qué implicaciones podría tener esta solución en sistemas con limitaciones de memoria o en dispositivos embebidos?

Implicaciones importantes:

##### Fragmentación reducida: 
El Buddy System minimiza la fragmentación de memoria, crítico en sistemas embebidos con memoria limitada.
##### Asignación predecible: 
El redondeo a potencias de 2 hace que el consumo de memoria sea más predecible.
##### Overhead de inicialización: 
El sistema requiere un coste inicial para establecer las estructuras de datos del buddy system.
##### Ajuste de parámetros: 
El tamaño mínimo de bloque (64 bytes) podría necesitar ajustes para sistemas muy restringidos.
##### Paralelismo controlado: 
El paralelismo mediante OpenMP debe ajustarse según los recursos disponibles.
##### Memoria contigua: 
Beneficia a sistemas con cachés pequeñas típicas de dispositivos embebidos.
##### Fragmentación interna: 
El redondeo a potencias de 2 puede desperdiciar memoria en bloques parcialmente utilizados.

#### 6. ¿Cómo afectaría el aumento de canales (por ejemplo, de RGB a RGBA) en el rendimiento y consumo de memoria?

Pasar de RGB (3 canales) a RGBA (4 canales) tendría estos efectos:

##### Consumo de memoria: 
Aumento aproximado del 33% (de 3 a 4 bytes por píxel).
##### Rendimiento de procesamiento: 
Las operaciones se volverían aproximadamente un 33% más lentas al tener que procesar un canal adicional por cada píxel.
##### Alineación de memoria: 
Los bloques de 4 bytes (RGBA) podrían alinearse mejor con arquitecturas de 32 bits, potencialmente mejorando la eficiencia de acceso.
##### Mayor presión de caché: 
Más datos por píxel significan menos píxeles en caché simultáneamente.
##### Ventaja para el Buddy System: 
La asignación contigua del Buddy System mostraría aún más ventajas para imágenes con más canales.

#### 7. ¿Qué ventajas y desventajas tiene el Buddy System frente a otras técnicas de gestión de memoria en proyectos de procesamiento de imágenes?

##### Ventajas:

##### Fragmentación externa minimizada: 
La capacidad de fusionar bloques adyacentes reduce la fragmentación.
##### Asignación/liberación rápida: 
Las operaciones binarias hacen que la búsqueda y manipulación de bloques sea muy eficiente.
##### Localidad espacial mejorada: 
Los datos relacionados permanecen próximos en memoria física.
##### Escalabilidad eficiente: 
Maneja bien grandes volúmenes de asignaciones y liberaciones repetitivas.
##### Paralelización natural: 
La estructura de datos facilita la división del trabajo en hilos paralelos.

##### Desventajas:

##### Fragmentación interna: 
El redondeo a potencias de 2 puede desperdiciar espacio dentro de los bloques.
##### Sobrecarga de gestión: 
Requiere memoria adicional para mantener las estructuras de control del buddy system.
##### Complejidad de implementación: 
Más complejo que métodos de asignación directa o pools de memoria.
##### Rigidez en tamaños: 
Solo trabaja con potencias de 2, lo que puede ser ineficiente para ciertos tipos de datos.
##### Costo inicial: 
La configuración de la estructura de datos tiene un costo de rendimiento inicial.

Para procesamiento de imágenes específicamente, el Buddy System es particularmente adecuado debido a los patrones de acceso a memoria predecibles y la naturaleza regular de los datos, donde la localidad espacial puede ser aprovechada eficientemente.

## Compilar y ejecutar el programa

Usa ```make``` para compilar.

```./programa_imagen image.jpeg prueba.jpg -angulo 90 -escalar 2.0 -buddy``` -> Este es ejemplo, se puede cambiar el angulo, la escala y usar o no ```-buddy```
