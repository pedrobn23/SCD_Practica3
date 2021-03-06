#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"
#include <vector>


using namespace std ;
using namespace SEM ;


const int num_fum = 3;

// semáforos compartidos
Semaphore fumador[num_fum] = {0,0,0};
Semaphore estanquero = 0;

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

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

inline int producir ()
{
  return aleatorio<0,2>(); 
}
void funcion_hebra_estanquero(  )
{
  
  while (1) {
    int producto = producir();
    cout << "Estanquero sirve producto: " << producto << endl;
    sem_signal(fumador[producto]);

    sem_wait(estanquero);
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
      sem_wait(fumador[num_fumador]);
      cout << "Fumador " << num_fumador << " retira el producto." << endl;
      sem_signal(estanquero);
      fumar(num_fumador);
    }
}

//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

  thread hebra_fumador[num_fum ];

  for ( int i = 1; i <= num_fum; ++i )
    hebra_fumador[i] = thread ( funcion_hebra_fumador, i );
     
  for ( int i = 1; i <= num_fum; ++i )
    hebra_fumador[i].join();
   

   cout << "-------------------------------------------------------" << endl
	<< "FIN"
	<< "-------------------------------------------------------" << endl;
}
