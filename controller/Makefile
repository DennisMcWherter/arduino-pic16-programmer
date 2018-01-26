all: clean build

clean:
	rm -rf programmer

build:
	g++ -std=c++17 -O2 -ggdb src/PICSerialProgrammer.cpp src/main.cpp -o programmer -lstdc++fs

run: build
	./programmer /dev/ttyACM0 /home/dennis/MPLABXProjects/ExamplePIC.X/dist/default/production/ExamplePIC.X.production.hex
