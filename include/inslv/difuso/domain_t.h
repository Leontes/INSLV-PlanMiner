// SPDX-License-Identifier: GPL-3.0-only
#ifndef _DOMAIN_TH_
#define _DOMAIN_TH_
#include <fstream>
#include <string>
#include "inslv/difuso/fuzzy_t.h"

namespace inslv {

class domain_t {
   private:
     int numero;        // Numero de etiquetas
     double rango_inf;  // rango inferior
     double rango_sup; // rango superio
     bool inf_inf;      // determina si el rango inferior es infinito
     bool sup_inf;      // determina si el rango superior es infinito
     fuzzy_t *label;    // vector de etiquetas
   public:
     domain_t();
     ~domain_t();
     domain_t &operator=(const domain_t &x);

     void Asigna(int n, double inf, double sup, double *a, double *b, double *c, double *d, string *name, bool singleton = false);
     int N_etiquetas() const;
     double Adaptacion(double x, const int etiqueta) const;
     double Adaptacion(double x, string etiquetas) const;




     void Pinta() const;
     string SPrint(int i) const;
     int Size() const;
};

}  // namespace inslv

#endif
