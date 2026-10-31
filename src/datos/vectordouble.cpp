// SPDX-License-Identifier: GPL-3.0-only
using namespace std;
#include <iostream>
#include <iomanip>
#include "inslv/datos/vectordouble.h"
#include "inslv/comun/progreso.hpp"

namespace inslv {


vectordouble::vectordouble(){
  reservado=0;
  numero=0;
    data=0;
}


vectordouble::vectordouble(int tamano){
  reservado=tamano;
  numero=0;
  data = new double[tamano];
}




vectordouble::vectordouble(const vectordouble &x){
  reservado=x.reservado;
  numero=x.numero;
  data = new double[reservado];

  for (int i=0; i<numero; i++)
    data[i]=x.data[i];
}

vectordouble &vectordouble::operator=(const vectordouble &x){
  if (reservado!=0)
    delete []data;

  reservado=x.reservado;
  numero=x.numero;
  data = new double[reservado];

  for (int i=0; i<numero; i++)
    data[i]=x.data[i];

  return *this;
}

vectordouble::~vectordouble(){
  if (reservado!=0)
    delete [] data;
  reservado=0;
  numero=0;
  data=0;
}








void vectordouble::Put(double *x, int tamano){
  if (reservado!=0)
    delete []data;

  reservado=tamano;
  numero=tamano;
  data = new double[tamano];

  for (int i=0; i< tamano; i++)
    data[i]=x[i];
}





double vectordouble::At(int pos) const {
  if (pos<0 || pos>numero){
    progreso() << "No existe esa posicion" << pos << endl;
    return -999999999; /*devuelve un error*/
  }
  else
    return data[pos];
}























}  // namespace inslv
