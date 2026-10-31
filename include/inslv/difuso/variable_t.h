// SPDX-License-Identifier: GPL-3.0-only
#ifndef _VARIABLE_TH_
#define _VARIABLE_TH_

#include <string>
#include <fstream>
#include "inslv/difuso/domain_t.h"
#include "inslv/difuso/fuzzy_t.h"

namespace inslv {
class variable_t{
   private:
     string nombre;     // Nombre de la variable
     domain_t *dominio; // Dominio de la variable
     bool activa;
     bool antecedente;
     int unit;
     double convert_factor;
   public:
     variable_t();
     variable_t &operator=(const variable_t &x);
     ~variable_t();
     void Asigna(int n, string varname, int status, int Unit, double c_factor, double inf, double sup, double *a,  double *b, double *c, double *d, string *name, bool singleton = false);
     double Adaptacion(double x, int etiqueta) const;
     double Adaptacion(double x, string etiqueta) const;
     void Pinta() const;
     void PrintVar() const;
     string SPrintVar() const;
     string SPrintDomain(int value) const;
     int SizeDomain() const;
     bool Activa() const;
     bool Antecedente() const;
     int N_etiquetas() const;
     string Name() const { return nombre;}




     domain_t Domain() const;
     variable_t Variable() const;


};


}  // namespace inslv

#endif
