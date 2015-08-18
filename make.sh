gcc -std=c99 -o main main.c math4.c \
    -I /usr/local/Cellar/glfw3/3.1.1/include/ \
    -L /usr/local/Cellar/glfw3/3.1.1/lib/ -l glfw3 \
    -framework OpenGL && \
    ./main
