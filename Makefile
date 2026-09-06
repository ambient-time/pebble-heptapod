.PHONY: test build icon package
PEBBLE ?= pebble
PYTHON ?= python3
TEST_FLAGS = -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer

test:
	mkdir -p build
	cc $(TEST_FLAGS) test/geometry.c -lm -o build/geometry-test
	build/geometry-test
	cc $(TEST_FLAGS) -DHP_TEST_FIXED_TRIG test/geometry.c -lm -o build/fixed-geometry-test
	build/fixed-geometry-test

build: test
	$(PEBBLE) build
	$(PYTHON) test/package.py

package:
	$(PYTHON) test/package.py --evidence build/evidence

icon:
	mkdir -p build
	cc -std=c11 -Wall -Wextra -Werror test/icon.c -lm -o build/icon-render
	build/icon-render resources/images/menu-icon.svg
	magick -background white resources/images/menu-icon.svg -resize 25x25 -threshold 50% -define png:exclude-chunks=date,time resources/images/menu-icon.png
