// SPDX-License-Identifier: GPL-3.0-only
#include <iostream>
#include <string>
#include <sstream>
#include "inslv/difuso/fuzzy_t.h"

namespace inslv {

using namespace std;

// Constructor por defecto de la clase
fuzzy_t::fuzzy_t(){
  a=b=c=d=0;
  menosinfinito=masinfinito=false;
  nombre = "Creado, no usado";
}












// Operador de asignaci�n por copia
fuzzy_t &fuzzy_t::operator=(const fuzzy_t &x){
  a=x.a;
  b=x.b;
  c=x.c;
  d=x.d;
  menosinfinito=x.menosinfinito;
  masinfinito=x.masinfinito;
  nombre = x.nombre;
  return *this;
}



// Operador de asignaci�n para etiquetas no acotadas por un extremo.
void fuzzy_t::Asigna(double a, double b, double c, double d, string name, bool menos, bool mas){
  this->a = a;
  this->b = b;
  this->c = c;
  this->d = d;
  menosinfinito = menos;
  masinfinito = mas;
  nombre = name;
}

// Operador de asignaci�n para etiquetas acotadas en sus dos extremos.
void fuzzy_t::Asigna(double a, double b, double c, double d, string name){
  this->a = a;
  this->b = b;
  this->c = c;
  this->d = d;
  menosinfinito = false;
  masinfinito = false;
  nombre = name;
}



// Operador que evalua el grado de adaptaci�n de un determinado valor x
// a la etiqueta.
double fuzzy_t::Adaptacion(double x) const {
  if (x==-999999999)
    return 1;
  if ( (menosinfinito && x<c) || (masinfinito && x>b))
    return 1;
  if (x<a)
    return 0;
  else if (x<b)
         return (x-a)/(b-a);
       else  if (x<=c)
                return 1;
             else  if (x<d)
	              return (d-x)/(d-c);
		   else
		      return 0;
}







// Operador que pone en el dispositivo de salida la definici�n de la etiqueta.
void fuzzy_t::Pinta() const {
   cout << "Nombre: " << nombre << endl;
   if (menosinfinito)
     cout << "[-inf,-inf," << c << "," << d << "]\n";
   else if (masinfinito)
          cout << "[" << a <<","<< b <<",inf,inf]\n";
	else
          cout << "[" << a <<","<< b <<"," << c << "," << d << "]\n";
}







// Operador que devuelve en un string el nombre de la etiqueta.
string fuzzy_t::SPrint() const {
  return nombre;
}


























}  // namespace inslv
