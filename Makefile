CXX=mpic++
CXXFLAGS=-g -O0
OBJECTS=main.o
TARGET=multiplier
.SUFFIXIES: .cpp .o

all: $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS)
fclean:
	rm $(OBJECTS) $(TARGET)
cpp.o:
	$(CXX) $(CXXFLAGS) -c $*.cpp
exe: all
	mpiexec --oversubscribe --mca opal_warn_on_missing_libcuda 0 -n 4 $(TARGET) matA.dat matB.dat 8
re: fclean all exe