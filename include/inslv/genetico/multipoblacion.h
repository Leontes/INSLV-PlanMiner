// SPDX-License-Identifier: GPL-3.0-only
#ifndef _MULTIPOBLACIONH_
#define _MULTIPOBLACIONH_
#include "inslv/genetico/poblacionbinaria.h"
#include "inslv/genetico/poblacionentera.h"
#include "inslv/genetico/poblacionreal.h"
#include "inslv/genetico/genetcode.h"
#include "inslv/datos/example_set.h"

namespace inslv {

class multipoblacion{
  private:
    int elitismo;
    int n_individuos;
    bool *modificado;
    int n_valoracion;
    double **valoracion;
    int poblacionesB;
    int poblacionesE;
    int poblacionesR;
    poblacionbinaria **Pb;
    poblacionentera **Pe;
    poblacionreal **Pr;
  public:
    multipoblacion(int pbin, int pent, int preal, int *rangoe, double *rangori, double *rangors, double *mut, double *cruce, int eli, int n, int *tama, int n_val);
    ~multipoblacion();
    // Operaciones en la estructura
    void Swap(int i, int j);
    void Pasar(int i, int j);

    int ClaseIndividuo(int ind) const;
    char* Subpoblacion_Binaria(int subpoblacion, int individuo, int &tama) const;
    int* Subpoblacion_Entera(int subpoblacion, int individuo, int &tama) const;
    double* Subpoblacion_Real(int subpoblacion, int individuo, int &tama) const;
    void Sort();
    void Sort_menos2ultimos();

    double Sort_4L(example_set &E, int n_clases, int* examples_per_class, int**ind_clase, double *agregado, double *peso_agregado, int total_ejemplos_particion);
    void Better(int nclases, int *better, int &nbetter, int **ind_clase) const;
    void PintaFitness_Estacionario(int i) const;
    int N_individuos() const;
    void Codigo(int i, genetcode &code) const;
    void PutCodigo(int i, const genetcode &code);
    bool Modificado(int i) const;
    void PutModificado(int i);
    int N_Valoracion() const;
    void Valoracion(int i, double *dato);
    double ValorFitness(int i) const;
    double ValorFitness(int i, int j) const;


    void PoblacionInicial(double **I, int rango, int n_items, int **sujetos, int n_var, int *n_etiquetas_variable);




    void MutacionUniforme_Estacionario_Especial();

    void MutacionUniforme_Estacionario();
    void CruceBasedLogical_Estacionario(double it);



};



}  // namespace inslv

#endif
