FROM gcc:12.2

WORKDIR /app

COPY src ./src
COPY include ./include
COPY data ./data
COPY main.cpp ./

RUN g++ -o main src/*.cpp --std=c++17 main.cpp -I include 

CMD ["./main"]
