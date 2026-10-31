// SPDX-License-Identifier: GPL-3.0-only
#ifndef _VECTORDOUBLEH_
#define _VECTORDOUBLEH_

namespace inslv {

class vectordouble{
  private:
    static constexpr double MISSING=-999999999;
    int reservado;
    int numero;
    double *data;
  public:
    vectordouble();
    vectordouble(int tamano);
    vectordouble(const vectordouble &x);
    vectordouble &operator=(const vectordouble &x);
    ~vectordouble();
    void Put(double *x, int tamano);
    double At(int pos) const;
};

}  // namespace inslv

#endif
