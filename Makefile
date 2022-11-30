all:
	g++ -c memlab.cpp -pthread
	ar -rcs libmemlab.a memlab.o
	g++ demo1.cpp -L. -lmemlab -pthread -o demo1
	g++ demo3.cpp -L. -lmemlab -pthread -o demo3
clean:
	rm memlab.o libmemlab.a demo1 demo3
