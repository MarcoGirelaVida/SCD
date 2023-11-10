.SUFFIXES:
.PHONY: x, pc1, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors

x: monitor_em_exe
	./$<

pcml: prodcons_mu_lifo_exe
	./$<

pcmf: prodcons_mu_fifo_exe
	./$<

fm: fumadores_mu_exe

%_exe: %.cpp scd.cpp scd.h
	$(compilador) $(opcionesc)  -o $@ $< scd.cpp

clean:
	rm -f *_exe
