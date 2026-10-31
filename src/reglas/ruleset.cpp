// SPDX-License-Identifier: GPL-3.0-only
/***************************************************************************
                          ruleset.cpp  -  description
                             -------------------
    begin                : Mon May 6 2002
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
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "inslv/reglas/ruleset.h"

namespace inslv {

using namespace std;


// Constructor por defecto de la clase
ruleset::ruleset(){
 reservado=10;
 n_rule=0;
 rule= new genetcode *[reservado];
 peso= new double[reservado];
 domain=0;
 domain_funcion=0;
 n_mas = new double[reservado];
 n_menos= new double[reservado];
 modificada = new bool [reservado];
}





// Destructor de la clase
ruleset::~ruleset(){
 for (int i=0; i<n_rule; i++)
   delete rule[i];

 delete [] rule;
 delete [] peso;
 delete [] n_mas;
 delete [] n_menos;
 delete [] modificada;
 if (domain!=0)
    delete domain;
 if (domain_funcion!=0)
    delete domain_funcion;
 reservado=n_rule=0;
}


ruleset& ruleset::operator=(const ruleset& x)
{
  if (this!=&x){
    if (reservado>0){
      for (int i=0; i<n_rule; i++)
	delete rule[i];

      delete [] rule;
      delete [] peso;
      delete [] n_mas;
      delete [] n_menos;
      delete [] modificada;
      if (domain!=0)
	  delete domain;
      if (domain_funcion!=0)
	  delete domain_funcion;
    }

    reservado=x.reservado;
    n_rule=x.n_rule;
    rule = new genetcode*[reservado];
    peso= new double[reservado];
    n_mas = new double[reservado];
    n_menos= new double[reservado];
    modificada = new bool [reservado];
    for (int i=0; i<n_rule; i++){
	rule[i] = new genetcode;
	rule[i][0] = x.rule[i][0];
	peso[i] = x.peso[i];
	n_mas[i]=x.n_mas[i];
	n_menos[i]=x.n_menos[i];
	modificada[i]=x.modificada[i];
    }
    domain = new VectorVar;
    domain[0] = x.domain[0];
    domain_funcion= new VectorVar;
    domain_funcion[0]= x.domain_funcion[0];

    return *this;
  }

  return *this;
}


// Añadir el dominio a un conjunto de reglas
// Precondicion: Usar solo una vez sobre un ruleset por el constructor por defecto
void ruleset::AddDomain(const VectorVar &x){
  domain = new VectorVar;
  domain[0]=x;
}

void ruleset::AddDomain_Funcion(const VectorVar &x){
  domain_funcion = new VectorVar;
  domain_funcion[0]=x;
}

// Operador de reubicaci� de un conjunto de reglas.
void ruleset::Realloc(){
  genetcode **aux = new genetcode*[2*reservado+1];
  double *p = new double[2*reservado+1];
  double *mas = new double[2*reservado+1];
  double *menos = new double[2*reservado+1];
  bool *m = new bool[2*reservado+1];

  for (int i=0; i<n_rule; i++){
    aux[i]=rule[i];
    rule[i]=0;
    p[i]=peso[i];
    mas[i]=n_mas[i];
    menos[i]=n_menos[i];
    m[i]=modificada[i];
  }
  delete [] rule;
  delete [] peso;
  delete [] n_mas;
  delete [] n_menos;
  delete [] modificada;
  rule = aux;
  peso = p;
  n_mas = mas;
  n_menos = menos;
  modificada = m;
  reservado=2*reservado+1;
}

// Aniade al principio del ruleset el conjunto de reglas incluido como argumento.
void ruleset::AddRuleset(ruleset &r){
  int i=0;
  while (n_rule+r.n_rule>=reservado){
    i++;
    //cout << "Realojo " << i << endl;
    Realloc();
  }

  for (int i=n_rule-1; i>=0; i--){
    rule[i+r.n_rule]=rule[i];
    peso[i+r.n_rule]=peso[i];
    n_mas[i+r.n_rule]=n_mas[i];
    n_menos[i+r.n_rule]=n_menos[i];
    modificada[i+r.n_rule]=modificada[i];


    //cout << "muevo la regla " << i << " a la posicion " << i+r.n_rule << endl;
  }
  n_rule=n_rule+r.n_rule;

  for (int i=0; i<r.n_rule; i++){
    rule[i]= new genetcode;
    genetcode *x = r.Get_Code_Rule(i);
    rule[i][0]= x[0];
    peso[i]=r.peso[i];
    n_mas[i]=r.n_mas[i];
    n_menos[i]=r.n_menos[i];
    modificada[i]=r.peso[i];
//cout << "Introducida la regla:\n";
    //cout << SPrint(i) << endl;
    //cout << " coloco la nueva regla " << i << endl;
  }
}



// Operador que a�de una nueva regla al final del conjunto de reglas.
// Asocia a la regal el peso "weight".
void ruleset::Add(const genetcode &x, double weight) {
 if (reservado==n_rule){
   Realloc();
 }
 rule[n_rule] = new genetcode;
 rule[n_rule][0] = x;
 peso[n_rule]=weight;
 n_mas[n_rule]=0;
 n_menos[n_rule]=0;
 modificada[n_rule]=true;
 n_rule++;
}


void ruleset::Add(const genetcode &x, double mas, double menos, bool modif) {
 if (reservado==n_rule){
   Realloc();
 }
 rule[n_rule] = new genetcode;
 rule[n_rule][0] = x;
 n_mas[n_rule]=mas;
 n_menos[n_rule]=menos;
 modificada[n_rule]=modif;
 peso[n_rule]=(n_mas[n_rule]+1)/(n_mas[n_rule]+n_menos[n_rule]+1);
 n_rule++;
}




void ruleset::Add_Weight(int rule, double mas, double menos) {
  n_mas[rule]+=mas;
  n_menos[rule]+=menos;
  peso[rule]=(n_mas[rule]+1)/(n_mas[rule]+n_menos[rule]+1);
}


void ruleset::Recalculate_Weight(int i){
  peso[i]=(n_mas[i]+1)/(n_mas[i]+n_menos[i]+1);
}

void ruleset::Recalculate_Weight(){
  for (int i=0; i<n_rule; i++)
    Recalculate_Weight(i);
}


double ruleset::Get_Weight(int rule) const{
  return peso[rule];
}



void ruleset::Add_N_Mas(int rule, double value) {
   n_mas[rule]+=value;
}


double ruleset::Get_N_Mas(int rule) const{
  return n_mas[rule];
}

void ruleset::Add_N_Menos(int rule, double value) {
   n_menos[rule]+=value;
}



double ruleset::Get_N_Menos(int rule) const{
  return n_menos[rule];
}


void ruleset::Modificada(int rule, bool value) {
   modificada[rule]=value;
}


double ruleset::Get_Modificada(int rule) const{
  return modificada[rule];
}

void ruleset::FixModifyToTrue(){
  for (int i=0; i<n_rule; i++)
    modificada[i]=true;
}






// Operador que elimina la ultima regla del conjunto de reglas.
void ruleset::Remove(){
   delete rule[n_rule-1];
   n_rule--;
}


// Extrae una regla del conjunto de reglas
genetcode *ruleset::Extract(int i, double &weight){
  genetcode *aux=0;
  if (i<n_rule){
    aux=rule[i];
    weight=peso[i];
    for (int j=i+1; j<n_rule; j++){
       rule[j-1]=rule[j];
       peso[j-1]=peso[j];
       n_mas[j-1]=n_mas[j];
       n_menos[j-1]=n_menos[j];
       modificada[j-1]=modificada[j];
    }
    n_rule--;
  }
  else
    weight=-1;

  return aux;
}





// Devuelve el codigo asociado a la regla i
genetcode *ruleset::Get_Code_Rule(int i) const {
  genetcode *aux;

  if (i<n_rule)
   aux=rule[i];
  else
    cout << "Error en Get_Code_Rule: Indice " << i << " de regla fuera de rango\n";

  return aux;
}






// Operador que devuelve el numero de reglas que contiene el conjunto de reglas.
int ruleset::N_rule() const {
  return n_rule;
}


// Operador que devuelve el numero de reglas del conjunto de reglas con peso superior o igual a weight.
int ruleset::N_rule(double weight) const {
  int cont=0;
  for (int i=0; i<n_rule; i++)
    if (peso[i]>=weight)
      cont++;
  return cont;
}


// Devuelve el numero medio de variables por regla en la base de reglas.
double ruleset::Variables_per_rule() const{
 int r,*nr;
 double **mr, cont=0;

 for (int i=0; i<n_rule; i++){
   rule[i][0].GetReal(r,nr,mr);
   for (int j=0;j<nr[0]-1;j++)
     if (mr[0][j]>=mr[0][nr[0]-1])
       cont=cont+1;
 }

 return cont/n_rule;
}


int ruleset::Numero_Ceros_of_rule(int i) const{
 int r,*nr,*nn,**nnn,n;
 int b, *nb;
 char **mb;
 double **mr, cont1=0, cont2=0;
 int tama=0, acu=0;

   rule[i][0].GetBinary(b,nb,mb);
   rule[i][0].GetReal(r,nr,mr);

   for (int j=0;j<nr[0]-1;j++){
     int longitud=domain[0].SizeDomain(j);
     if (mr[0][j]>=mr[0][nr[0]-1]){
       for (int s=0; s<longitud; s++)
         if (mb[0][tama+s]=='0')
	   acu++;
     }
     tama+=longitud;
   }

 return acu;
}



int ruleset::Variables_of_rule(int i) const{
 int r,*nr,*nn,**nnn,n;
 double **mr, cont1=0, cont2=0;

   rule[i][0].GetReal(r,nr,mr);
   for (int j=0;j<nr[0]-1;j++)
     if (mr[0][j]>=mr[0][nr[0]-1])
       cont1=cont1+1;

   rule[i][0].GetInteger(n,nn,nnn);
   for (int j=0; j<nn[1]; j++)
     if (nnn[1][j]!=0)
       cont2++;

 return cont1+cont2;
}



int ruleset::New_Atributes_of_rule(int i) const{
 int r,*nr,*nn,**nnn,n;
 double **mr, cont1=0, cont2=0;

 rule[i][0].GetInteger(n,nn,nnn);
 for (int j=0; j<nn[1]; j++)
   if (nnn[1][j]!=0)
     cont2++;

 return cont2;
}

















//Devuelve la proporcion de variables usadas en las reglas en relacion al global de variables involucradas




//Devuelve la proporcion de variables usadas en las reglas en relacion al global de variables involucradas
// y ademas en frec devuelve el numero de veces que aparece cada variable en la base de reglas

double ruleset::Frecuence_each_Variables(int *frec) const {
 int r,*nr,b,*nb;
 char **mb;
 double **mr, cont=0;

 rule[0][0].GetReal(r,nr,mr);

 for (int j=0; j<nr[0]-1; j++)
   frec[j]=0;

 int donde,z, n_casos;
 char *s;
 for (int i=0; i<n_rule; i++){
   rule[i][0].GetReal(r,nr,mr);
   rule[i][0].GetBinary(b,nb,mb);
   donde=0;
   s= new char[nb[0]+1];
   for (int j=0; j<nb[0]; j++)
     s[j]=mb[0][j];
   s[nb[0]]='\0';

   for (int j=0;j<nr[0]-1;j++){
     n_casos=domain[0].SizeDomain(j);
     for(z=0; z<n_casos && s[donde+z]=='1';z++);

     if (mr[0][j]>=mr[0][nr[0]-1] && z!=n_casos)
       frec[j]++;
     donde+=n_casos;
   }

   delete []s;
 }

 for (int j=0; j<nr[0]-1; j++)
   if (frec[j]>0)
     cont=cont+1;



 return cont/(nr[0]-1);
}


int ruleset::Condiciones_per_RB() const{
 int r,*nr,b,*nb;
 char **mb;
 double **mr, cont=0;

 rule[0][0].GetReal(r,nr,mr);

 int donde,z, n_casos, medida=0;
 char *s;
 for (int i=0; i<n_rule; i++){
   rule[i][0].GetReal(r,nr,mr);
   rule[i][0].GetBinary(b,nb,mb);
   donde=0;
   s= new char[nb[0]+1];
   for (int j=0; j<nb[0]; j++)
     s[j]=mb[0][j];
   s[nb[0]]='\0';

   for (int j=0;j<nr[0]-1;j++){
     n_casos=domain[0].SizeDomain(j);
     for(z=0; z<n_casos && s[donde+z]=='1';z++);

     if (mr[0][j]>=mr[0][nr[0]-1] && z!=n_casos)
       medida++;
     donde+=n_casos;
   }

   delete []s;
 }


 return medida;

}






// Operador que devuelve el ordinal de la regla con mejor adaptacion sobre el ejemplo (v) con el conjunto de reglas y
// el grado de adaptacion de esa regla
//
int ruleset::InferenceC(const vectordouble &v, double &grado, int &regla_disparada) const{
 double max=0,aux;
 int re=-1;
 int b,*nb,n,*nn,**nnn,r,*nr;
 char **mb;
 double **mr;
 string regla, regla_funcion;
 int var1, var2;


 for (int i=0; i<n_rule; i++){
    rule[i][0].GetBinary(b,nb,mb);
    char *s= new char[nb[0]+1];
    for (int j=0; j<nb[0]; j++)
      s[j]=mb[0][j];
    s[nb[0]]='\0';
    regla=s;
    delete []s;

    s= new char[nb[1]+1];
    for (int j=0; j<nb[1]; j++)
      s[j]=mb[1][j];
    s[nb[1]]='\0';
    regla_funcion=s;
    delete []s;

    rule[i][0].GetInteger(n,nn,nnn);
    rule[i][0].GetReal(r,nr,mr);
    aux=domain[0].Adaptacion(v,regla,mr[0],mr[0][nr[0]-1]);
    aux=domain_funcion[0].Adaptacion_funcion(v,regla_funcion, nnn[1], nn[1], aux);

    //Aplicar las relaciones

    //aux=domain[0].Adaptacion(v,regla);

    aux = aux*peso[i];



    if (aux>max){
     max=aux;
     re=i;
     grado = aux;
    }
    else if (re!=-1 && aux>0 && aux==max && peso[i]>peso[re]) {
           max=aux;
           re=i;
           grado = aux;
         }
 }

 regla_disparada=re;

 if (re!=-1) {
   rule[re][0].GetInteger(n,nn,nnn);
   return nnn[0][0];
 }
 else
   return -1;
}






int ruleset::InferenceC(const vectordouble &v, int clase, double &grado_p, double &peso_p, double &grado_n, double &peso_n, int &regla_disparada, int &aislada) const{
 double max=0,aux, grado=0;
 int re=-1;
 int b,*nb,n,*nn,**nnn,r,*nr;
 char **mb;
 double **mr;
 string regla, regla_funcion;
 int var1, var2;
 double *adaptacion_particular = new double[n_rule];
 bool clasifica_bien=false;
 int de_clase=0, de_noclase=0, indice_pos=n_rule, indice_neg=n_rule;

 grado_p=0;
 peso_p=0;
 grado_n=0;
 peso_n=0;

 for (int i=0; i<n_rule; i++){
    rule[i][0].GetBinary(b,nb,mb);
    char *s= new char[nb[0]+1];
    for (int j=0; j<nb[0]; j++)
      s[j]=mb[0][j];
    s[nb[0]]='\0';
    regla=s;
    delete []s;

    s= new char[nb[1]+1];
    for (int j=0; j<nb[1]; j++)
      s[j]=mb[1][j];
    s[nb[1]]='\0';
    regla_funcion=s;
    delete []s;

    rule[i][0].GetInteger(n,nn,nnn);
    rule[i][0].GetReal(r,nr,mr);

    aux=domain[0].Adaptacion(v,regla,mr[0],mr[0][nr[0]-1]);
    aux=domain_funcion[0].Adaptacion_funcion(v,regla_funcion, nnn[1], nn[1], aux);

    //Aplicar las relaciones

    //aux=domain[0].Adaptacion(v,regla);

    aux = aux*peso[i];

    if (nnn[0][0]==clase)
      adaptacion_particular[i]=aux;
    else
      adaptacion_particular[i]=-aux;



    if (aux>0) {
      if (nnn[0][0]==clase){
        if (aux>grado_p or (aux==grado_p and peso[i]>peso_p)){
          grado_p=aux;
          peso_p=peso[i];
          indice_pos=i;
        }
      }
      else {
        if (aux>grado_n or (aux==grado_n and peso[i]>peso_n)){
          grado_n=aux;
          peso_n=peso[i];
          indice_neg=i;
        }
      }
    }


    if (aux>max){
     max=aux;
     re=i;
     grado = aux;
     if (nnn[0][0]==clase)
       clasifica_bien=true;
     else
       clasifica_bien=false;
    }
    else if (re!=-1 && aux>0 && aux==max && peso[i]>peso[re]) {
           max=aux;
           re=i;
           grado = aux;
           if (nnn[0][0]==clase)
             clasifica_bien=true;
           else
            clasifica_bien=false;
         }
 }

 // Numero de reglas de la clase que clasifican correctamente el ejemplo antes de una regla que lo clasifical mal o no lo clasifica.

 if (clasifica_bien){
   de_clase=0;
   for (int i=0; i<n_rule;i++)
     if (adaptacion_particular[i]>0 and
        ((adaptacion_particular[i]>grado_n) or
        (adaptacion_particular[i]==grado_n and peso[i]>peso_n) or
        (adaptacion_particular[i]==grado_n and peso[i]==peso_n and i<indice_neg)) )
       de_clase++;
   if (de_clase==0){
      cout << "Error!!!!  Clase " << clase << "  Grado Pos: " << grado_p << " Grado Neg: "<< grado_n << "  Regla: " << re <<endl;
      for (int t=0; t<n_rule; t++)
        cout <<"\t"<<adaptacion_particular[t];
      cout << endl;
   }

   aislada=de_clase;
 }
 else {
   if (grado_p==0)
     de_noclase=-1000;
   else {
     de_noclase=0;
     for (int i=0; i<n_rule; i++)
       if (adaptacion_particular[i]<0 and
          ((-adaptacion_particular[i]>grado_p) or
          (-adaptacion_particular[i]==grado_p and peso[i]>peso_p) or
          (-adaptacion_particular[i]==grado_p and peso[i]==peso_p and i<indice_pos) ))
         de_noclase--;
   }

   aislada=de_noclase;
 }



 delete []adaptacion_particular;

 regla_disparada=re;

 if (re!=-1) {
   rule[re][0].GetInteger(n,nn,nnn);
   return nnn[0][0];
 }
 else
   return -1;
}
































int ruleset::Class_of_rule (int i) const{
 int n,*nn,**nnn;
 rule[i][0].GetInteger(n,nn,nnn);
 return nnn[0][0];
}

























string ruleset::SPrint_Consequent(int i) const{
 int n,*nn,**nnn;

 rule[i][0].GetInteger(n,nn,nnn);
 return domain[0].SPrintDomain(domain[0].Consecuente(),nnn[0][0]);
}



string ruleset::SPrint(int i) const{
 string buffer="";
 int b,*nb,n,*nn,**nnn,r,*nr;
 char **mb;
 double **mr;
 string regla, regla_funcion;
 int var1, var2;
 int n_ant;

 // extraer la componente binaria
 rule[i][0].GetBinary(b,nb,mb);
 char *s= new char[nb[0]+1];
 for (int j=0; j<nb[0]; j++)
   s[j]=mb[0][j];
 s[nb[0]]='\0';
 regla=s;
 delete []s;

 s= new char[nb[1]+1];
 for (int j=0; j<nb[1]; j++)
   s[j]=mb[1][j];
 s[nb[1]]='\0';
 regla_funcion=s;
 delete []s;


 rule[i][0].GetInteger(n,nn,nnn);
 rule[i][0].GetReal(r,nr,mr);
 n_ant = domain[0].N_Antecedente();
 int z=0;
 int j=0;
 int donde=0;
 int n_casos=0;
 buffer+= "IF\n";
 while (j<n_ant){

   if (domain[0].IsActiva(j) && domain[0].IsAntecedente(j)){
     n_casos=domain[0].SizeDomain(j);
     for(z=0; z<n_casos && regla[donde+z]=='1';z++);
     if (mr[0][j]>=mr[0][nr[0]-1] && z!=n_casos){
       buffer+="\t";
       buffer+=domain[0].SPrintVar(j);
       buffer+=" = {";
       for(int t=donde; t<donde+n_casos;t++){
         if (regla[t]=='1'){
           buffer+= " ";
	   buffer+=domain[0].SPrintDomain(j,t-donde);
         }
        }
       buffer+= "}\n";
     }

   }
   donde+=n_casos;
   j++;
 }

 //Parte de funciones
 donde=0;
 for (int j=0; j<nn[1]; j++){
     if (nnn[1][j]!=0){
        n_casos=domain_funcion[0].SizeDomain(nnn[1][j]-1);
        for(z=0; z<n_casos && regla_funcion[donde+z]=='1';z++);
        buffer+="\t";
        buffer+=domain_funcion[0].SPrintVar(nnn[1][j]-1);
        buffer+=" = {";
        for(int t=donde; t<donde+n_casos;t++){
            if (regla_funcion[t]=='1'){
                buffer+= " ";
                buffer+=domain_funcion[0].SPrintDomain(nnn[1][j]-1,t-donde);
            }
        }
       buffer+= "}\n";
     }
     n_casos=2;
     donde+=n_casos;
}





 //Conclusion de la regla
 buffer+="THEN ";
 buffer+=domain[0].SPrintVar(domain[0].Consecuente());
 buffer+=" IS ";
 buffer+=domain[0].SPrintDomain(domain[0].Consecuente(),nnn[0][0]);
 buffer+="   W ";
 ostringstream ss;
 ss << peso[i];
 buffer+= ss.str();
 buffer+=" and N+= ";
 ostringstream ss2;
 ss2 << n_mas[i];
 buffer+= ss2.str();
 buffer+=" and N-= ";
 ostringstream ss3;
 ss3 << n_menos[i];
 buffer+= ss3.str();
 buffer+="\n";

 return buffer;

}





void ruleset::SaveRuleInterpreted_append(const char *nomfich) const {
  ofstream f(nomfich,ios::app);
  string regla;
  cout << "Numero de reglas =" << n_rule << endl;
  for (int i=0; i<n_rule; i++){
    regla=SPrint(i);
    f << regla.c_str() << endl;
  }

  f.close();
}




// Condiciones del antecedente de la regla i: los atributos relevantes, con las etiquetas que
// admiten. Un atributo es relevante si su nivel (parte real del código genético) llega al umbral
// de la regla y no admite todas sus etiquetas.
//
// Es la parte general de SplitAntecedent, que estaba en Old/: la interpretación de PlanMiner
// (solo atributos de hasta dos etiquetas, con el nombre de la etiqueta o su centro) se hace fuera.
std::vector<Condition> ruleset::Antecedente(int i) const{
  std::vector<Condition> condiciones;

  int b,*nb,n,*nn,**nnn,r,*nr;
  char **mb;
  double **mr;
  string regla;
  int n_ant;

  // extraer la componente binaria
  rule[i][0].GetBinary(b,nb,mb);
  char *s= new char[nb[0]+1];
  for (int j=0; j<nb[0]; j++){
    s[j]=mb[0][j];
  }
  s[nb[0]]='\0';
  regla=s;
  delete []s;

  rule[i][0].GetInteger(n,nn,nnn);
  rule[i][0].GetReal(r,nr,mr);
  n_ant = domain[0].N_Antecedente();
  int z=0;
  int j=0;
  int donde=0;
  int n_casos=0;
  while (j<n_ant){
    if (domain[0].IsActiva(j) && domain[0].IsAntecedente(j)){
      n_casos=domain[0].SizeDomain(j);
      for(z=0; z<n_casos && regla[donde+z]=='1';z++);
      if (mr[0][j]>=mr[0][nr[0]-1] && z!=n_casos){
        Condition condicion;
        condicion.attribute = j;
        for(int t=donde; t<donde+n_casos;t++){
          if (regla[t]=='1'){
            condicion.labels.push_back(t-donde);
          }
        }
        condiciones.push_back(condicion);
      }
    }
    donde+=n_casos;
    j++;
  }
  return condiciones;
}

}  // namespace inslv
