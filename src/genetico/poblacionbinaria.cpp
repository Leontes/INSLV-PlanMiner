// SPDX-License-Identifier: GPL-3.0-only
using namespace std;
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include "inslv/genetico/poblacionbinaria.h"
#include "inslv/genetico/operaciones.h"

namespace inslv {









poblacionbinaria::poblacionbinaria(double mut, double cruce, int eli, int n, int tama){
  prob_mutacion=mut;
  prob_cruce=cruce;
  elitismo=eli;
  n_individuos=n;
  tamano= new int[n_individuos];
  for (int i=0; i<n_individuos; i++)
    tamano[i]=tama;

  modificado= new bool[n_individuos];
  for (int i=0; i<n_individuos; i++)
    modificado[i]=true;

  valoracion= new double[n_individuos];

  individuos = new char*[n_individuos];
  for (int i=0; i<n_individuos; i++)
    individuos[i] = new char[tamano[i]];
}












poblacionbinaria::~poblacionbinaria(){
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


char * poblacionbinaria::Individuo(int i, int &tama) const{
  tama = tamano[i];
  return individuos[i];
}



void poblacionbinaria::Pasar(int i, const poblacionbinaria &x, int j){

  if (tamano[i]>0)
    delete[] individuos[i];

  tamano[i]=x.tamano[j];
  individuos[i]= new char[tamano[i]];
  for (int k=0; k<tamano[i]; k++)
    individuos[i][k]=x.individuos[j][k];
}


void poblacionbinaria::Swap(int i, int j){
  Swap_int(tamano[i],tamano[j]);
  Swap_bool(modificado[i],modificado[j]);
  Swap_double(valoracion[i],valoracion[j]);
  char *p=individuos[i];
  individuos[i]=individuos[j];
  individuos[j]=p;
}


void poblacionbinaria::Codigo(int i, char *&v, int &tama) const{

  //if (v!=0)
    //delete [] v;

  tama=tamano[i];
  v=new char[tama];
  for (int j=0;j<tama;j++)
    v[j]=individuos[i][j];
}












void poblacionbinaria::PutValue(int indiv, int bit, char value){
  individuos[indiv][bit]=value;
}



void poblacionbinaria::PoblacionInicialAleatoria(){
  for (int i=0; i<n_individuos; i++)
    for (int j=0; j<tamano[i]; j++)
        individuos[i][j]= (Probabilidad(0.9))?'1':'0';
        //individuos[i][j]= '1';
}





void poblacionbinaria::PoblacionInicial(int **sujeto, int tama, int* tama_dom){
  int j;
  for (int i=0; i<n_individuos; i++){
    j=0;
    for (int k=0; k<tama; k++){
      if (sujeto[i][k]==-1){
        for (int l=0; l<tama_dom[k]; l++){
          individuos[i][j]= (Probabilidad(0.9))?'1':'0';
          j++;
        }
      }
      else {
        for (int l=0; l<tama_dom[k]; l++)
          individuos[i][j+l]='0';
        individuos[i][j+sujeto[i][k]]='1';
        j+=tama_dom[k];
      }
    }
  }
}














void poblacionbinaria::MutacionUniforme_Estacionario(){
  for (int i=n_individuos-2; i<n_individuos; i++){
    for (int j=0; j<tamano[i]; j++){
      if (Probabilidad(prob_mutacion)){
         //cout << "Mutado: " << i << "(" << j <<")"<<endl;
         if (individuos[i][j]=='1')
            individuos[i][j]='0';
        else
            individuos[i][j]='1';
        modificado[i]=true;
      }
    }
  }

}














void poblacionbinaria::Cruce2puntos_Estacionario(int indiv1, int indiv2){
  int a,b,p1,p2;
  char aux;
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

  //}
}

void poblacionbinaria::CruceAND_OR_Estacionario(int indiv1, int indiv2){
  int a,b,p1,p2;
  char aux;
  int i=n_individuos-2;

  PuntoCorte2(tamano[i],p1,p2);
  modificado[i]=true;
  modificado[i+1]=true;
  for (int j=p1; j<p2; j++){
    if (individuos[indiv1][j] =='1' && individuos[indiv2][j] =='1')
       individuos[i][j]='1';
    else
       individuos[i][j]='0';

    if (individuos[indiv1][j] =='1' || individuos[indiv2][j] =='1')
       individuos[i+1][j]='1';
    else
       individuos[i+1][j]='0';
  }
}

void poblacionbinaria::CruceNAND_NOR_Estacionario(int indiv1, int indiv2){
  int a,b,p1,p2;
  char aux;
  int i=n_individuos-2;
      modificado[i]=true;
      modificado[i+1]=true;
      for (int j=0; j<tamano[i]; j++){
        if (individuos[indiv1][j] =='1' && individuos[indiv2][j] =='1')
          individuos[i][j]='0';
        else
          individuos[i][j]='1';

        if (individuos[indiv1][j] =='1' || individuos[indiv2][j] =='1')
          individuos[i+1][j]='0';
        else
          individuos[i+1][j]='1';
      }
}

}  // namespace inslv
