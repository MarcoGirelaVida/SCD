/*
¿Cómo se podría hacer una copia de un fichero f en otro g de forma concurrente usando cobegin-coend
Lo que podemos hacer concerrentemente es la lectura y la escritura
*/

#include<iostream>
#include<fstream>
using namespace std;

int funcion_hebra_copia(ifstream fi, ofstream fo)
{
    char next_char = fi.get();

    while (!fi)
    {
        cobegin;
            fo << next_char;
            next_char = fi.get
        coend;
    }
    
}

