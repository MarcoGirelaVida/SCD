#include<semaphore.h>#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "./P1/scd.h"

class Puente{

    int colasur = 0;
    int colanorte = 0;
    int entrandosur = 0;
    int entrandonorte = 0;
    int contadorpasando = 0;

    void EntrarCocheDelNorte()
    {
        if (entrandosur && contadorpasando == 10)
            colanorte.wait();

        entrandonorte++;
        if (colasur.queu()) 
            contadorpasando++;

        
        if(pasando < 10)
            colanorte.signal();

    }

    void SalirCocheNorte()
    {
        entrandonorte--;
        if (colanorte)
        {
            pasando = 0;
            colasur.signal();
        }
        
    }

    void EntrarCocheDelSur()
    {
        if (entrandonorte)
        {
            colasur.wait();
        }
        entrandosur++;
        colasur.signal();

    }


    void SalirCocheSur()
    {
        entrandosur--;
        if (colasur)
        {
            colanorte.signal();
        }
    }

    void init()
    {

    }
};