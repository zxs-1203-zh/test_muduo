default:main
gdb:main_g

main:*.h *.cc
	g++ -std=c++11 -o main *.cc -lmuduo_base -pthread -Wall
main_g:*.h *.cc
	g++ -std=c++11 -o main_g *.cc -lmuduo_base -pthread -g -Wall

clean:
	rm -f main main_g
