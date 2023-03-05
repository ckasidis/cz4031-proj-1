# CZ4031 Project 1
## how to run?
### Docker
**requirement: have docker installed on your system
- run the following commands
1. `docker build -t cz4031 .`
2. `docker run --rm cz4031`

### Windows
**requirement: have g++ available on your system
- run `.\run_main_windows.sh` or
- run the following commands
1. compile `g++ -o main src/*.cpp --std=c++17 main.cpp -I include`
2. run `./main.exe`
### Unix
**requirement: have g++ available on your system
- run `./run_main.sh` or 
- run the following commands 
1. compile `g++ -o main src/*.cpp --std=c++17 main.cpp -I include`
2. run `./main`
