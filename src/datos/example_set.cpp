// SPDX-License-Identifier: GPL-3.0-only
using namespace std;
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include "inslv/datos/example_set.h"
#include "inslv/comun/progreso.hpp"

namespace inslv {


example_set::example_set(){
  n_example=0;
  n_variable=0;
  data=0;
  covered=0;
  g_positive_covered=0;
  g_negative_covered=0;
  peso_positive=0;
  peso_negative=0;
  rule_used=0;
  rule_aislada=0;
  n_partition=0;
  partition=0;
}

example_set::example_set(int ejemplos, int variables){
  // En el original solo avisaba y dejaba los punteros sin inicializar.
  if (ejemplos<0 ||variables<=0)
    throw std::logic_error("inslv: example_set con " + std::to_string(ejemplos) + " ejemplos y " +
                           std::to_string(variables) + " variables");
  else {
    n_example=ejemplos;
    n_variable=variables;
    n_partition=1;
    covered = new bool[n_example];
    g_positive_covered = new double [n_example];
    g_negative_covered = new double [n_example];
    peso_positive= new double [n_example];
    peso_negative= new double [n_example];
    rule_used= new int [n_example];
    rule_aislada= new int [n_example];
    data = new double*[n_example];
    partition = new int[n_example];
    for (int i=0; i<n_example; i++){
      data[i]= new double[variables];
      covered[i]=false;
      g_positive_covered[i]=0;
      g_negative_covered[i]=0;
      peso_positive[i]=0;
      peso_negative[i]=0;
      rule_used[i]=-1;
      rule_aislada[i]=0;
      partition[i]=0;
    }
  }
}







example_set::example_set(std::vector < std::vector < double > > * dataset){

  n_example =(*dataset)[0].size();
  n_variable = dataset -> size();

  n_partition=1;
  partition= new int[n_example];
  data = new double*[n_example];
  covered = new bool[n_example];
  g_positive_covered = new double [n_example];
  g_negative_covered = new double [n_example];
  peso_positive= new double [n_example];
  peso_negative= new double [n_example];
  rule_used= new int [n_example];
  rule_aislada= new int [n_example];

  for (int i=0; i<n_example; i++){
    data[i] = new double[n_variable];
    covered[i]=false;
    g_positive_covered[i]=0;
    g_negative_covered[i]=0;
    peso_positive[i]=0;
    peso_negative[i]=0;
    rule_used[i]=-1;
    rule_aislada[i]=0;
    partition[i]=0;
  }

  for (int i=0; i<n_variable; i++){
    for (int j=0; j<n_example; j++){
      data[j][i] = (*dataset)[i][j];
    }
  }
}







example_set &example_set::operator=(const example_set &x){
  if (this == &x)
    return *this;
  // Se libera también sin ejemplos: los constructores reservan los vectores aunque estén vacíos
  // (en el original solo se liberaban si había ejemplos).
  if (data!=0){
    for (int i=0; i<n_example; i++)
      delete [] data[i];
  }
  delete []data;
  delete []partition;
  delete []covered;
  delete []g_positive_covered;
  delete []g_negative_covered;
  delete []peso_positive;
  delete []peso_negative;
  delete []rule_used;
  delete []rule_aislada;

  n_example=x.n_example;
  n_variable=x.n_variable;
  n_partition=x.n_partition;
  covered = new bool[n_example];
  g_positive_covered = new double [n_example];
  g_negative_covered = new double [n_example];
  peso_positive= new double [n_example];
  peso_negative= new double [n_example];
  rule_used = new int [n_example];
  rule_aislada = new int [n_example];
  partition= new int[n_example];
  data = new double*[n_example];
  for (int i=0; i<n_example; i++){
    data[i] = new double[n_variable];
    partition[i]=x.partition[i];
    covered[i]=x.covered[i];
    g_positive_covered[i]=x.g_positive_covered[i];
    g_negative_covered[i]=x.g_negative_covered[i];
    peso_positive[i]=x.peso_positive[i];
    peso_negative[i]=x.peso_negative[i];
    rule_used[i]=x.rule_used[i];
    rule_aislada[i]=x.rule_aislada[i];
  }

  for (int i=0; i<n_example; i++)
    for (int j=0; j<n_variable; j++)
      data[i][j]=x.data[i][j];

  return *this;
}


example_set::~example_set(){
  // Se libera también sin ejemplos (ver operator=).
  if (data!=0){
    for (int i=0; i<n_example; i++)
      delete [] data[i];
  }
  delete [] data;
  delete [] covered;
  delete []g_positive_covered;
  delete []g_negative_covered;
  delete []peso_positive;
  delete []peso_negative;
  delete []rule_used;
  delete []rule_aislada;
  delete [] partition;

}



struct Local {
  Local(int p) {col = p;}
  bool operator()(const vector<double>& v1, const vector<double>& v2) {
    return v1[col] < v2[col];
  }

