clear
gcc graphics/*.c main.c \
	-lmingw32 -lglew32 -lopengl32 -lglu32 -lSDL2main -lSDL2 \
	-L build/lib \
	-I build/include \
	-std=c99 \
	-o build/bin/game.exe \
        -mwindows

echo "Compilation finished."
