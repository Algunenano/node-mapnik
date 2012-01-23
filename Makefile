all: mapnik.node

NPROCS:=1
OS:=$(shell uname -s)

ifeq ($(OS),Linux)
	NPROCS:=$(shell grep -c ^processor /proc/cpuinfo)
endif
ifeq ($(OS),Darwin)
	NPROCS:=$(shell sysctl -n hw.ncpu)
endif

gyp:
	python gen_settings.py
	python gyp/gyp build.gyp --depth=. -f make --generator-output=./projects/makefiles
	make -j$(NPROCS) -C ./projects/makefiles/ V=1
	cp projects/makefiles/out/Default/_mapnik.node lib/_mapnik.node

install: all
	@node-waf build install

mapnik.node:
	@node-waf build -j $(NPROCS)

clean:
	@node-waf clean distclean
	@rm -rf ./projects/makefiles/

uninstall:
	@node-waf uninstall

test-tmp:
	@rm -rf tests/tmp
	@mkdir -p tests/tmp

ifndef only
test: test-tmp
	@PATH=./node_modules/expresso/bin:${PATH} && NODE_PATH=./lib:$NODE_PATH expresso
else
test: test-tmp
	@PATH=./node_modules/expresso/bin:${PATH} && NODE_PATH=./lib:$NODE_PATH expresso test/${only}.test.js
endif

fix:
	@fixjsstyle lib/*js bin/*js test/*js examples/*/*.js examples/*/*/*.js

lint:
	@./node_modules/.bin/jshint lib/*js bin/*js test/*js examples/*/*.js examples/*/*/*.js


.PHONY: test lint fix gyp
