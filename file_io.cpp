#include "file_io.h"
#include <iostream>
#include <stdexcept>

// Necesitarás incluir alguna biblioteca para manipulación de imágenes
// Por ejemplo: stb_image.h (una biblioteca de cabecera única)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Deberás descargar este archivo
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // Deberás descargar este archivo también

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
        image.width    = width;
        image.height   = height;
        image.channels = channels;
        image.allocateMemory(useBuddySystem);  // Pasar el modo de asignación

        // Copia los datos al formato de nuestra matriz tridimensional
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
        bool        success = false;
        std::string ext     = filename.substr(filename.find_last_of(".") + 1);

        if (ext == "jpg" || ext == "jpeg")
        {
            success = stbi_write_jpg(
                filename.c_str(), image.width, image.height, image.channels, data, 90);
        }
        else if (ext == "png")
        {
            success = stbi_write_png(filename.c_str(),
                                     image.width,
                                     image.height,
                                     image.channels,
                                     data,
                                     image.width * image.channels);
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

} // namespace FileIO