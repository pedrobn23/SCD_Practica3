#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 40 ,   // número de items
          tam_vec   = 10 ;   // tamaño del buffer

unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos


int primera_libre = 0,
    primera_ocupada = 0 ,
    buffer[tam_vec];


// semáforos compartidos
Semaphore puede_escribir = tam_vec ,
          puede_leer  = 0 ; // 1 si hay valor pendiente de leer


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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;
   
}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(  )
{
  for( unsigned i = 0 ; i < num_items ; i++ )
    {
      //produce el dato de modo concurrente
      int dato = producir_dato() ;
      
      //espera a que sea posible la inserción
      sem_wait( puede_escribir ) ;

      buffer[primera_libre] = dato ; // escribe el valor
      primera_libre = (primera_libre+1)%tam_vec; //actualiza el valor del indice (cola circular)
      cout << "escrito: " << dato << endl ;

      sem_signal( puede_leer ) ;
      //envía señal para que se pueda proceder con la lectura
    }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
     //espera hasta que haya algo que leer
      sem_wait( puede_leer ) ;

      int dato = buffer[primera_ocupada] ; // lee el valor generado
      primera_ocupada = (primera_ocupada+1)%tam_vec; // incrementa el valor del lecto (cola circular)
      cout << "leído: " << dato << endl;

      sem_signal( puede_escribir ) ;
      //Envía señal de que se puede sobreescribir una casilla del buffer

      
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

   test_contadores();

   return 0;
}
