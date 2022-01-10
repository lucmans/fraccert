##
# Makefile: the makefile for Fraccert
# @author Luc de Jonckheere
##

# Min required -std=c++11


# General compiler flags
CXX = g++
CXXFLAGS = -std=c++11 -s  #-fsanitize=address
WARNINGS = -Wall -Wextra -Wfloat-equal
OPTIMIZATION = -O3 #-march=native -mtune=native # -mfma -mavx2 -ftree-vectorize -ffast-math
LIBS = -lSDL2 -lgmp -fopenmp
CORES = 8

# Front-end building and linking info
BIN = fraccert
FRONTEND = select_scale.o graphics.o program.o iocontroller.o console.o locations.o main.o

# Back-end building and linking info
LIBNAME = fracfast
BACKEND = shapes.o fractal.o mandelbrot.o julia.o
# It's also possible to build it shared by changing .a to .so and removing the comment below
# Be use to rebuild ("make -B") when switching between static-shared!
FRACCERTLIB = lib$(LIBNAME).a
SHAREDLINK = #-L./ -Wl,-rpath=./ -l$(LIBNAME)


all:
	make -j $(CORES) $(BIN)

$(BIN): $(FRONTEND) $(FRACCERTLIB)  #$(addprefix $(LIBNAME)/, $(BACKEND))
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(OPTIMIZATION) -o $@ $^ $(SHAREDLINK) $(LIBS)


# Front-end
main.o: main.cpp tests.cpp locations.h iocontroller.h
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(OPTIMIZATION) -c $<

iocontroller.o: iocontroller.cpp iocontroller.h program.h console.h
console.o: console.cpp console.h locations.h program.h
program.o: program.cpp program.h graphics.h select_scale.h
graphics.o: graphics.cpp graphics.h select_scale.h
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(OPTIMIZATION) -c $<


# Back-end
# Static building
lib$(LIBNAME).a: $(addprefix $(LIBNAME)/, $(BACKEND))
	ar rcs $@ $^

# Shared building
lib$(LIBNAME).so: SHAREDCOMP := -fPIC
lib$(LIBNAME).so: $(addprefix $(LIBNAME)/, $(BACKEND))
	$(CXX) $(OPTIMIZATION) -shared -Wl,-soname,$@ -o $@ $^

$(LIBNAME)/fractal.o: $(LIBNAME)/fractal.cpp $(LIBNAME)/fractal.h  $(LIBNAME)/borderTrace.cpp $(LIBNAME)/borderTrace.h
	$(CXX) $(CXXFLAGS) -fopenmp $(SHAREDCOMP) $(WARNINGS) $(OPTIMIZATION) -c $< -o $@

$(LIBNAME)/mandelbrot.o: $(LIBNAME)/mandelbrot.cpp $(LIBNAME)/mandelbrot.h $(LIBNAME)/mandelbrotGMP.cpp  $(LIBNAME)/shapes.h
$(LIBNAME)/%.o: $(LIBNAME)/%.cpp $(LIBNAME)/%.h
	$(CXX) $(CXXFLAGS) $(SHAREDCOMP) $(WARNINGS) $(OPTIMIZATION) -c $< -o $@



# Various scripts
run:
	./$(BIN)


# For studying the generated assembly
%.s: %.cpp  %.h
	$(CXX) -S -fverbose-asm -g -O2 $<


valgrind: valgrind.supp
	clear
	valgrind --leak-check=full --show-reachable=yes --error-limit=no --suppressions=./valgrind.supp ./$(BIN)

# Generate suppressions for SDL, X11 and Intel i965 driver shared libraries
gensupp:
	../gen_val_suppress.py


lines:
	wc -l *.h *.cpp fracfast/*.h fracfast/*.cpp


todo:
	grep -n TODO *.cpp *.h || echo -e "Nothing left to do!\n"


clean:
	rm -f *.o
	rm -f $(LIBNAME)/*.o
	rm -f $(BIN)
	rm -f *.a
	rm -f *.so
	rm -f *.s
	rm -f vgcore*

rmresults:
	rm -f results/*.txt
	rm -f results/gmp/*.txt
	rm -f results/threading/*.txt
	rm -f results/threading/threads/*.txt
	rm -f results/threading/splits/*.txt


zip:
	tar -czvf ../fraccert.tar.gz ../gen_val_suppress.py --exclude "rsc" ../fractal
