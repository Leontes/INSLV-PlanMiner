// SPDX-License-Identifier: GPL-3.0-only
#ifndef _POBLACIONBINARIAH_
#define _POBLACIONBINARIAH_

namespace inslv {

class poblacionbinaria{
  private:
    double prob_mutacion;
    double prob_cruce;
    int elitismo;
    int n_individuos;
    int *tamano;
    bool *modificado;
    double *valoracion;
    char **individuos;
  public:
    poblacionbinaria(double mut, double cruce, int eli, int n, int tama);
    ~poblacionbinaria();
    // Operaciones en la estructura
    char * Individuo(int i, int &tama) const;
    void Pasar(int i,const poblacionbinaria &x, int j);
    void Swap(int i, int j);
    void Codigo(int i, char *&v, int &tama) const;
    void PutValue(int indiv, int bit, char value);
    void PoblacionInicialAleatoria();

    void PoblacionInicial(int **sujeto, int tama, int* tama_dom);



    void MutacionUniforme_Estacionario();
    void Cruce2puntos_Estacionario(int indiv1, int indiv2);
    void CruceAND_OR_Estacionario(int indiv1, int indiv2);
    void CruceNAND_NOR_Estacionario(int indiv1, int indiv2);
    //void And();
    //void Or();
    //void Not();
};



}  // namespace inslv

#endif
