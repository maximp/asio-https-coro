all: build-debug build-release

build-debug: config-debug
	cd _debug && make

build-release: config-release
	cd _release && make

config-debug: _debug
	cd _debug && cmake -DCMAKE_BUILD_TYPE=Debug ..

config-release: _release
	cd _release && cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

_debug:
	mkdir _debug

_release:
	mkdir _release

