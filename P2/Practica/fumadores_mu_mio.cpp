
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
   constexpr unsigned short int NUM_FUMADORES_DEFAULT = 3;
   constexpr unsigned short int NUM_ESTANQUEROS_DEFAULT = 1;
   constexpr int NUM_CIGARROS_POR_FUMADOR_DEFAULT = 1000;

   int * cigarros_fumados_por_cada_fumador;

//-------------------------------------------------------------------------
// FUNCIONES

const unsigned short int producir_ingrediente_aleatorio(const unsigned short int NUM_FUMADORES)
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produccion( aleatorio<10,90>() );

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produccion );

   const unsigned short int min_aleatorio = 0;
   const unsigned short int max_aleatorio = NUM_FUMADORES-1;
   // NO FUNCIONA CON ARGUMENTOS VARIABLES DE NUM_FUMADORES
   const unsigned short int num_ingrediente = aleatorio<min_aleatorio, NUM_FUMADORES_DEFAULT>();
   return num_ingrediente;
}

// --- //

void fumar()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<100,300>() );

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );
}

//-------------------------------------------------------------------------
// MONITOR
// 1 var "mostrador" con el ingrediente que tiene
// 1 var condición por cada fumador 1 var condición para el mostrador
// Método coger ingrediente
// Método poner ingrediente
class Estanco : public HoareMonitor
{
private:
   //---//
   short int ingrediente_mostrador;
   bool mostrador_en_uso;

   //---//
   //CondVar esperando_mostrador_libre;
   CondVar estanquero_esperando_mostrador_vacio;
   CondVar fumador_esperando_ingrediente[];
   
public:
   Estanco(const unsigned short int NUM_FUMADORES_ESTANCO);
   ~Estanco();
   void obtener_ingrediente(unsigned short int ingrediente);
   void ofrecer_ingrediente(unsigned short int ingrediente);
};

Estanco::Estanco(const unsigned short int NUM_FUMADORES_ESTANCO)
{
   ingrediente_mostrador = -1;
   mostrador_en_uso = false;

   estanquero_esperando_mostrador_vacio = newCondVar();

   // La librería no permite
   fumador_esperando_ingrediente = new CondVar[NUM_FUMADORES_ESTANCO];
   for (size_t i = 0; i < NUM_FUMADORES_ESTANCO; i++)
      fumador_esperando_ingrediente[i] = newCondVar();
}

Estanco::~Estanco()
{
   //delete fumador_esperando_ingrediente;
}

void Estanco::obtener_ingrediente(unsigned short int ingrediente)
{
   if (ingrediente_mostrador != ingrediente)
      fumador_esperando_ingrediente[ingrediente].wait();
   
   //if (mostrador_en_uso)
   //   esperando_mostrador_libre.wait();
   
   //mostrador_en_uso = true;
   cigarros_fumados_por_cada_fumador[ingrediente]++;
   ingrediente_mostrador = -1;
   //mostrador_en_uso = false;
   estanquero_esperando_mostrador_vacio.signal();
   //esperando_mostrador_libre.signal();
}

void Estanco::ofrecer_ingrediente(unsigned short int ingrediente)
{
   if (ingrediente_mostrador != -1)
      estanquero_esperando_mostrador_vacio.wait();
   
   //if (mostrador_en_uso)
   //   esperando_mostrador_libre.wait();

   //mostrador_en_uso = true;
   ingrediente_mostrador = ingrediente;
   //mostrador_en_uso = false;
   fumador_esperando_ingrediente[ingrediente].signal();
   //esperando_mostrador_libre.signal();
}


//-------------------------------------------------------------------------
// FUNCIONES HEBRAS
// funcion hebra estanquero
void funcion_hebra_estanquero(MRef<Estanco> estanco, const unsigned short int NUM_FUMADORES, const int num_cigarros_a_producir)
{
   bool infinitos_cigarros = num_cigarros_a_producir == -1;
   for (size_t i = 0; i < num_cigarros_a_producir || infinitos_cigarros; i++)
   {
      unsigned short int ingrediente = producir_ingrediente_aleatorio(NUM_FUMADORES);
      estanco->ofrecer_ingrediente(ingrediente);
   }
}

