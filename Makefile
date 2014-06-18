all: cmake casmi

cmake:
	mkdir build || cmake . build

casmi: cmake
	make -C build casmi

clean:
	make -C build clean

check: cmake
	make -C build check
