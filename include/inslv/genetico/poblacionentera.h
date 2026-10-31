// SPDX-License-Identifier: GPL-3.0-only
#ifndef _POBLACIONENTERAH_
#define _POBLACIONENTERAH_

namespace inslv {

class poblacionentera{
  private:
    double prob_mutacion;
    double prob_cruce;
    int elitismo;
    int n_individuos;
    int *tamano;
    bool *modificado;
    double *valoracion;
    int rango;
    int **individuos;
  public:
    poblacionentera(int rango, double mut, double cruce, int eli, int n, int tama);
    ~poblacionentera();
    // Operaciones en la estructura
    int* Individuo(int i, int &tama) const;
    void Pasar(int i, const poblacionentera &x, int j);
    void Codigo(int i, int *&v, int &tama) const;
    void Swap(int i, int j);
    void PutValue(int indiv, int bit, int value);
    int GetValue(int individuo, int bit) const;
    // Operaciones geneticas
    void PoblacionInicialAleatoria();
    void PoblacionInicialAleatoria(int consecuente);



    void MutacionUniforme_Estacionario();
    void MutacionUniformeMod_Estacionario(double prob0);
    void MutacionUniformeMod_Estacionario_Especial();

    void Cruce2puntos_Estacionario(int indiv1, int indiv2);

};



}  // namespace inslv

#endif
