// SPDX-License-Identifier: GPL-3.0-only
#ifndef _VECTORVARH_
#define _VECTORVARH_
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include "inslv/difuso/fuzzy_t.h"
#include "inslv/difuso/domain_t.h"
#include "inslv/difuso/variable_t.h"
#include "inslv/datos/vectordouble.h"
#include "inslv/inslv.hpp"

namespace inslv {

struct combinado{
   int op;
   int operando[2];
   double rango[2];
};

class VectorVar {
  private:
    int numero;
    variable_t *lista;
    combinado *oper;

  public:
    VectorVar();
    explicit VectorVar(const Datos &data);
    ~VectorVar();
    VectorVar &operator=(const VectorVar &x);
    int N_Antecedente() const;

    void Encode(int &tb, int &te) const ;
    // void Decode(char *bin, int *ent);



    void Pinta() const;
    void PrintVar(int variable) const;
    string SPrintVar(int variable) const;
    string SPrintDomain(int variable, int value) const;
    bool IsActiva(int variable) const;
    bool IsAntecedente(int variable) const;
    int TotalVariables() const;
    int SizeDomain(int variable) const;


    double Adaptacion(double x, int variable, int dominio) const;
    bool Es_Valida(string regla, double *var, double umbral, double &simplicidad) const;
    void SecuenciasDeEtiquetasActivas(string sub, int tam, int &unos, int &ceros, int &n_unos) const;

    double Adaptacion(vectordouble x, string regla, double *var, double umbral) const;
    double Adaptacion_funcion(vectordouble x, string regla, int *lista_funcion, int tama, double adaptacion_previa) const;

    domain_t Domain(int var) const;
    variable_t Variable(int var) const;
    int Consecuente() const;

    void Funcion(int pos, int &x1, int &x2, int &op) const;



};

}  // namespace inslv

#endif
