OUTPUT_DIRECTORY = bin

all:
	mkdir -p $(OUTPUT_DIRECTORY)
	gcc jcurses.c -o $(OUTPUT_DIRECTORY)/jcurses -lncurses
	kotlinc -include-runtime main.kt -d  $(OUTPUT_DIRECTORY)/main.jar

run:
	cd $(OUTPUT_DIRECTORY) && kotlin main.jar

run-native:
	cd $(OUTPUT_DIRECTORY) && ./main

native: all
	native-image --no-fallback -jar  $(OUTPUT_DIRECTORY)/main.jar -o $(OUTPUT_DIRECTORY)/main
