%.o: %.c
	g++ -c -o $@ $<

all: dt-format dt-check

dt-format: dt-format.o dt.o
	g++ -o $@ $^

dt-check: dt-check.o dt.o
	g++ -o $@ $^