// funcion hebra fumador
void funcion_hebra_fumador(MRef<Estanco> estanco, const unsigned short int num_fumador, const int num_cigarros_a_fumar)
{
   bool infinitos_cigarros = num_cigarros_a_fumar == -1;
   for (size_t i = 0; i < num_cigarros_a_fumar || infinitos_cigarros; i++)
   {
      estanco->obtener_ingrediente(num_fumador);
      fumar();
   }
}

//-------------------------------------------------------------------------
// TEST
void test_fumadores(const int NUM_CIGARROS_TOTAL, const int NUM_CIGARROS_POR_FUMADOR)
{
   bool ok = true ;
   cout << "comprobando contadores ...." << endl ;

   for( unsigned i = 0 ; i < NUM_CIGARROS_TOTAL ; i++ )
   {
      if ( cigarros_fumados_por_cada_fumador[i] !=  NUM_CIGARROS_POR_FUMADOR)
      {
         cout << "error: fumador " << i << " fumó " << cigarros_fumados_por_cada_fumador[i] << " cigarros." << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//-------------------------------------------------------------------------
// MAIN
int main(int argc, char * argv[])
{
   unsigned short int tmp_num_fumadores = NUM_FUMADORES_DEFAULT;
   unsigned short int tmp_num_estanqueros = NUM_ESTANQUEROS_DEFAULT;
   int tmp_num_cigarros_por_fumador = NUM_CIGARROS_POR_FUMADOR_DEFAULT;

   //---//

   if (argc >= 1 && argc <= 4)
   {
      if (argc >= 2)
      {
         tmp_num_cigarros_por_fumador = atoi(argv[1]);
         if (argc >= 3)
         {
            tmp_num_fumadores = atoi(argv[2]);

            if (argc == 4)
               tmp_num_estanqueros = atoi(argv[3]);
         }
      }
   }
   else
   {
      cerr << "Error, número de argumentos incorrecto, uso: [num_cigarros_por_fumador] [fumadores_mu] [num_estaqueros]" << endl;
      exit(1);
   }

   //---//
   
   const short int NUM_FUMADORES = tmp_num_fumadores;
   const short int NUM_ESTANQUEROS = tmp_num_estanqueros;
   const int NUM_CIGARROS_POR_FUMADOR = NUM_CIGARROS_POR_FUMADOR_DEFAULT;
   const int NUM_CIGARROS_TOTAL = NUM_CIGARROS_POR_FUMADOR * NUM_FUMADORES;

   MRef<Estanco> estanco = Create<Estanco>(NUM_FUMADORES);

   cigarros_fumados_por_cada_fumador = new int[NUM_FUMADORES];
   cigarros_fumados_por_cada_fumador = {0};

   //---------------------------//

   cerr << "-----------------------------------------------------------------" << endl;
   cout << "Problema de los fumadores con monitores." << endl;
   cout << "------------------------------------------------------------------" << endl;
   cout << flush ;

   //---------------------------//

   thread hebras_estanqueros[NUM_ESTANQUEROS];
   thread hebras_fumadores[NUM_FUMADORES];

   for (size_t i = 0; i < NUM_ESTANQUEROS; i++)
      hebras_estanqueros[i] = thread(funcion_hebra_estanquero, estanco, NUM_FUMADORES, NUM_CIGARROS_TOTAL);
   for (size_t i = 0; i < NUM_FUMADORES; i++)
      hebras_fumadores[i] = thread(funcion_hebra_fumador, estanco, i, NUM_CIGARROS_POR_FUMADOR);

   //---//
   
   for (size_t i = 0; i < NUM_ESTANQUEROS; i++)
      hebras_estanqueros[i].join();
   
   for (size_t i = 0; i < NUM_FUMADORES; i++)
      hebras_fumadores[i].join();
   
   //---------------------------//

   test_fumadores(NUM_CIGARROS_TOTAL, NUM_CIGARROS_POR_FUMADOR);

   return 0;
}

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




