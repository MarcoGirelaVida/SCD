// ************************************************************************* //
/**
 * @file fumadores_mu.cpp
 * @brief Fichero con el ejercicio de la práctica 2 de SCD : Implementación de P1/fumadores.cpp con monitores
 * Consta de 6 partes:
 * 
 * @section sec_global_vars 1. Variables Globales
 * Contiene las variables globales del programa:  
 * @ref NUM_FUMADORES "NUM_FUMADORES"  
 * @ref cigarros_fumados_por_cada_fumador "cigarros_fumados_por_cada_fumador"  
 * @ref cantidad_fabricada_de_cada_ingrediente "cantidad_fabricada_de_cada_ingrediente"  
 * 
 * @section sec_func_no_em 2. Funciones a Ejecutar Concurrentemente
 * Contiene las funciones que se pueden ejecutar de forma concurrente, estas son:  
 * Función que podrán ejecutar los fumadores concurrentemente: @ref fumar "fumar"  
 * Función que podrá ejecutar el estanquero concurrentemente: @ref fabricar_ingrediente "fabricar_ingrediente"  
 * 
 * @section sec_class_mostrador 3. Clase Mostrador
 * Contiene el monitor Mostrador que proporcionará las variables y funciones necesarias para acceder al mostrador garantizando la exclusión mutua  
 * @ref Mostrador "Clase Mostrador"  
 * 
 * @section sec_func_hebras 4. Funciones a Ejecutar por cada Hebra.
 * Contiene las funciones que ejecutará cada actor  
 * Función que ejecutará el estanquero: @ref "funcion_hebra_estanquero" funcion_hebra_estanquero  
 * Función que ejecutará cada fumador: @ref "funcion_hebra_fumador" funcion_hebra_fumador  
 * 
 * @section sec_test 5. Tests de Exclusión Mutua
 * Contiene el test para comprobar que cada fumador a fumado lo que le corresponde  
 * @ref test_fumadores "test_fumadores"  
 * 
 * @section sec_main 6. Main
 * Contiene el main del programa  
 * @ref main "main"  
 * 
 * @author Marco Girela Vida
 * @date 11-11-2023
*/
// ************************************************************************* //
/**
 * @brief Dependencias: scd.h, isotream, thread, cassert, chrono
*/
#include "scd.h"
#include <iostream>
#include <thread>
#include <cassert>
#include <chrono>

using namespace std;
using namespace scd;

// ************************************************************************* //
/**
 * @brief Número de fumadores disponibles
 * @pre 0 <= NUM_FUMADORES
*/
constexpr unsigned short int NUM_FUMADORES = 3;

/**
 * @brief Vector que almacena el número de cigarros fumados por cada fumador
 * Se usa en @ref test_fumadores "test_fumadores" para comprobar que el programa se ha ejecutado correctamente
*/
unsigned int cigarros_fumados_por_cada_fumador[NUM_FUMADORES] = {0};

/**
 * @brief Vector que almacena el número de cada ingrediente que ha fabricado el estanquero
 * Se usa en @ref test_fumadores "test_fumadores" para comprobar que el programa se ha ejecutado correctamente
*/
unsigned int cantidad_fabricada_de_cada_ingrediente[NUM_FUMADORES] = {0};

// ************************************************************************* //
/** 
 * @brief Fuma un cigarro
 * @param num_fumador Número del fumador que va a fumar
 * @note Sería mejor si no se hiciesen couts, el parámetro no sería necesario. Pero bueno
*/
void fumar(const unsigned short int num_fumador)
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

/**
 * @brief Fabrica un ingrediente aleatorio del cigarro
 * @return Número del ingrediente fabricado
*/
const unsigned short int fabricar_ingrediente()
{
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_produ( aleatorio<10,90>() );

    // informa de que comienza a producir
    cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
    this_thread::sleep_for( duracion_produ );

    const int num_ingrediente = aleatorio<0,NUM_FUMADORES-1>() ;

    // informa de que ha terminado de producir
    cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

    return num_ingrediente ;
}

// ************************************************************************* //
/**
 * @brief Representa el mostrador del estanco
 * El mostrador es el punto de concurrencia entre fumadores y estanquero donde se debe garantizar la exclusión mutua.
 * Contiene las variables y métodos del mostrador donde se tienen que dar la exclusión mutua.
*/
class Mostrador : public HoareMonitor
{
private:
    /**
     * @brief Identificador del ingrediente que se encuentra actualmente en el mostrador
     * @note Si @p ingrediente_en_mostrador = -1 significa que no hay ningún ingrediente
    */
    short int ingrediente_en_mostrador;

    /**
     * @brief Cola de los fumadores/estanquero esperando a que se libere el mostrador
    */
    CondVar esperando_mostrador_libre;

    /**
     * @brief Cola de los fumadores esperando a que su ingrediente se encuentre en el mostrador
     * @note Quizás sería mejor hacer una cola para cada ingrediente y que no se den signal en bano
    */
    CondVar esperando_ingrediente;

public:

    /**
     * @brief Constructor por defecto
     * @post Genera una instancia de la clase mostrador con ningún ingrediente en el mostrador ( @ref Mostrador::ingrediente_en_mostrador "ingrediente_en_motrador" = -1)
     * @return Objeto de la clase mostrador 
    */
    Mostrador();

