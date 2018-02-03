a.out: inputbuf.o lexer.o parser.o 
	g++ inputbuf.o lexer.o parser.o -o a.out

inputbuf.o: inputbuf.cc inputbuf.h
	g++ -c inputbuf.cc

lexer.o: lexer.cc lexer.h inputbuf.h
	g++ -c lexer.cc
clean:
	rm *.o

cleanAll:
	rm *.o a.out