  int col;
};

void example_set::OrderByClass(int order, int VarClase) { // order=0 -> as it is, order=1 -> by class in order
  int old_n_clase = 0;
  vector< vector<double> > vect(n_example);

  switch (order) {
    case 1: { // Ordered by class (in order)
     progreso(Verbosidad::depuracion) << "Total 1: " << n_example << endl;
     for (int i = 0; i < n_example; i++) {
       vector<double> aux(n_variable);
       for (int j = 0; j < n_variable; j++) {
	 aux[j] = data[i][j];
       }
       vect[i] = aux;
     }

     sort(vect.begin(), vect.end(), Local(VarClase));

     n_clase.push_back(0);
     old_n_clase = 0;
     for (int i = 0; i < n_example; i++) {
       for (int j = 0; j < n_variable; j++) {
	 data[i][j] = vect[i][j];
	 if (j == VarClase) {
	   if (old_n_clase == data[i][j]) {
	     n_clase[data[i][j]]++;
	   }
	   else {
	     n_clase.push_back(1);
	     old_n_clase = data[i][j];
	   }
	 }
       }
     }

     progreso(Verbosidad::depuracion) << "Clases" << endl;
     for (int i = 0; i < n_clase.size(); i++)
       progreso(Verbosidad::depuracion) << n_clase[i] << endl;
     /*progreso(Verbosidad::depuracion) << "CARGA: " << VarClase << endl;
     for (int i = 0; i < n_example; i++) {
       for (int j = 0; j < n_variable; j++)
	 progreso(Verbosidad::depuracion) << data[i][j] << " ";
       progreso(Verbosidad::depuracion) << endl;
     }*/
     }
     break;
    case 2: { // Ordered by class (one at a time... 1 2 3)
      progreso(Verbosidad::depuracion) << "Total 2: " << n_example << endl;
      for (int i = 0; i < n_example; i++) {
	vector<double> aux(n_variable);
	for (int j = 0; j < n_variable; j++) {
	  aux[j] = data[i][j];
	}
	vect[i] = aux;
      }

      sort(vect.begin(), vect.end(), Local(VarClase));

      n_clase.push_back(0);
      old_n_clase = 0;
      for (int i = 0; i < n_example; i++) {
	for (int j = 0; j < n_variable; j++) {
	  data[i][j] = vect[i][j];
	  if (j == VarClase) {
	    if (old_n_clase == data[i][j]) {
	      n_clase[data[i][j]]++;
	    }
	    else {
	      n_clase.push_back(1);
	      old_n_clase = data[i][j];
	    }
	  }
	}
      }

      // Reorder
      vector<int> index(n_clase.size(), 0);
      int k = 0;
      for (int i = 0; i < n_example; i++) {
	for (int j = 0; j < n_variable; j++) {
	  if (k == 0)
	    data[i][j] = vect[index[k]][j];
	  else
	    data[i][j] = vect[index[k] + n_clase[k-1]][j];
	}
	index[k]++;
	do {
	  k++;
	  if (k == index.size())
	    k = 0;
	} while ((index[k] == n_clase[k]) && (i + 1 < n_example));
      }

      progreso(Verbosidad::depuracion) << "Clases" << endl;
      for (int i = 0; i < n_clase.size(); i++)
	progreso(Verbosidad::depuracion) << n_clase[i] << endl;
      /*progreso(Verbosidad::depuracion) << "CARGA: " << VarClase << endl;
      for (int i = 0; i < n_example; i++) {
	for (int j = 0; j < n_variable; j++)
	  progreso(Verbosidad::depuracion) << data[i][j] << " ";
	progreso(Verbosidad::depuracion) << endl;
      }*/
    }
    break;
  }
}








double example_set::Data(int ejemplo, int variable) const{
  return data[ejemplo][variable];
}


vectordouble example_set::Data(int ejemplo) const{
  vectordouble x;

  x.Put(data[ejemplo],n_variable);
  return x;
}



int example_set::N_Examples() const{
  return n_example;
}



void example_set::Generate_Partitions(int num_partition){
  if (num_partition<1)
     throw std::logic_error("inslv: número de particiones no válido: " + std::to_string(num_partition));


  n_partition=num_partition;

  if (n_partition==1){
    for (int i=0; i<n_example; i++)
      partition[i]=1;
  }
  else {
     for (int i=0; i<n_example; i++)
        partition[i]= (int) ((1.0*rand()*num_partition)/(RAND_MAX-1));
  }
}














int example_set::Not_Covered_Examples() const{
  int numero=0;
  for (int i=0; i<n_example; i++){
    //cout << "--> " << i << "\t" << gcovered[i]<< endl;
    if (!covered[i])
      numero++;
  }

  return numero;
}


void example_set::Examples_per_Class(int VarClass, int particion, int nclasses, int *n_examples_in_class) const{
  for (int i=0; i<nclasses; i++)
    n_examples_in_class[i]=0;

// cout << "a:" <<endl;
//
// for (int i = 0; i< n_example; i++) {
// 	for (int i = 0; i< n_variable; i++)
// 		cout << i << ' ';
// 	cout << endl;
// }

  int k;
  for (int i=0; i<n_example; i++)
     if (/*partition[i]!=particion &&*/ (!covered[i])) {
       k = (int) (data[i][VarClass]);
       n_examples_in_class[k]++;
     }

}





















void example_set::Mark(int *v, int tama, double *grado_p, double *peso_p, double *grado_n, double *peso_n, int *rule, int *r_aislada){

  for (int i=0; i<tama; i++){
    if (v[i]==1)
      covered[i]=true;
    else
      covered[i]=false;

    g_positive_covered[i]=grado_p[i];
    peso_positive[i]=peso_p[i];
    g_negative_covered[i]=grado_n[i];
    peso_negative[i]=peso_n[i];
    rule_used[i]=rule[i];
    rule_aislada[i]=r_aislada[i];
  }
}



void example_set::UnMarkAll(){
  for (int i=0; i<n_example; i++) {
    covered[i]=false;
    g_positive_covered[i]=0;
    g_negative_covered[i]=0;
    peso_positive[i]=0;
    peso_negative[i]=0;
    rule_used[i]=-1;
    rule_aislada[i]=0;
  }
}


bool example_set::Is_Covered(int i) const{
  return covered[i];
}




int example_set::Class_of_example(int VarClase, int i) const {
 int k;

 k = (int) (data[i][VarClase]);
 return k;
}

double example_set::Grade_Is_Positive_Covered(int i) const{
    return g_positive_covered[i];
}

double example_set::Grade_Is_Negative_Covered(int i) const{
    return g_negative_covered[i];
}

double example_set::Peso_Is_Positive_Covered(int i) const{
    return peso_positive[i];
}

double example_set::Peso_Is_Negative_Covered(int i) const{
    return peso_negative[i];
}


int example_set::Rule_Used(int i) const{
    return rule_used[i];
}

int example_set::Rule_Aislada(int i) const{
    return rule_aislada[i];
}


/*example_set example_set::Extract_Training_Set(int particion, int order) const{
  int sum=0;
  for (int i=0; i<n_example; i++)
    if (partition[i]!=particion)
     sum++;

  progreso(Verbosidad::depuracion) << "ejemplos: " << sum << "  variables: " << n_variable << endl;
  example_set E1(sum,  n_variable);


  int l=0;
  for (int i=0; i<n_example; i++){
     if (partition[i]!=particion){
       for (int j=0; j<n_variable; j++)
         E1.data[l][j]=data[i][j];
       l++;
     }
  }

  return E1;
}*/

example_set example_set::Extract_Training_Set(int particion, int VarClase, int order) const {
  int sum=0;
  for (int i=0; i<n_example; i++)
    if (partition[i]!=particion)
      sum++;

  progreso(Verbosidad::depuracion) << "ejemplos: " << sum << "  variables: " << n_variable << endl;
  example_set E1 = example_set(sum,  n_variable);

  progreso(Verbosidad::depuracion) << "Training E" << endl;

  int l=0;
  for (int i=0; i<n_example; i++){
    if (partition[i]!=particion) {
      for (int j=0; j<n_variable; j++) {
	E1.data[l][j]=data[i][j];
	//cout << E1.data[l][j] << " ";
      }
      //cout << endl;
      l++;
    }
  }

  switch (order) {
    case 1: E1.OrderByClass(1, VarClase); break;
    case 2: E1.OrderByClass(2, VarClase); break;
    default: break;
  }


  /*progreso(Verbosidad::depuracion) << "Training E ordered" << endl;
  for (int i=0; i< E1.N_Examples(); i++) {
    for (int j=0; j<n_variable; j++)
      progreso(Verbosidad::depuracion) << E1.data[i][j] << " ";
    progreso(Verbosidad::depuracion) << endl;
  }
  progreso(Verbosidad::depuracion) << "end" << endl;*/

  return E1;
}

example_set example_set::Extract_Test_Set(int particion, int VarClase) const{
  int sum=0;
  for (int i=0; i<n_example; i++)
    if (partition[i]==particion)
      sum++;

  progreso(Verbosidad::depuracion) << "ejemplos: " << sum << "  variables: " << n_variable << endl;
  example_set E1 = example_set(sum,  n_variable);

  int l=0;
  for (int i=0; i<n_example; i++){
    if (partition[i]==particion){
      for (int j=0; j<n_variable; j++)
	E1.data[l][j]=data[i][j];
      l++;
    }
  }

  return E1;
}


example_set example_set::SubSet(int tama) const {
  example_set E1(tama, n_variable);
  for (int i=0; i<tama; i++)
    for (int j=0; j<n_variable; j++)
      E1.data[i][j]=data[i][j];

  return E1;
}


example_set example_set::SubSet(int ini, int fin) const {
  example_set E1(fin-ini+1, n_variable);
  for (int i=0; i<fin-ini+1; i++)
    for (int j=0; j<n_variable; j++)
      E1.data[i][j]=data[i+ini][j];

  return E1;
}



example_set example_set::Union(const example_set &A){
  example_set resultado(n_example+A.n_example,n_variable);


  for (int i=0; i<n_example; i++)
    for (int j=0; j<n_variable; j++)
      resultado.data[i][j]=data[i][j];

  for (int i=0; i<A.n_example; i++)
    for (int j=0; j<n_variable; j++)
      resultado.data[i+n_example][j]=A.data[i][j];

  return resultado;
}

}  // namespace inslv