    /**
     * @brief Un fumador intenta recoger el @p ingrediente indicado del mostrador
     * @param ingrediente Número del ingrediente que se está intentando recoger
     * @post Si se encuentra en el mismo finaliza con normalidad
     * @post Si no se encuetra se va a la cola  @ref Mostrador::esperando_ingrediente "esperando_ingrediente" e intentará recogerlo de nuevo cuando se le avise
    */
    void recoger_ingrediente(const unsigned short int ingrediente);

    /**
     * @brief El estanquero intenta ofrecer el @p ingrediente en el mostrador
     * @param ingrediente Número del ingrediente que se está intentando ofrecer
     * @post Si el mostrador se encuentra vacio lo ofrece y finaliza con normalidad
     * @post Si el mostrador se encuetra ocupado se va a la cola @ref Mostrador::esperando_mostrador_libre "esperando_mostrador_libre" e intentará ofrecerlo de nuevo cuando se le avise
    */
    void ofrecer_ingrediente(const unsigned short int ingrediente);
};

Mostrador::Mostrador()
{
   ingrediente_en_mostrador = -1;
   esperando_mostrador_libre = newCondVar();
   esperando_ingrediente = newCondVar();
}

void Mostrador::recoger_ingrediente(const unsigned short int ingrediente)
{
    while (ingrediente_en_mostrador != ingrediente){
      esperando_ingrediente.signal();
      esperando_ingrediente.wait();
    }

    cigarros_fumados_por_cada_fumador[ingrediente]++;
    ingrediente_en_mostrador = -1;
    cout << "           Retirado ingrediente: " << ingrediente << endl;
    esperando_mostrador_libre.signal();
}

void Mostrador::ofrecer_ingrediente(const unsigned short int ingrediente)
{
    while (ingrediente_en_mostrador != -1)
        esperando_mostrador_libre.wait();

    ingrediente_en_mostrador = ingrediente;
    cout << "           Puesto ingrediente: " << ingrediente << endl;
    esperando_ingrediente.signal();
}

// ************************************************************************* //
/**
 * @brief Función que ejecuta la hebra de cada fumador
 * Esta incluye @ref fumar "fumar" (Concurrente) y @ref Mostrador::recoger_ingrediente "recoger_ingrediente" (Exclusión Mutua)
 * @param mostrador referencia al mostrador del estanco donde está solicitando los ingredientes el fumador
 * @param num_fumador Número del fumador asociado a la hebra y por tanto del ingrediente que busca.
*/
void funcion_hebra_fumador(MRef<Mostrador> mostrador, const unsigned short int num_fumador)
{
    while (true)
    {
        mostrador->recoger_ingrediente(num_fumador);
        fumar(num_fumador);  
    }
}

/**
 * @brief Función que ejecuta la hebra del estanquero
 * Esta incluye @ref fabricar_ingrediente "fabricar_ingrediente" (Concurrente) y @ref Mostrador::ofrecer_ingrediente "ofrecer_ingrediente" (Exclusión Mutua)
 * @param mostrador referencia al mostrador del estanco donde el estanquero está ofreciendo los ingredientes
*/
void funcion_hebra_estanquero(MRef<Mostrador> mostrador)
{
    while (true)
    {
        const unsigned short int ingrediente_fabricado = fabricar_ingrediente();
        cantidad_fabricada_de_cada_ingrediente[ingrediente_fabricado]++;
        mostrador->ofrecer_ingrediente(ingrediente_fabricado);
    }
}

// ************************************************************************* //
/**
 * @brief Testea que cada fumador haya tanto cigarros como ingredientes (del fumador) produjo el estanquero
 * @post Interrumpe la ejecución del programa si algún fumador fumó un número incorrecto de cigarros e informa de dicho error
*/
void test_fumadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << endl ;

   for( unsigned i = 0 ; i < NUM_FUMADORES; i++ )
   {
      if ( cigarros_fumados_por_cada_fumador[i] !=  cantidad_fabricada_de_cada_ingrediente[i])
      {
         cout << "error: fumador " << i << " fumó " << cigarros_fumados_por_cada_fumador[i] << " cigarros y solo se fabricaron: " << cantidad_fabricada_de_cada_ingrediente[i] << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// ************************************************************************* //
/**
 * @brief main
 * @post Se mostrará por la salida estandar el resultado de @ref test_fumadores "test_fumadores"
*/
int main()
{
    MRef<Mostrador> mostrador = Create<Mostrador>();

    //---------------------------//

    cerr << "-----------------------------------------------------------------" << endl;
    cout << "Problema de los fumadores con monitores." << endl;
    cout << "------------------------------------------------------------------" << endl;
    cout << flush ;

    //---------------------------//

    thread hebra_estanquero = thread (funcion_hebra_estanquero, mostrador);

    thread hebras_fumadores[NUM_FUMADORES];
    for (size_t i = 0; i < NUM_FUMADORES; i++)
        hebras_fumadores[i] = thread(funcion_hebra_fumador, mostrador, i);

    //---------------------------//

    for (size_t i = 0; i < NUM_FUMADORES; i++)
        hebras_fumadores[i].join();

    hebra_estanquero.join();

    //---------------------------//

    //test_fumadores();

    return 0;
}


// ************************************************************************* //
// ************************************************************************* //
// ************************************************************************* //
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