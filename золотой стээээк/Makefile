debug:
	g++ stack.cpp main.cpp hash.cpp memchecker.cpp -fsanitize=address -fsanitize=leak -Wall

relizz:
	g++ stack.cpp main.cpp hash.cpp memchecker.cpp -D _RELIZ_

run:
	rm ./log_stack
	./a.out

clean:
	rm ./a.out
	rm ./log_stack
