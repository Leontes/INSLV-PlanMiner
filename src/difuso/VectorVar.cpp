// SPDX-License-Identifier: GPL-3.0-only
using namespace std;
#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include "inslv/difuso/VectorVar.h"
#include "inslv/comun/progreso.hpp"

namespace inslv {





VectorVar::VectorVar(){
  numero=0;
  lista=0;
  oper=0;
}




void Libera_Memoria_Var_aux (double *&va, double *&vb, double *&vc, double *&vd,
                             string *&vname){
   delete []va;
   delete []vb;
   delete []vc;
   delete []vd;
   delete []vname;
}





// Las variables del conjunto de ejemplos de la API: primero los atributos (antecedentes) y al
// final la clase (consecuente). Las etiquetas no se extienden hasta el infinito (singleton).
VectorVar::VectorVar(const Dataset &data){
  double *va,*vb,*vc,*vd,inf,sup;

  numero = data.attributes.size() + 1;
  lista = new variable_t[numero];
  oper = new combinado[numero];

  for(int v = 0; v < numero; v++){
    const inslv::Variable &variable = v + 1 < numero ? data.attributes[v] : data.target;
    // 0: antecedente; 1: consecuente.
    int estado = v + 1 < numero ? 0 : 1;
    int n = variable.labels.size();

    inf = variable.labels.front().a;
    sup = variable.labels.back().d;

    va = new double[n];
    vb = new double[n];
    vc = new double[n];
    vd = new double[n];
    string *vname = new string[n];

    for(int i = 0; i < n; i++){
      va[i] = variable.labels[i].a;
      vb[i] = variable.labels[i].b;
      vc[i] = variable.labels[i].c;
      vd[i] = variable.labels[i].d;
      vname[i] = variable.labels[i].label;
    }

    // unit = 0, c_factor = 0
    lista[v].Asigna(n, variable.name, estado, 0, 0, inf, sup, va, vb, vc, vd, vname, true);
    oper[v].op=0;
    Libera_Memoria_Var_aux(va,vb,vc,vd,vname);
  }
}


VectorVar::~VectorVar(){
  if (lista!=0){
    delete []lista;
    delete []oper;
  }
  numero=0;
  lista=0;
  oper=0;
}




VectorVar & VectorVar::operator=(const VectorVar &x){
 if (this!=&x){
  if (lista!=0)
    delete [] lista;

  numero=x.numero;
  lista = new variable_t[numero];
  oper = new combinado[numero];
  for (int i=0; i<numero; i++){
    lista[i]=x.lista[i];
    oper[i]=x.oper[i];
  }
 }

  return *this;
}





int VectorVar::N_Antecedente() const {
  int s=0;
  for (int i=0; i<numero; i++)
    if (lista[i].Activa() && lista[i].Antecedente())
      s++;

  return s;
}


void VectorVar::Encode(int &tb, int &te) const {
  tb=0;
  for (int i=0; i<numero; i++)
    if (lista[i].Activa() && lista[i].Antecedente())
      tb+=lista[i].N_etiquetas();
    else  if (lista[i].Activa() && !lista[i].Antecedente())
            te=lista[i].N_etiquetas();
}





void VectorVar::Pinta() const{
  for (int i=0; i<numero; i++){
    progreso() << std::endl;
    lista[i].Pinta();
    progreso() << "----------" << std::endl;
  }
}







void VectorVar::PrintVar(int variable) const{
  lista[variable].PrintVar();
}

string VectorVar::SPrintVar(int variable) const{
  return lista[variable].SPrintVar();
}


string VectorVar::SPrintDomain(int variable, int value) const{
  return lista[variable].SPrintDomain(value);
}

bool VectorVar::IsActiva(int variable) const{
  return lista[variable].Activa();
}


bool VectorVar::IsAntecedente(int variable) const{
  return lista[variable].Antecedente();
}


int VectorVar::TotalVariables() const{
 return numero;
}

int VectorVar::SizeDomain(int variable) const{
  return lista[variable].SizeDomain();
}













double VectorVar::Adaptacion(double x, int variable, int dominio) const {
  if (variable >=0 && variable < numero)
    return lista[variable].Adaptacion(x,dominio);
  else
    throw std::logic_error("inslv: no existe la variable " + std::to_string(variable));
}







double VectorVar::Adaptacion_funcion(vectordouble x, string regla, int *lista_funcion, int tama, double adaptacion_previa) const{
  double max=adaptacion_previa,aux;
  string sub;
  int trozo=0,tam,x1,x2,op;
  double resultado;
  for (int i=0; i<tama && max>0; i++){
    if (lista_funcion[i]!=0){
       tam=lista[lista_funcion[i]-1].N_etiquetas();
       sub=regla.substr(trozo,tam);
       x1 = oper[lista_funcion[i]-1].operando[0];
       x2 = oper[lista_funcion[i]-1].operando[1];
       op = oper[lista_funcion[i]-1].op;
       if (x.At(x1)==-999999999 or x.At(x2)==-999999999)
        resultado= -999999999;
       else if (op==1){
              if (x.At(x1)>=x.At(x2))
                resultado=1;
              else
                resultado=0;
            } else if (op==2)
                      resultado=(x.At(x1))+(x.At(x2));
                   else if (op==3)
                          resultado=x.At(x1)*x.At(x2);
                        else if (op==4)
                               resultado=(x.At(x1))-(x.At(x2));
                             else if (op==5 and x.At(x2)!=0)
                                    resultado = (x.At(x1))/(x.At(x2));
                                  else if (op==5)
                                         return -999999999;
                                       else if (op==19){
					      if ((x.At(x1))==(x.At(x2)))
						resultado=1;
					      else
						resultado=0;
					   }
                                       else if (op==7 or op==8)
                                              resultado=x.At(x1);
                                           else
                                              max=0;

       if (max>0)
         aux=lista[lista_funcion[i]-1].Adaptacion(resultado,sub);

       if (aux<max)
         max=aux;
    }
    //else
        tam=2;

    trozo+=tam;
  }

  return max;
}






int NumeroEtiquetasActivas(string sub, int tam){
  int n=0;
  for (int i=0; i<tam; i++)
    if (sub[i]=='1')
      n++;
  return n;
}

void VectorVar::SecuenciasDeEtiquetasActivas(string sub, int tam, int &unos, int &ceros, int &n_unos) const{
  unos=0;
  ceros=0;
  n_unos=0;
  bool last_uno;
  int i=1;
  if (sub[0]=='0'){
    ceros++;
    last_uno=false;
  }
  else{
    unos++;
    last_uno=true;
    n_unos++;
  }
  while (i<tam){
    if (sub[i]=='1')
      n_unos++;
    if (last_uno && sub[i]=='0'){
      last_uno=false;
      ceros++;
    }
    else if (!last_uno && sub[i]=='1'){
           last_uno=true;
           unos++;
    }
    i++;
  }
}

bool VectorVar::Es_Valida(string regla, double *var, double umbral, double &simplicidad) const{
  string sub;
  int trozo=0,tam, unos, ceros, n_unos;
  int i=0;
  simplicidad=0;
  bool valida=true;
  while (i<numero && valida){
    if (lista[i].Activa() && lista[i].Antecedente()){
      tam=lista[i].N_etiquetas();
      if (var[i]>=umbral){
        sub=regla.substr(trozo,tam);
        //unos = NumeroEtiquetasActivas(sub,tam);
        SecuenciasDeEtiquetasActivas(sub,tam,unos,ceros, n_unos);
        valida=(unos!=0);
        if (valida){
          if (unos==1 || ceros==1)
            simplicidad=simplicidad+1;
        }
      }
      trozo+=tam;
    }
    i++;
  }

  return valida;
}

double VectorVar::Adaptacion(vectordouble x, string regla, double *var, double umbral) const{
  double max=1,aux;
  string sub;
  int trozo=0,tam, unos;
  //cout << "Numero de variables: " << numero << endl;
  for (int i=0; i<numero && max>0; i++){
    if (lista[i].Activa() && lista[i].Antecedente()){
      tam=lista[i].N_etiquetas();
      if (var[i]>=umbral){
        sub=regla.substr(trozo,tam);
        unos = NumeroEtiquetasActivas(sub,tam);
        if (unos==0)
          max=0;
        else if (unos<tam){
               aux=lista[i].Adaptacion(x.At(i),sub);
               if (aux<max)
                 max=aux;
             }
      }
      trozo+=tam;
    }
  }

  return max;
}






















domain_t VectorVar::Domain(int var) const{
  domain_t aux;
  aux = lista[var].Domain();
  return aux;
}

variable_t VectorVar::Variable(int var) const{
  variable_t aux;
  aux = lista[var].Variable();
  return aux;
}





int VectorVar::Consecuente() const {
  int i=0;
  while ((lista[i].Antecedente() || !lista[i].Activa()) && i<numero)
    i++;

  if (i!=numero)
    return i;
  else {
    progreso() << "No hay variable consecuente\n";
    return -1;
  }
}

















void VectorVar::Funcion(int pos, int &x1, int &x2, int &op) const{
    x1 = oper[pos].operando[0];
    x2 = oper[pos].operando[1];
    op = oper[pos].op;
}

}  // namespace inslv
