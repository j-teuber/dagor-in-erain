flags := -std=c++17 -Wall -Weffc++ -Wextra -Werror -pedantic-errors #-Wconversion -Wsign-conversion
debug_flags := -ggdb 
release_flags := -O3 -DNDEBUG
ld_flags :=

src := ./src
build_dir := ./build
obj_dir := $(build_dir)/objects
release_obj_dir := $(obj_dir)/release
debug_obj_dir := $(obj_dir)/debug
app_dir := $(build_dir)/app_dir

units := main bitboard movetables game_state uci test
src_files := $(foreach u, $(units), $(src)/$(u).cpp)
debug_objects := $(foreach u, $(units), $(debug_obj_dir)/$(u).o)
release_objects := $(foreach u, $(units), $(release_obj_dir)/$(u).o)

.PHONY: all run clean dirs docs test

all: debug release docs

debug: $(app_dir)/debug

release: $(app_dir)/release

run: $(app_dir)/debug
	@^

test: $(app_dir)/release
	$^ test

dirs:
	mkdir -p $(release_obj_dir)
	mkdir -p $(debug_obj_dir)
	mkdir -p $(app_dir)

clean: 
	rm -rf $(build_dir)
	rm -f $(src)/movetables.cpp
	mkdir -p $(release_obj_dir)
	mkdir -p $(debug_obj_dir)
	mkdir -p $(app_dir)

docs:
	doxygen > /dev/null

$(app_dir)/release: $(release_objects)
	g++ $(flags) $(release_flags) -o $@ $^

$(app_dir)/debug: $(debug_objects)
	g++ $(flags) $(debug_flags) -o $@ $^

$(release_objects): $(release_obj_dir)/%.o : $(src)/%.cpp
	g++ $(flags) $(release_flags) -c -o $@ $^

$(debug_objects): $(debug_obj_dir)/%.o : $(src)/%.cpp
	g++ $(flags) $(debug_flags) -c -o $@ $^

$(src)/movetables.cpp: $(src)/generate_movetables.cpp $(release_obj_dir)/bitboard.o
	g++ $(flags) $(release_flags) -c -o $(release_obj_dir)/generate_movetables.o $(src)/generate_movetables.cpp
	g++ $(flags) $(release_flags) -o $(app_dir)/generate_movetables $(release_obj_dir)/generate_movetables.o $(release_obj_dir)/bitboard.o
	$(app_dir)/generate_movetables
	mv movetables.cpp $(src)/movetables.cpp

#movetables.cpp: generate_movetables.cpp $(debug_obj_dir)/bitboard.o
#	g++ $(flags) $(debug_flags) -c -o $(debug_obj_dir)/generate_movetables.o $(src)/generate_movetables.cpp
#	g++ $(flags) $(debug_flags) -o $(app_dir)/generate_movetables $(debug_obj_dir)/generate_movetables.o $(debug_obj_dir)/bitboard.o
#	$(app_dir)/generate_movetables
#	mv movetables.cpp $(src)/movetables.cpp


