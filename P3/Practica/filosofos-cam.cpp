// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,                    // número de filósofos 
   num_filo_ten  = 2*num_filosofos,       // número de filósofos y tenedores 
   num_procesos  = num_filo_ten + 1,      // número de procesos total (por ahora solo hay filo y ten)
   ID_CAMARERO = 10,                       // El id que deseamos que tenga el camarero
   TAG_TENEDORES = 0,
   TAG_CAMARERO = 1,
   tag_camarero_termina_comer = 2,
   tag_camarero_solicitud_asiento = 3;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_filosofos( const int id, const int id_camarero, const int tag_tenedores, const int tag_camarero)
{
   MPI_Status estado;
   int id_ten_izq = (id+1)  % num_filo_ten, //id. tenedor izq.
      id_ten_der = (id+num_filo_ten-1)  % num_filo_ten; //id. tenedor der.
   int valor = 0;

   while ( true )
   {
      char solicito_sentarme = 's';
      char puedo_sentarme;
      cout << "Filósofo " << id << " se sienta a la mesa." << endl;
      MPI_Ssend(&solicito_sentarme, 1, MPI_CHAR, id_camarero, tag_camarero_solicitud_asiento, MPI_COMM_WORLD);
      MPI_Recv(&puedo_sentarme, 1, MPI_CHAR, id_camarero, tag_camarero_solicitud_asiento, MPI_COMM_WORLD, &estado);
      // Se usa Ssend para que la comunicación se síncrona segura

      char solicito_tenedor = 's';
      cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
      // ... solicitar tenedor izquierdo (completar)
      MPI_Ssend(&solicito_tenedor, 1, MPI_CHAR, id_ten_izq, tag_tenedores, MPI_COMM_WORLD);

      cout <<"Filósofo " << id <<" solicita ten. der." <<id_ten_der <<endl;
      // ... solicitar tenedor derecho (completar)
      MPI_Ssend(&solicito_tenedor, 1, MPI_CHAR, id_ten_der, tag_tenedores, MPI_COMM_WORLD);
      
      cout <<"Filósofo " << id <<" comienza a comer" <<endl ;
      sleep_for( milliseconds( aleatorio<10,100>() ) );

      char termine_usar_tenedor = 't';
      cout <<"Filósofo " << id <<" suelta ten. izq. " <<id_ten_izq <<endl;
      // ... soltar el tenedor izquierdo (completar)
      MPI_Ssend(&termine_usar_tenedor, 1, MPI_CHAR, id_ten_izq, tag_tenedores, MPI_COMM_WORLD);

      cout<< "Filósofo " << id <<" suelta ten. der. " <<id_ten_der <<endl;
      // ... soltar el tenedor derecho (completar)
      MPI_Ssend(&termine_usar_tenedor, 1, MPI_CHAR, id_ten_der, tag_tenedores, MPI_COMM_WORLD);
     

      char termine_de_comer = 't';
      cout<< "Filósofo " << id <<" se levanta de la mesa" <<id_ten_der <<endl;
      MPI_Ssend(&termine_de_comer, 1, MPI_CHAR, id_camarero, tag_camarero_termina_comer, MPI_COMM_WORLD);


      cout << "Filosofo " << id << " comienza a pensar" << endl;
      sleep_for( milliseconds( aleatorio<10,100>() ) );
   }
}
// ---------------------------------------------------------------------

void funcion_tenedores( const int id, const int tag_tenedores)
{
   int valor, id_filosofo ;  // valor recibido, identificador del filósofo
   MPI_Status estado ;       // metadatos de las dos recepciones
   char solicita_tenedor;
   char termino_usar_tenedor;

   while ( true )
   {
      // ...... recibir petición de cualquier filósofo (completar)
      MPI_Recv(&solicita_tenedor, 1, MPI_CHAR, MPI_ANY_SOURCE, tag_tenedores, MPI_COMM_WORLD, &estado);
      // ...... guardar en 'id_filosofo' el id. del emisor (completar)
      id_filosofo = estado.MPI_SOURCE;

      cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;


      // ...... recibir liberación de filósofo 'id_filosofo' (completar)
      MPI_Recv(&termino_usar_tenedor, 1, MPI_CHAR, id_filosofo, tag_tenedores, MPI_COMM_WORLD, &estado);

      cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
   }
}

void funcion_camarero(const int tag_camarero)
{
   int id_filosofo_solicitante;  // valor recibido, identificador del filósofo
   MPI_Status estado ;       // metadatos de las dos recepciones
   size_t filosofos_sentados = 0;
   char puedes_sentarte = 'p';
   char mensaje;

   while (true)
   {
      //MPI_Iprobe( MPI_ANY_SOURCE, tag_camarero, MPI_COMM_WORLD, &hay_mensaje_pendiente, &estado);
      if (filosofos_sentados >= 4)
      {
         MPI_Recv(&mensaje, 1, MPI_CHAR, MPI_ANY_SOURCE, tag_camarero_termina_comer, MPI_COMM_WORLD, &estado);
         filosofos_sentados--;
      }
      else
      {
         MPI_Recv(&mensaje, 1, MPI_CHAR, MPI_ANY_SOURCE, tag_camarero_solicitud_asiento, MPI_COMM_WORLD, &estado);
         filosofos_sentados++;
         id_filosofo_solicitante = estado.MPI_SOURCE;
         MPI_Ssend(&puedes_sentarte, 1, MPI_CHAR, id_filosofo_solicitante, tag_camarero_solicitud_asiento, MPI_COMM_WORLD);
      }
   }
   
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );          // Para saber qué proceso se está ejecutando actalmente
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );// Para crear el grupo con todos los procesos


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if (id_propio == ID_CAMARERO)
         funcion_camarero( TAG_CAMARERO );
      else if (id_propio % 2 == 0)           // si es par (por tanto cualquiera de una mitad de los procesos)
         funcion_filosofos( id_propio, ID_CAMARERO, TAG_TENEDORES, TAG_CAMARERO);     //   es un filósofo
      else                                   // si es impar (por tanto cualquiera de una segunda mitad de los procesos)
         funcion_tenedores( id_propio , TAG_TENEDORES);     //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
