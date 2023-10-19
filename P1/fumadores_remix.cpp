#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 

const int num_fumadores = 3 ;
int ing_mostrador = -1;
Semaphore mostrador_libre = 1;
Semaphore fumadores_libres = 3;

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero()
{
   while (true)
   {
      // Fabrica el ingrediente
      int ingrediente_producido = producir_ingrediente();

      // Ocupa el mostrador o espera por si hay alguno recogiendo
      mostrador_libre.sem_wait();

      // Produce el ingrediente
      ing_mostrador = ingrediente_producido;
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      if (ing_mostrador == num_fumador)
      {
         // calcular milisegundos aleatorios que va a tardar en recoger
         chrono::milliseconds duracion_recogida(aleatorio<10,20>());

         // informa de que está recogiendo el ingrediente
         cout << "Fumador " << num_fumador << "  :"
         << " recoge el ingrediente (" << duracion_recogida.count() << " milisegundos)" << endl;

         //Libera el mostrador
         ing_mostrador = -1;
         mostrador_libre.sem_signal();

         // El fumador indica que no está disponible para recoger ingredientes y procede a fumar
         fumadores_libres.sem_wait();
         fumar(num_fumador);

         // Una vez termina indica qu vuelve a estar disponible para fumar
         fumadores_libres.sem_signal();
      }
      
   }
}

//----------------------------------------------------------------------

int main()
{
   thread hebra_estanquero(funcion_hebra_estanquero),
          hebra_fumador1(funcion_hebra_fumador,0),
          hebra_fumador2(funcion_hebra_fumador,1),
          hebra_fumador3(funcion_hebra_fumador,2);
/*
   for (int i = 0; i < num_fumadores; i++)
   {
      thread hebra_fumador(funcion_hebra_fumador,i);
   }

   for (int i = 0; i < num_fumadores; i++)
   {
      hebra_estanquero.join();
   }
*/
   hebra_estanquero.join();
   hebra_fumador1.join();
   hebra_fumador2.join();
   hebra_fumador2.join();


}
