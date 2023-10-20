.SUFFIXES:
.PHONY: pc, fu, pcpla, t 
.PRECIOUS: %_exe

compilador:=g++

all: fumadores_exe fumadores_remix_exe prodcons_cola_exe prodcons_pila_exe prodcons_cola_multi_exe prodcons_pila_multi_exe

fu: fumadores_exe
	./$< 

fur: fumadores_remix_exe
	./$<

pcc: prodcons_cola_exe
	./$<

pcp: prodcons_pila_exe
	./$<

pccm: prodcons_cola_multi_exe
	./$<

pcpm: prodcons_pila_multi_exe
	./$<

## regla para compilar (.cpp --> _exe)
%_exe: %.cpp scd.cpp scd.h
		$(compilador) -std=c++11 -pthread -o $@ $< scd.cpp

mr.proper:
	rm -f *_exe
