// SPDX-License-Identifier: GPL-3.0-only
using namespace std;
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include "inslv/genetico/poblacionentera.h"
#include "inslv/genetico/operaciones.h"

namespace inslv {










poblacionentera::poblacionentera(int rang, double mut, double cruce, int eli, int n, int tama){
  prob_mutacion=mut;
  prob_cruce=cruce;
  elitismo=eli;
  n_individuos=n;
  rango=rang;
  tamano= new int[n_individuos];
  for (int i=0; i<n_individuos; i++)
    tamano[i]=tama;

  modificado= new bool[n_individuos];
  for (int i=0; i<n_individuos; i++)
    modificado[i]=true;

  valoracion= new double[n_individuos];

  individuos = new int*[n_individuos];
  for (int i=0; i<n_individuos; i++)
    individuos[i] = new int[tamano[i]];
}









poblacionentera::~poblacionentera(){
  if (n_individuos>0){
    delete [] tamano;
    delete [] modificado;
    delete [] valoracion;
    for (int i=0; i<n_individuos; i++)
      if (individuos[i]!=0){
        delete [] individuos[i];
	individuos[i]=0;
      }
    delete [] individuos;
  }

  n_individuos=0;
}

int * poblacionentera::Individuo(int i, int &tama) const{
  tama = tamano[i];
  return individuos[i];
}



void poblacionentera::Pasar(int i, const poblacionentera &x, int j){
  if (tamano[i]>0)
    delete[] individuos[i];

  tamano[i]=x.tamano[i];
  individuos[i]= new int[tamano[i]];
  for (int k=0; k<tamano[i]; k++)
    individuos[i][k]=x.individuos[j][k];
}


void poblacionentera::Swap(int i, int j){
  Swap_int(tamano[i],tamano[j]);
  Swap_bool(modificado[i],modificado[j]);
  Swap_double(valoracion[i],valoracion[j]);
  int *p=individuos[i];
  individuos[i]=individuos[j];
  individuos[j]=p;
}



void poblacionentera::Codigo(int i, int *&v, int &tama) const{
  //if (v!=0)
    //delete [] v;

  tama=tamano[i];
  v=new int[tama];
  for (int j=0;j<tama;j++)
    v[j]=individuos[i][j];
}












void poblacionentera::PutValue(int indiv, int bit, int value){
  individuos[indiv][bit]=value;
}

int poblacionentera::GetValue(int indiv, int bit) const{
  return individuos[indiv][bit];
}

void poblacionentera::PoblacionInicialAleatoria(){
  for (int i=0; i<n_individuos; i++)
    for (int j=0; j<tamano[i]; j++)
        individuos[i][j]= i%rango;
}

void poblacionentera::PoblacionInicialAleatoria(int consecuente){
  for (int i=0; i<n_individuos; i++)
    for (int j=0; j<tamano[i]; j++)
        individuos[i][j]= consecuente;
}














void poblacionentera::MutacionUniforme_Estacionario(){
  int aux;
  for (int i=n_individuos-2; i<n_individuos; i++)
    for (int j=0; j<tamano[i]; j++)
      if (Probabilidad(prob_mutacion)){
        do{
          aux=rand()%rango;
        }while (aux==individuos[i][j]);

	individuos[i][j]=aux;
	modificado[i]=true;
      }
}


void poblacionentera::MutacionUniformeMod_Estacionario(double prob0){
  int aux,j;
  if (rango>1){
    for (int i=n_individuos-2; i<n_individuos; i++){
        if (Probabilidad(prob0)){
            j=rand()%tamano[i];
            if (individuos[i][j]!=0){
                individuos[i][j]=0;
            }
            else{
                aux=rand()%rango;
                individuos[i][j]=aux;
            }
            modificado[i]=true;
        }
    }
  }
}



void poblacionentera::MutacionUniformeMod_Estacionario_Especial(){
  int aux,j;
  if (rango>1){
    for (int i=n_individuos-2; i<n_individuos; i++){
      int intentos=0;
      j=rand()%tamano[i];
      while (intentos<5 and individuos[i][j]==0){
         intentos++;
         j=rand()%tamano[i];
      }
      if (intentos<5){
            if (individuos[i][j]!=0){
                individuos[i][j]=0;
            }
            else{
                aux=rand()%rango;
                individuos[i][j]=aux;
            }
            modificado[i]=true;
        }
    }
  }
}







/* Solo vale cuando la poblacion entera tiene un unico individuo */
void poblacionentera::Cruce2puntos_Estacionario(int indiv1, int indiv2){
  int a,b,p1,p2;
  int aux;
  int i=n_individuos-2;
  //for (int i=n_individuos-2; i<n_individuos; i++){
      //a=Select_Random_Individuo(n_individuos,i,0);
      //b=Select_Random_Individuo(n_individuos,i,0);
      PuntoCorte2(tamano[indiv1],p1,p2);
      modificado[indiv1]=true;
      modificado[indiv2]=true;
      for (int j=p1; j<p2; j++){
        aux=individuos[indiv2][j];
        individuos[indiv2][j]=individuos[indiv1][j];
        individuos[indiv1][j]=aux;
      }
}

}  // namespace inslv
