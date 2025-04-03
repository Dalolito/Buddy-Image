CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3 -march=native -fopenmp
LIBS = -lm -fopenmp

# Archivos fuente y objetos
SRCS = main.cpp image_processor.cpp file_io.cpp buddy_system.cpp
OBJS = $(SRCS:.cpp=.o)

# Nombre del ejecutable
TARGET = programa_imagen

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Regla para compilar archivos .cpp a .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Dependencias
main.o: main.cpp image_processor.h file_io.h buddy_system.h
image_processor.o: image_processor.cpp image_processor.h buddy_system.h
file_io.o: file_io.cpp file_io.h image_processor.h
buddy_system.o: buddy_system.cpp buddy_system.h

# Descargar stb_image si no existe
stb_image.h:
	wget https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

stb_image_write.h:
	wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Dependencia adicional para asegurar que las bibliotecas de imagen estén disponibles
file_io.o: stb_image.h stb_image_write.h

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean