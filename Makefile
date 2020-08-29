testcpp : cpp tests/semtestcase.c
	@g++ tests/semtestcase.c mythread.a -I.
	@./a.out
testc : c tests/semtestcase.c
	@gcc tests/semtestcase.c mythread.a -I.
	@./a.out
c: objectc archive clean 
cpp: objectcpp archive clean
objectc: 
	@gcc -c thread.c -o test
objectcpp:
	@g++ -c thread.cpp -o test
archive:
	@ar rcs mythread.a ./test
clean:
	@rm test
