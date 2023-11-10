
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

//-------------------------------------------------------------------------
// VARIABLES GLOBALES

//-------------------------------------------------------------------------
// FUNCIONES
// fumar
// producir ingrediente

//-------------------------------------------------------------------------
// MONITOR
// 1 var "mostrador" con el ingrediente que tiene
// 1 var condición por cada fumador
// Método coger ingrediente
// Método poner ingrediente

//-------------------------------------------------------------------------
// FUNCIONES HEBRAS
// funcion hebra fumador
// funcion hebra estanquero

//-------------------------------------------------------------------------
// MAIN


/*
// numero de fumadores 

const int num_fumadores = 3 ;

Semaphore mostr_vacio = 1;
Semaphore ingr_disp[num_fumadores] = {0, 0, 0};

//-------------------------------------------------------------------------

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,90>() );

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

void funcion_hebra_estanquero(  )
{
	int i;	
	
	while (true) {
		i = producir_ingrediente();
		
		sem_wait(mostr_vacio);
		cout << "Puesto ingrediente " << i << " en el mostrador" << endl;
		sem_signal(ingr_disp[i]);
	}
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<100,300>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente " << num_fumador << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{	
   while( true )
   {
	sem_wait(ingr_disp[num_fumador]);
	
	cout << "Retirando ingrediente " << num_fumador << endl;
	sem_signal(mostr_vacio);
	fumar(num_fumador);
        cout << "           Fumador " << num_fumador << "  : ha fumado." << endl;
   }
}

//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl 
        << "------------------------------------------------------------------" << endl
        << flush ;
        
   
   //Declaración de hebras
   thread hebra_estanquero(funcion_hebra_estanquero),
   		  hebras_fumadoras[num_fumadores];
   	
   //Poner en marcha las hebras fumadoras	  
   for(int i=0; i<num_fumadores; i++)
   		hebras_fumadoras[i] = thread(funcion_hebra_fumador, i);
   
   
   //Esperar a que terminan las hebras
   hebra_estanquero.join();
   
   for(int i=0; i<num_fumadores; i++)
   		hebras_fumadoras[i].join();
              
        
}
*/




