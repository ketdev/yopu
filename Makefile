# Compiler
desktop: 	CC = g++
web: 		CC = emcc

# Compiler flags
# 	-w suppresses all warnings
# 	-Wl,-subsystem,windows gets rid of the console window
COMMON_FLAGS = -O0 --std=c++2a
desktop: 	FLAGS = $(COMMON_FLAGS) -g -w -Wl,-subsystem,console
web: 		FLAGS = $(COMMON_FLAGS) --preload-file src/assets

OBJS = src\main.cpp src\engine\app.cpp src\game\game.cpp

INCLUDE_DIRS = -IC:\ket\dev\libs\SDL2-2.0.12\i686-w64-mingw32\include -IC:\ket\dev\libs\entt\single_include -IC:\ket\dev\libs\glm

LIBRARY_DIRS = -LC:\ket\dev\libs\SDL2-2.0.12\i686-w64-mingw32\lib -LC:\ket\dev\libs\SDL2_image-2.0.5\i686-w64-mingw32\lib -LC:\ket\dev\libs\SDL2_mixer-2.0.4\i686-w64-mingw32\lib

# The libraries we're linking against
desktop: 	LINKER_LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer
web: 		LINKER_LIBS = -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2; -s SDL2_IMAGE_FORMATS='["png"]' 

# Target output 
desktop: 	OUTPUT = dist\puyo.exe
web: 		OUTPUT = dist\puyo.js

# Define build targets

desktop: all
web: all

all: $(OBJS)
	$(CC) $(FLAGS) $(OBJS) $(INCLUDE_DIRS) $(LIBRARY_DIRS) $(LINKER_LIBS) -o $(OUTPUT)
