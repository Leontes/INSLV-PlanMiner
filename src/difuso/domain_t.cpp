// SPDX-License-Identifier: GPL-3.0-only
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include "inslv/difuso/domain_t.h"

namespace inslv {

using namespace std;


// Constructor de clase por defecto
domain_t::domain_t(){
  numero=0;
  rango_inf=rango_sup=0;
  inf_inf=sup_inf=false;
  label=0;
}



// Destructor de la clase
domain_t::~domain_t(){
  numero=0;
  delete []label;
  label=0;
}




// Operador de asignaci�n
domain_t & domain_t::operator=(const domain_t &x){
  if (label!=0)
    delete [] label;
  numero=x.numero;
  rango_inf=x.rango_inf;
  rango_sup=x.rango_sup;
  inf_inf=x.inf_inf;
  sup_inf=x.sup_inf;
  label = new fuzzy_t[numero];
  for (int i=0; i<numero; i++)
     label[i]=x.label[i];

  return *this;
}















// Operaci�n de asignaci�n a un objeto de la clase
// Crea un dominio de n etiquetas en [inf,sup]. La definici�n de cada etiqueta
// que compone el dominio viene definida en 4 vectores a,b,c y d, donde
// [a[i],b[i],c[i],d[i]] representa la etiqueta i-esima del dominio.
// el vector name contiene el nombre de cada etiqueta.
void domain_t::Asigna(int n, double inf, double sup, double *a, double *b, double *c, double *d, string *name, bool singleton){
  numero=n;
  rango_inf=inf;
  rango_sup=sup;
  inf_inf=true;
  sup_inf=true;
  if (label!=0)
    delete label;
  label = new fuzzy_t[numero];


  if(!singleton){
    label[0].Asigna(a[0],b[0],c[0],d[0],name[0],true,false);
    for (int i=1; i<numero-1; i++)
      label[i].Asigna(a[i],b[i],c[i],d[i],name[i]);

    label[numero-1].Asigna(a[numero-1],b[numero-1],c[numero-1],d[numero-1],name[numero-1],false,true);
  }
  else{
    label[0].Asigna(a[0],b[0],c[0],d[0],name[0],false,false);
    for (int i=1; i<numero-1; i++)
      label[i].Asigna(a[i],b[i],c[i],d[i],name[i]);

    label[numero-1].Asigna(a[numero-1],b[numero-1],c[numero-1],d[numero-1],name[numero-1],false,false);
  }
}


// Operaci�n que devuelve el n�mero de etiquetas que contiene un dominio
int domain_t::N_etiquetas() const{
  return numero;
}


// Operaci�n que devuelve el n�mero de etiquetas que contiene un dominio
int domain_t::Size() const {
  return numero;
}





// Operaci�n que devuelve el grado de adaptaci�n de un determinado valor x
// a una determinada etiqueta del dominio referenciada por su posici�n dentro
// del dominio.
double domain_t::Adaptacion(double x, const int etiqueta) const{
  if (numero==0) {
    cout << "El dominio no esta creado\n";
    exit(1);
  }

  if (etiqueta>=numero){
    cout << "Esa etiqueta no esta en el dominio\n";
    exit(1);
  }

  return label[etiqueta].Adaptacion(x);
}



// Operaci�n que evalua el grado de adaptaci�n de un determinado valor x
// a un conjunto de etiquetas del dominio. Las etiquetas se expresan como
// un vector ordenado (cadena de caracteres <string>) de ceros y unos. El cero
// representa ausencia y el uno presencia.
double domain_t::Adaptacion(double x, string etiquetas) const{
  double mayor, nuevo;
  if (numero==0) {
    cout << "El dominio no esta creado\n";
    exit(1);
  }

  mayor=etiquetas.length();
  if (mayor>numero){
    cout << "Esa etiqueta no esta en el dominio\n";
    exit(1);
  }

  mayor=0;
  for (int i=0; i<numero and mayor<1; i++)
    if (etiquetas[i]=='1'){
      if (mayor==0)
        mayor = label[i].Adaptacion(x);
      else if (label[i].Adaptacion(x)>0)
             mayor=1;
    }

  return mayor;
}










/*void domain_t::Amplia_Delta(){
 string nombre;
 double a,b,c,d, centro_anterior;
 const double factor=0.25;

  double crecimiento;

  label[0].GetDefinicion(a,b,c,d,nombre);
  crecimiento=(d-c)*factor;
  label[0].PutDefinicion(a,b,c+crecimiento,d+crecimiento,nombre);

  for (int i=1; i<numero; i++){
    centro_anterior=c+crecimiento;
    label[i].GetDefinicion(a,b,c,d,nombre);
    label[i].PutDefinicion(a+crecimiento,b+crecimiento,c+crecimiento,d+crecimiento,nombre);
  }

  //label[numero-1].GetDefinicion(a,b,c,d,nombre);
  //label[numero-1].PutDefinicion(a+crecimiento,b,c,d+crecimiento,nombre);


  rango_sup=rango_sup+crecimiento;
}
*/










//Operaci�n que pone en el dispositivo de salida la definici�n del dominio
void domain_t::Pinta() const{
  for (int i=0; i<numero; i++)
    label[i].Pinta();
}








string domain_t::SPrint(int i) const {
  return label[i].SPrint();
}























}  // namespace inslv
