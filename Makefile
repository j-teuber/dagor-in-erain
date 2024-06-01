flags := -std=c++17 -Wall -Weffc++ -Wextra -Werror -pedantic-errors #-Wconversion -Wsign-conversion
debug_flags := -ggdb 
release_flags := -O3 -DNDEBUG
ld_flags :=

build_dir := ./build
obj_dir := $(build_dir)/objects
release_obj_dir := $(obj_dir)/release
debug_obj_dir := $(obj_dir)/debug
app_dir := $(build_dir)/app_dir

units := main bitboard print movetables
src_files := $(foreach u, $(units), $(u).cpp)
debug_objects := $(foreach u, $(units), $(debug_obj_dir)/$(u).o)
release_objects := $(foreach u, $(units), $(release_obj_dir)/$(u).o)

.PHONY: all run clean dirs docs

all: debug release docs

debug: $(app_dir)/debug

release: $(app_dir)/release

run: $(app_dir)/debug
	$(app_dir)/debug

dirs:
	mkdir -p $(release_obj_dir)
	mkdir -p $(debug_obj_dir)
	mkdir -p $(app_dir)

clean: 
	rm -rf $(build_dir)
	mkdir -p $(release_obj_dir)
	mkdir -p $(debug_obj_dir)
	mkdir -p $(app_dir)

docs:
	doxygen > /dev/null
	cd docs/latex && make pdf &> /dev/null && cd ../..

$(app_dir)/release: $(release_objects)
	g++ $(flags) $(release_flags) -o $@ $^

$(app_dir)/debug: $(debug_objects)
	g++ $(flags) $(debug_flags) -o $@ $^

$(release_objects): $(release_obj_dir)/%.o : %.cpp
	g++ $(flags) $(release_flags) -c -o $@ $^

$(debug_objects): $(debug_obj_dir)/%.o : %.cpp
	g++ $(flags) $(debug_flags) -c -o $@ $^

movetables.cpp: generate_movetables.cpp $(debug_obj_dir)/bitboard.o
	g++ $(flags) $(debug_flags) -c -o $(debug_obj_dir)/generate_movetables.o generate_movetables.cpp
	g++ $(flags) $(debug_flags) -o $(app_dir)/generate_movetables $(debug_obj_dir)/generate_movetables.o $(debug_obj_dir)/bitboard.o
	$(app_dir)/generate_movetables


