// SPDX-License-Identifier: GPL-3.0-only
/***************************************************************************
                          poblacionreal.cpp  -  description
                             -------------------
    begin                : Tue Apr 30 2002
    copyright            : (C) 2002 by Ral P�ez
    email                : fgr@decsai.ugr.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
using namespace std;
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "inslv/genetico/poblacionreal.h"
#include "inslv/comun/progreso.hpp"
#include "inslv/genetico/operaciones.h"

namespace inslv {











poblacionreal::poblacionreal(double raninf, double ransup, double mut, double cruce, int eli, int n, int tama){
  prob_mutacion=mut;
  prob_cruce=cruce;
  elitismo=eli;
  n_individuos=n;
  rango_i=raninf;
  rango_s=ransup;
  tamano= new int[n_individuos];
  for (int i=0; i<n_individuos; i++)
    tamano[i]=tama;

  modificado= new bool[n_individuos];
  for (int i=0; i<n_individuos; i++)
    modificado[i]=true;

  individuos = new double*[n_individuos];
  for (int i=0; i<n_individuos; i++)
    individuos[i] = new double[tamano[i]];
}











poblacionreal::~poblacionreal(){
  if (n_individuos>0){
    delete [] tamano;
    delete [] modificado;
    for (int i=0; i<n_individuos; i++)
      if (individuos[i]!=0){
        delete [] individuos[i];
	      individuos[i]=0;
      }
    delete [] individuos;
  }

  n_individuos=0;
}

double* poblacionreal::Individuo(int i, int &tama) const{
  tama = tamano[i];
  return individuos[i];
}



void poblacionreal::Pasar(int i, const poblacionreal &x, int j){
  if (tamano[i]>0)
    delete[] individuos[i];

  tamano[i]=x.tamano[i];
  individuos[i]= new double[tamano[i]];
  for (int k=0; k<tamano[i]; k++)
    individuos[i][k]=x.individuos[j][k];
}


void poblacionreal::Swap(int i, int j){
  Swap_int(tamano[i],tamano[j]);
  Swap_bool(modificado[i],modificado[j]);
  double *p=individuos[i];
  individuos[i]=individuos[j];
  individuos[j]=p;
}



void poblacionreal::Codigo(int i, double *&v, int &tama) const{
  //if (v!=0)
    //delete [] v;

  tama=tamano[i];
  v=new double[tama];
  for (int j=0;j<tama;j++)
    v[j]=individuos[i][j];
}











void poblacionreal::PutValue(int indiv, int bit, double value){
  individuos[indiv][bit]=value;
}







void poblacionreal::PoblacionInicialAleatoria(int n_item){
  for (int i=0; i<n_individuos; i++)
    individuos[i][tamano[i]-1]=n_item;
}











void poblacionreal::PoblacionInicial(double **I, int clases){
  double *max = new double[clases], *min = new double[clases];

  double mayor, menor;
progreso(Verbosidad::depuracion) << "CL " << tamano[0] - 1 << endl;
  for (int i=0; i<clases; i++){
    mayor=I[0][i+1];
    menor=I[0][i+1];
progreso(Verbosidad::depuracion) << "I " << I[0][i+1] << endl;
    for (int j=1; j<tamano[0]-1; j++){
      if (I[j][i+1]>mayor)
        mayor=I[j][i+1];
      else if (I[j][i+1]<menor)
             menor=I[j][i+1];

      max[i]=mayor;
      min[i]=menor;
    }
  }

  for (int i=0; i<n_individuos; i++) {
    for (int j=0; j<tamano[i]-1; j++)
      if (min[i%clases]==max[i%clases])
        individuos[i][j]=((1.0*rand())/(RAND_MAX+1.0));
      else
        individuos[i][j]=I[j][(i%clases)+1];

    //individuos[i][tamano[i]-1]=(max[i%clases]-min[i%clases])*(1.0*rand())/(RAND_MAX+1.0);
    individuos[i][tamano[i]-1]=0.2;
  }
  delete [] max;
  delete [] min;
}




















void poblacionreal::MutacionUniforme_Estacionario(){
  double aux;
  int pos;
  for (int i=n_individuos-2; i<n_individuos; i++){
    for (int k=0; k<tamano[i]-1; k++){
      //pos = (int) (1.0*tamano[i]*rand()/(RAND_MAX+1.0));
      if (Probabilidad(prob_mutacion)){
         //individuos[i][k]= 1.0*rand()/(RAND_MAX+1.0);
         //individuos[i][k] = 1.0 - individuos[i][k];
         if (individuos[i][k]<individuos[i][tamano[i]-1])
            //individuos[i][k]=(1.0*(1-individuos[i][tamano[i]-1])*rand())/(RAND_MAX+1.0)+individuos[i][tamano[i]-1];
            individuos[i][k]=individuos[i][tamano[i]-1]+0.01;
         else
            //individuos[i][k]=(1.0*(individuos[i][tamano[i]-1])*rand())/(RAND_MAX+1.0);
            individuos[i][k]=individuos[i][tamano[i]-1]-0.01;

         if (individuos[i][k]>1)
           individuos[i][k]=1;
         else if (individuos[i][k]<0)
                individuos[i][k]=0;
      }

      //individuos[i][pos]=(1.0*rand())/(RAND_MAX+1.0);
    }
    modificado[i]=true;
  }
}











void poblacionreal::Cruce2puntos_Estacionario(int indiv1, int indiv2){
  int a,b,p1,p2;
  double aux;
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
  //}
}


void poblacionreal::CruceAND_OR_Estacionario(int indiv1, int indiv2){
  int a,b,p1,p2;
  double aux;
  int i=n_individuos-2;
  //for (int i=n_individuos-2; i<n_individuos; i++){
      //a=Select_Random_Individuo(n_individuos,i,0);
      //b=Select_Random_Individuo(n_individuos,i,0);
      PuntoCorte2(tamano[i]-1,p1,p2);
      modificado[i]=true;
      modificado[i+1]=true;
      for (int j=p1; j<=p2; j++){
	if (individuos[indiv1][j]< individuos[indiv2][j]){
	       individuos[i][j]=individuos[indiv2][j];
	       individuos[i+1][j]=individuos[indiv1][j];
        }
        else {
	       individuos[i][j]=individuos[indiv1][j];
	       individuos[i+1][j]=individuos[indiv2][j];
        }
      }
  //}
}


void poblacionreal::CruceNAND_NOR_Estacionario(int indiv1, int indiv2){
  int a,b,p1,p2;
  double aux;
  int i=n_individuos-2;
  //for (int i=n_individuos-2; i<n_individuos; i++){
      //a=Select_Random_Individuo(n_individuos,i,0);
      //b=Select_Random_Individuo(n_individuos,i,0);
      PuntoCorte2(tamano[i]-1,p1,p2);
      modificado[i]=true;
      modificado[i+1]=true;
      for (int j=p1; j<=p2; j++){
	if (individuos[indiv1][j]< individuos[indiv2][j]){
	       individuos[i][j]=1-individuos[indiv2][j];
	       individuos[i+1][j]=1-individuos[indiv1][j];
        }
        else {
	       individuos[i][j]=1-individuos[indiv1][j];
	       individuos[i+1][j]=1-individuos[indiv2][j];
        }
      }
  //}
}

}  // namespace inslv
