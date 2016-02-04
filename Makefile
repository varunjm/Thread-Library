c: objectc archive clean 
cpp: objectcpp archive clean

objectc: 
		gcc -c thread.c -o test
objectcpp:
		g++ -c thread.cpp -o test
archive:
		ar rcs mythread.a ./test
clean:
	rm test
