// SPDX-License-Identifier: GPL-3.0-only
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <cmath>
#include "inslv/genetico/multipoblacion.h"
#include "inslv/comun/progreso.hpp"
#include "inslv/genetico/operaciones.h"
#include "inslv/datos/example_set.h"

namespace inslv {

using namespace std;




multipoblacion::multipoblacion(int pbin, int pent, int preal, int *rangoe,
                               double *rangori, double *rangors, double *mut, double *cruce,
                               int eli, int n, int *tama, int n_val){

  elitismo=eli;
  n_individuos=n;
  poblacionesB=pbin;
  poblacionesE=pent;
  poblacionesR=preal;
  if (pbin>0){
    Pb= new poblacionbinaria*[pbin];
    for (int j=0; j<pbin; j++)
      Pb[j]= new poblacionbinaria(mut[j], cruce[j], eli, n, tama[j]);
  }

  if (pent>0){
    Pe= new poblacionentera*[pent];
    for (int j=0; j<pent; j++)
      Pe[j]= new poblacionentera(rangoe[j], mut[j+pbin], cruce[j+pbin], eli, n, tama[j+pbin]);
  }

  if (preal>0){
    Pr= new poblacionreal*[preal];
    for (int j=0; j<preal; j++)
      Pr[j]= new poblacionreal(rangori[j], rangors[j], mut[j+pbin+pent], cruce[j+pbin+pent], eli, n, tama[j+pbin+pent]);
  }


  modificado= new bool[n_individuos];
  for (int i=0; i<n_individuos; i++)
    modificado[i]=true;

  n_valoracion=n_val;
  valoracion = new double *[n_individuos];
  for (int i=0; i<n_individuos; i++){
     valoracion[i]= new double[n_valoracion];
     for (int j=0; j<n_valoracion; j++)
       valoracion[i][j]=0;
  }
}












multipoblacion::~multipoblacion(){
  if (n_individuos>0){
    if (modificado!=0)
      delete [] modificado;
    if (valoracion!=0){
      // El constructor reserva una fila por individuo (en el original se liberaban n_valoracion).
      for (int i=0; i<n_individuos; i++)
        delete [] valoracion[i];
      delete [] valoracion;
    }
  }

  if (poblacionesB>0){
    for (int i=0; i<poblacionesB; i++)
      delete Pb[i];
    delete [] Pb;
  }

  if (poblacionesE>0){
    for (int i=0; i<poblacionesE; i++)
      delete Pe[i];
    delete []Pe;
  }

  if (poblacionesR>0){
    for (int i=0; i<poblacionesR; i++)
      delete Pr[i];
    delete [] Pr;
  }

  elitismo=0;
  poblacionesB=0;
  poblacionesE=0;
  n_individuos=0;
  modificado=0;
  n_valoracion=0;
  valoracion=0;
  Pb=0;
  Pe=0;
}


void multipoblacion::Swap(int i, int j){

  for (int k=0; k<poblacionesB; k++)
     Pb[k][0].Swap(i,j);

  for (int k=0; k<poblacionesE; k++)
     Pe[k][0].Swap(i,j);

  for (int k=0; k<poblacionesR; k++)
     Pr[k][0].Swap(i,j);

  Swap_bool(modificado[i],modificado[j]);

  double *p=valoracion[i];
  valoracion[i]=valoracion[j];
  valoracion[j]=p;
}


void multipoblacion::Pasar(int i, int j){
    for (int k=0; k<poblacionesB; k++){
       Pb[k][0].Pasar(i,this->Pb[k][0],j);
    }

    for (int k=0; k<poblacionesE; k++)
       Pe[k][0].Pasar(i,this->Pe[k][0],j);

    for (int k=0; k<poblacionesR; k++)
       Pr[k][0].Pasar(i,this->Pr[k][0],j);

}



void multipoblacion::Sort(){
  int k;
  for (int i=0; i<n_individuos-1; i++)
    for (int j=n_individuos-1; j>i; j--){
      k=0;
      while (k<n_valoracion && valoracion[j][k]==valoracion[j-1][k])
        k++;
      if (k<n_valoracion && valoracion[j][k]>valoracion[j-1][k])
        Swap(j,j-1);
    }
}


int multipoblacion::ClaseIndividuo(int ind) const{
  return Pe[0][0].GetValue(ind,0);
}


char* multipoblacion::Subpoblacion_Binaria(int subpoblacion, int individuo, int &tama) const{
  char *puntero;
  puntero=Pb[subpoblacion][0].Individuo(individuo,tama);
  return puntero;
}

int* multipoblacion::Subpoblacion_Entera(int subpoblacion, int individuo, int &tama) const{
  int *puntero;
  puntero=Pe[subpoblacion][0].Individuo(individuo,tama);
  return puntero;
}


double* multipoblacion::Subpoblacion_Real(int subpoblacion, int individuo, int &tama) const{
  double *puntero;
  puntero=Pr[subpoblacion][0].Individuo(individuo,tama);
  return puntero;
}








void multipoblacion::Better(int nclases, int *better, int &nbetter, int **ind_clase) const{

  nbetter=1;
  better[0]=0;
  /*for (int i=0; i<nclases; i++){
    if ( ind_clase[i][0]>1 && ind_clase[i][1]!=0 && valoracion[ind_clase[i][1]][0]>0) {
       //if (ValorIndividuo(1,i)>1) {
         better[nbetter]=ind_clase[i][1];
         nbetter++;
       //}
    }
  }*/
}



bool Mayor (double *v1, double *v2, int n){
  int k=0;
  while (k<n && v1[k]==v2[k])
        k++;

  if (k==n)
    return false;
  else
    return v1[k]>v2[k];
}

void multipoblacion::Sort_menos2ultimos(){
  int k;
  bool cambio=true;
  for (int i=0; cambio and i<n_individuos-3; i++){
    cambio=false;
    for (int j=n_individuos-3; j>i; j--){
      k=0;
      while (k<n_valoracion && valoracion[j][k]==valoracion[j-1][k])
        k++;
      if (k<n_valoracion && valoracion[j][k]>valoracion[j-1][k]){
        Swap(j,j-1);
        cambio=true;
      }
    }
  }
}



double multipoblacion::Sort_4L(example_set &E, int n_clases, int *n_examples_per_class, int **ind_clase, double *adap_reglas, double *peso_reglas, int n_examples){
  int k;
  int *cl,ncl;
  double acierto=0, acierto_old=0, fallo=0, fallo_old=0;

  // Reservo espacio para guardar infomacion sobre los valores de fitness
  int numero_ejemplos;
  //double vaux[3];

  // Comprobar si los individuos encontrados mejoran al lider de su subpoblacion
  // Si alguno mejora al lider, entonces reordenar la poblacion.
  // Si no mejora, entonces no es necesario reordenar y basta con aplicar el cubrimiento
  // sobre las subpoblaciones.

//  double *adaptacion, *agregacion, *disparada, *peso_agregacion, *mejor_valoracion;
//  agregacion= new double[n_examples];
//  disparada = new double[n_examples];
//  peso_agregacion= new double[n_examples];
//  mejor_valoracion= new double[n_individuos];
//  for (int l=0; l<n_examples; l++){
//    agregacion[l]=adap_reglas[l];
//    peso_agregacion[l]=peso_reglas[l];
//    disparada[l]=-1;
//  }


  // Ordeno la poblacion asumiendo que solo se han modificado los 2 ultimos individuos.
  // Ordenacion por insercion
  for (int i=n_individuos-2; i<n_individuos; i++){
    k=i;
    while (k>0 && Mayor(valoracion[k],valoracion[k-1],n_valoracion)){
       Swap(k,k-1);
       k--;
    }
  }

  // Calculo el numero de ejemplos total
  numero_ejemplos=0;
  for (int i=0; i<n_clases; i++){
    numero_ejemplos+=n_examples_per_class[i];
  }



// ------------------------------------------------------------------------------- */
  // Recalculo de nuevo las supoblaciones despues de la ultima ordenacion
  for (int i=0; i<n_clases; i++)
    ind_clase[i][0]=0;

  for (int i=0; i<n_individuos-2; i++){
    cl=Subpoblacion_Entera(0,i,ncl);
    ind_clase[cl[0]][0]++;
    ind_clase[cl[0]][ind_clase[cl[0]][0]]=i;
  }

  /*progreso() << "\t(" << ind_clase[0][0];
  for (int i=1; i<n_clases; i++)
    progreso() << ", " << ind_clase[i][0];
  progreso() << ")\n";*/



  int num_medio_individuos_por_clase = (n_individuos-2)/(1.0*n_clases)-1;

  int num_min_individuos_por_clase = num_medio_individuos_por_clase ;
  //if (num_min_individuos_por_clase >= num_medio_individuos_por_clase || num_min_individuos_por_clase<0)
    //num_min_individuos_por_clase = 0;

  /* Reglas del tamaño minimo de las poblaciones */
  int *PobMinClase = new int[n_clases];
   for (int k=0;k<n_clases;k++){
     if (n_examples_per_class[k]/(1.0*numero_ejemplos)<0.05)
       PobMinClase[k]=num_min_individuos_por_clase;
     else if (ind_clase[k][0]>0 && fabs(valoracion[ind_clase[k][1]][0]-valoracion[ind_clase[k][ind_clase[k][0]]][0] )<0.1)
            PobMinClase[k]=num_min_individuos_por_clase;
          else
            PobMinClase[k]=num_min_individuos_por_clase;
   }

//  for (int i=0; i<n_clases; i++)
//    PobMinClase[i]=n_individuos/(1+n_clases);

  bool usados_2_ultimos=true;

  /* Fijar los individuos que seran reemplazados */
  for (int i=n_individuos-2; i<n_individuos; i++){
    k = ClaseIndividuo(i);
    if (ind_clase[k][0]<=PobMinClase[k] ){
      usados_2_ultimos=false;
      int j=n_individuos-3;
      while (ind_clase[ClaseIndividuo(j)][0]<=PobMinClase[ClaseIndividuo(j)])
        j--;

      ind_clase[ClaseIndividuo(i)][ind_clase[ClaseIndividuo(i)][0]+1]=ind_clase[ClaseIndividuo(j)][ind_clase[ClaseIndividuo(j)][0]];
      ind_clase[ClaseIndividuo(i)][0]++;
      ind_clase[ClaseIndividuo(j)][0]--;
      Swap(i,j);
    }
  }

  if (!usados_2_ultimos)
    Sort_menos2ultimos();

  delete [] PobMinClase;

  return 0.0;
}































void multipoblacion::PintaFitness_Estacionario(int i) const{
  double minf0,minf1,minf2,maxf0,maxf1,maxf2;
  progreso() << "Fitnes: ";
  for (int j=0; j<n_valoracion; j++)
    progreso() << valoracion[i][j] << "   ";
  //cout << "[" << MediaFitness_Estacionario(minf0,maxf0,minf1,maxf1,minf2,maxf2) << "]";
  //cout << "(" << minf0 << "," << maxf0 <<") , ("<<minf1 <<","<<maxf1<<") , ("<<minf2 << "," << maxf2 <<") ";
  progreso() << "Clase => " << ClaseIndividuo(i);
  progreso() << endl;
}


/*void multipoblacion::PintaIndividuo(int i) const{
  for (int j=0; j<poblacionesR; j++)
    Pr[j][0].PintaBin(i);
  for (int j=0; j<poblacionesE; j++)
    Pe[j][0].Pinta(i);
  for (int j=0; j<poblacionesB; j++)
    Pb[j][0].Pinta(i);
  progreso() << "Fitnes: ";
  for (int j=0; j<n_valoracion; j++)
    progreso() << valoracion[i][j] << " ";
  progreso() << endl <<endl;

}*/





int multipoblacion::N_individuos() const {
  return n_individuos;
}


int multipoblacion::N_Valoracion() const{
  return n_valoracion;
}

void multipoblacion::Codigo(int i, genetcode &code) const{
 // Paso a code la parte binaria
  char **bin=new char*[poblacionesB];
  for (int j=0; j<poblacionesB; j++)
     bin[j]=0;
  int *tb=new int[poblacionesB];
  for (int j=0;j<poblacionesB; j++)
    Pb[j][0].Codigo(i,bin[j],tb[j]);

  code.PutBinary(poblacionesB,tb,bin);

  delete [] tb;
  for (int j=0; j<poblacionesB; j++)
    delete [] bin[j];
  delete [] bin;

  // Paso a code la parte entera
  int **ent=new int*[poblacionesE];
  for (int j=0; j<poblacionesE; j++)
   ent[j]=0;
  int *te=new int[poblacionesE];
  for (int j=0;j<poblacionesE; j++)
    Pe[j][0].Codigo(i,ent[j],te[j]);

  code.PutInteger(poblacionesE,te,ent);

  delete [] te;
  for (int j=0; j<poblacionesE; j++)
    delete [] ent[j];
  delete [] ent;

  // Paso a code la parte real
  double **rea=new double*[poblacionesR];
  for (int j=0; j<poblacionesR; j++)
    rea[j]=0;
  int *tr=new int[poblacionesR];
  for (int j=0;j<poblacionesR; j++)
    Pr[j][0].Codigo(i,rea[j],tr[j]);

  code.PutReal(poblacionesR,tr,rea);

  delete [] tr;
  for (int j=0; j<poblacionesR; j++)
    delete [] rea[j];
  delete [] rea;

}



void multipoblacion::PutCodigo(int i, const genetcode &code){
 // Paso a code la parte binaria
  for (int j=0; j<poblacionesB; j++)
    for (int k=0; k<code.TamaBinary(j); k++)
       Pb[j][0].PutValue(i,k,code.GetValueBinary(j,k));


  // Paso a code la parte entera
  for (int j=0; j<poblacionesE; j++)
    for (int k=0; k<code.TamaInteger(j); k++)
       Pe[j][0].PutValue(i,k,code.GetValueInteger(j,k));


  // Paso a code la parte real
  for (int j=0; j<poblacionesR; j++)
    for (int k=0; k<code.TamaReal(j); k++)
       Pr[j][0].PutValue(i,k,code.GetValueReal(j,k));

}




bool multipoblacion::Modificado(int i) const {
  return modificado[i];
}

void multipoblacion::PutModificado(int i){
  modificado[i]=true;
}

void multipoblacion::Valoracion(int i, double *valor){
  for (int j=0; j<n_valoracion; j++)
    valoracion[i][j]=valor[j];
  modificado[i]=false;
}


















void multipoblacion::PoblacionInicial(double **I, int rango, int n_items, int **sujetos, int n_var, int *n_etiquetas_variable){
  // Nivel de Valor
  Pb[0][0].PoblacionInicial(sujetos, n_var, n_etiquetas_variable);
  Pb[1][0].PoblacionInicialAleatoria();

  // Consecuente
  Pe[0][0].PoblacionInicialAleatoria();
  // Dominancia
  Pe[1][0].PoblacionInicialAleatoria(0);

  // Nivel de Variable
  Pr[0][0].PoblacionInicial(I,rango);
  //Conjunto de ejemplos cubiertos
  Pr[1][0].PoblacionInicialAleatoria(n_items);

  for (int j=0; j<n_individuos; j++)
    modificado[j]=true;

  for (int j=0; j<n_individuos; j++)
    for (int k=0; k<n_valoracion; k++)
      valoracion[j][k]=0;
}




























void multipoblacion::MutacionUniforme_Estacionario_Especial(){
  if (Probabilidad(0.5)){
    Pe[1][0].MutacionUniformeMod_Estacionario_Especial();
  }
  else if (Probabilidad(0.5)){
          Pr[0][0].MutacionUniforme_Estacionario();
       }
}






void multipoblacion::MutacionUniforme_Estacionario(){

bool modificado_alguno=false;

 while (!modificado_alguno){

  if (Probabilidad(0.5)) {
    Pb[0][0].MutacionUniforme_Estacionario();
    modificado_alguno=true;
  }

  if (Probabilidad(0.5)){
    Pb[1][0].MutacionUniforme_Estacionario();
    modificado_alguno=true;
  }

  if (Probabilidad(0.5)){
    Pe[0][0].MutacionUniforme_Estacionario();
    modificado_alguno=true;
  }

  if (Probabilidad(0.8)){
    Pe[1][0].MutacionUniformeMod_Estacionario(0.2);
    modificado_alguno=true;
  }


  if (Probabilidad(0.8)){
    Pr[0][0].MutacionUniforme_Estacionario();
    modificado_alguno=true;
  }

  //Pr[2][0].MutacionUniforme_Estacionario_Normal();
 }

}

















void multipoblacion::CruceBasedLogical_Estacionario(double it){
int a,b;
  if (Probabilidad(1-it)){
    a=Select_Random_Individuo(n_individuos-2,-1,0);
    b=Select_Random_Individuo(n_individuos-2,a,0);
  } else {
    a=0;
    b=Select_Random_Individuo(n_individuos-2,a,0);
  }

  Pasar(n_individuos-2,a);
  Pasar(n_individuos-1,b);

//cout <<"Cruce de : " << a << " y b: " << b << endl;

  if (Probabilidad(1.0)){
  // Con 0.5 se aplica un cruce sobre 2 puntos.
   modificado[n_individuos-2]=false;
   while (!modificado[n_individuos-2]){
    if (Probabilidad(0.25)){
      Pb[0][0].Cruce2puntos_Estacionario(n_individuos-2,n_individuos-1);
      modificado[n_individuos-2]=true;
      modificado[n_individuos-1]=true;
    }
    if (Probabilidad(0.25)){
      Pb[1][0].Cruce2puntos_Estacionario(n_individuos-2,n_individuos-1);
      modificado[n_individuos-2]=true;
      modificado[n_individuos-1]=true;
    }
    if (Probabilidad(0.25)){
      Pe[1][0].Cruce2puntos_Estacionario(n_individuos-2,n_individuos-1);
      modificado[n_individuos-2]=true;
      modificado[n_individuos-1]=true;
    }
    if (Probabilidad(0.25)){
      Pr[0][0].Cruce2puntos_Estacionario(n_individuos-2,n_individuos-1);
      modificado[n_individuos-2]=true;
      modificado[n_individuos-1]=true;
    }
   }
  }
  else if (ClaseIndividuo(a)==ClaseIndividuo(b)){
      // Si son de la misma clase se aplica el operador AND_OR
        Pb[0][0].CruceAND_OR_Estacionario(a,b);

        Pe[0][0].Cruce2puntos_Estacionario(a,b);

        Pr[0][0].CruceAND_OR_Estacionario(a,b);

      }
       else {
       // Si son de clases distintas se aplica el operador NAND_NOR
          Pb[0][0].CruceNAND_NOR_Estacionario(a,b);

          Pe[0][0].Cruce2puntos_Estacionario(a,b);

          Pr[0][0].CruceNAND_NOR_Estacionario(a,b);

       }

  modificado[n_individuos-2]=true;
  modificado[n_individuos-1]=true;
}










double multipoblacion::ValorFitness(int i) const{
  return valoracion[i][0];
}

double multipoblacion::ValorFitness(int i, int j) const{
  return valoracion[i][j];
}


















}  // namespace inslv
