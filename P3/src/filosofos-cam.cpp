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

const int num_filosofos = 5 ,
  num_procesos    = 2*num_filosofos+1 ,
  id_camarero     = 2*num_filosofos ,
  etiq_sentarse   = 1 ,
  etiq_levantarse = 2 ;


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

//*********************************************************************
// Función que gestiona el comportamiento de los filósofos
void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)                % (num_procesos-1), //id. tenedor izq.
    id_ten_der = (id+(num_procesos-1)-1) % (num_procesos-1); //id. tenedor der.

  int peticion=0;

 
  while ( true )
    {
      cout <<"Filósofo " <<id << " solicita sentarse." <<endl;
      MPI_Ssend( &peticion,    1, MPI_INT, id_camarero, etiq_sentarse , MPI_COMM_WORLD);
      
      cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
      MPI_Ssend( &peticion,    1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

      cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
      MPI_Ssend( &peticion,    1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

      cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
      sleep_for( milliseconds( aleatorio<10,100>() ) );

      cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
      MPI_Ssend( &peticion,    1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

      cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
      MPI_Ssend( &peticion,    1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

      cout <<"Filósofo " <<id << " solicita levantarse." <<endl;
      MPI_Ssend( &peticion,    1, MPI_INT, id_camarero, etiq_levantarse , MPI_COMM_WORLD);

      cout << "Filosofo " << id << " comienza a pensar" << endl;
      sleep_for( milliseconds( aleatorio<10,100>() ) );


    }
}
// ---------------------------------------------------------------------


//*********************************************************************
// Función que gestiona el comportamiento de los tenedores
void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
    {
      MPI_Recv ( &valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado );
      id_filosofo = estado.MPI_SOURCE;// ...... guardar en 'id_filosofo' el id. del emisor (completar)
      cout <<"          Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

      MPI_Recv ( &valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado );
      cout <<"          Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
    }
}
// ---------------------------------------------------------------------


//*********************************************************************
// Función que gestiona el comportamiento del camarero
void funcion_camarero( int id )
{
  int s = 0, valor, hay_mens, id_recibido, etiq_aceptable ;  // valor recibido
  MPI_Status estado_recibido, estado_sondeado ;       // metadatos de las dos recepciones
  
  
  while( true )
    {
      //El camarero inicializa el id que espera a 0, y que no hay mensaje
      id_recibido = 0;
      hay_mens = 0;

      //El camarero piensa como está el aforo.
      if (s < num_filosofos-1) {
	etiq_aceptable = MPI_ANY_TAG;
	cout << "     Camarero: Voy a aceptar cualquier petición." << endl;
      }
      else {
	etiq_aceptable = etiq_levantarse;
	cout << "     Camarero: Voy a esperar a que se vacie un poco la mesa" << endl;
      }
 

      //El camarero prueba si hay peticiones de los filosofos, empezando por el 0 y en bucle
      //hasta que encuentre una satisfactoria ( cuando haya un mensaje )
      while ( !hay_mens )
	{
	  // Comento la comprobación para hacer legible la salida. 
	  //  cout << id_recibido << ", " ;
	  MPI_Iprobe( id_recibido, etiq_aceptable, MPI_COMM_WORLD, &hay_mens, &estado_sondeado);
	  id_recibido = (id_recibido+2)%(num_filosofos*2) ;
	}

      cout << "     Camarero acepta petición de filósofo " << estado_sondeado.MPI_SOURCE << endl;

      
      
      // Recive el mensaje del filósofo, dandole acceso a la mesa
      MPI_Recv ( &valor, 1, MPI_INT, estado_sondeado.MPI_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado_recibido );


      // Actualiza el aforo
      if ( estado_recibido.MPI_TAG == etiq_sentarse )	    
      	s++;
      else
      	s--;
    }
}



// ---------------------------------------------------------------------
int main( int argc, char** argv )
{
  int id_propio, num_procesos_actual ;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
  MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


  if ( num_procesos == num_procesos_actual )
    {
      // ejecutar la función correspondiente a 'id_propio'
      if  (id_propio == num_filosofos*2)// si es el proceso más alto 
	funcion_camarero( id_propio );  //   es un camarero 
      else if ( id_propio % 2 == 0 )    // si es par
	funcion_filosofos( id_propio ); //   es un filósofo
      else                              // si es impar
	funcion_tenedores( id_propio ); //   es un tenedor
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
