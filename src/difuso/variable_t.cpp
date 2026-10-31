// SPDX-License-Identifier: GPL-3.0-only
using namespace std;
#include <iostream>
#include <string>
#include <cstdlib>
#include "inslv/difuso/variable_t.h"

namespace inslv {

using namespace std;

// Constructor por defecto de la clase
variable_t::variable_t(){
  dominio=0;
  nombre="Sin asignar";
  activa=false;
  antecedente=true;
  unit=0;
  convert_factor=1;
}









// Operador de asignaci�n por copia
variable_t & variable_t::operator=(const variable_t &x){
  if (dominio!=0)
    delete dominio;

  dominio = new domain_t;
  nombre=x.nombre;
  activa=x.activa;
  antecedente=x.antecedente;
  unit = x.unit;
  convert_factor = x.convert_factor;
  *dominio=*(x.dominio);
  return *this;
}


// Destructor de la clase
variable_t::~variable_t(){
  delete dominio;
  dominio=0;
  nombre="Eliminado";
}








// Operador de asignaci�n param�trico de la clase.
// Define una variable en base a un dominio que viene expresado como una secuencia
// ordenada de etiquetas ( [a[i],b[i],c[i],d[i]] expresa la definici�n de la etiqueta i)
// inf y sup indican los limites inferior y superior respectivamente.
// varname es el nombre de la variable y name es un vector con los nombres de las etiquetas.
// status indica como considerar a la variable (-1 variable antecedente y no activa,
// 0 variable antecedente y activa y 1 variable consecuente y activa).



void variable_t::Asigna(int n, string varname, int status, int Unit, double c_factor, double inf,
double sup, double  *a, double *b, double *c, double *d, string *name, bool singleton){
  nombre=varname;
  if (status==-1){
    activa=false;
    antecedente=true;
    unit=0;
    convert_factor=1;
  }
  else if (status==0) {
          activa=true;
	  antecedente=true;
          unit = Unit;
          convert_factor = c_factor;
       }
       else {
          activa=true;
      	  antecedente=false;
          unit = Unit;
          convert_factor = c_factor;
       }
  if (dominio!=0)
    delete dominio;

  dominio = new domain_t;
  (*dominio).Asigna(n,inf,sup,a,b,c,d,name,singleton);
}





// Operador que devuelve el grado de adaptaci�n de un determinado valor x
// a la etiqueta "etiqueta" de la variable.
double variable_t::Adaptacion(double x, int etiqueta) const {
  if (dominio==0){
    cout << "Variable sin dominio asociado\n";
    exit(1);
  }
  return (*dominio).Adaptacion(x,etiqueta);
}



// Operador que devuelve el grado de adaptaci�n de un determinado valor x
// a un grupo de etiquetas "etiquetas" de la variable. El par�metro etiquetas
// viene expresado como un vector ordenado (cadena caracteres string) de ceros
// y unos, representando ausencia y presencia respectivamente.
double variable_t::Adaptacion(double x, string etiquetas) const {
  if (dominio==0){
    cout << "Variable sin dominio asociado\n";
    exit(1);
  }
  return (*dominio).Adaptacion(x,etiquetas);
}



// Operador que pone en el dispositivo de salida la definici�n de la variable
void variable_t::Pinta() const{
  if (dominio==0){
    cout << "Variable sin dominio asociado\n";
    exit(1);
  }
  cout << "Variable: " << nombre << endl;
  cout << "========================\n";
  (*dominio).Pinta();
}




// Operador que pone en el dispositivo de salida el nombre de la variable
void variable_t::PrintVar() const{
  cout << nombre;
}

// Operador que devuelve en un string el nombre de la variable.
string variable_t::SPrintVar() const{
  return nombre;
}




// Operador que devuelve un string con el nombre de la etiqueta "value" de la variable.
string variable_t::SPrintDomain(int value) const{
  return (*dominio).SPrint(value);
}


// Operador que devuelve el numero de etiquetas que componen el dominio de la variable.
int variable_t::SizeDomain() const{
  return (*dominio).Size();
}

// Operador que devuelve si la etiqueta es considerada en el proceso de aprendizaje.
bool variable_t::Activa()const{
  return activa;
}


//Operador que devuelve si es un variable antecedente en la regla.
bool variable_t::Antecedente() const{
  return antecedente;
}


// Operador que devuelve el n�mero de etiquetas que componen el dominio de la variable.
int variable_t::N_etiquetas() const{
  return (*dominio).N_etiquetas();
}















// Operador que determina si el dominio asociado a la variable esta compuesto exclusivamente
// por intervalos.









// Operador que devuelve un objeto <domain_t> con la definici�n del dominio de la variable
domain_t variable_t::Domain() const{
  domain_t aux;
  aux=(*dominio);
  return aux;
}


// Devuelve una copia de la variable
variable_t variable_t::Variable() const{
  variable_t aux;
  aux=*this;
  return aux;
}









}  // namespace inslv
