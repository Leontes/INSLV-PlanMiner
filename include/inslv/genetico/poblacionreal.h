// SPDX-License-Identifier: GPL-3.0-only
/***************************************************************************
                          poblacionreal.h  -  description
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
#ifndef _POBLACIONREALH_
#define _POBLACIONREALH_

namespace inslv {

class poblacionreal{
  private:
    double prob_mutacion;
    double prob_cruce;
    int elitismo;
    int n_individuos;
    int *tamano;
    bool *modificado;
    double rango_i;
    double rango_s;
    double **individuos;
  public:
    poblacionreal(double raninf, double ransup, double mut, double cruce, int eli, int n, int tama);
    ~poblacionreal();
    // Operaciones en la estructura
    double* Individuo(int i, int &tama) const;
    void Pasar(int i, const poblacionreal &x, int j);
    void Codigo(int i, double *&v, int &tama) const;
    void Swap(int i, int j);
    void PutValue(int indiv, int bit, double value);
    void PoblacionInicialAleatoria(int n_item);

    void PoblacionInicial(double **I, int clases);



    void MutacionUniforme_Estacionario();
    void Cruce2puntos_Estacionario(int indiv1, int indiv2);
    void CruceAND_OR_Estacionario(int indiv1, int indiv2);
    void CruceNAND_NOR_Estacionario(int indiv1, int indiv2);
};



}  // namespace inslv

#endif
