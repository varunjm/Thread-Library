c: objectc archive clean 
cpp: objectcpp archive clean

objectc: 
		gcc -c test.c -o test
objectcpp:
		g++ -c test.cpp -o test
archive:
		ar rcs mythread.a ./test
clean:
	rm test