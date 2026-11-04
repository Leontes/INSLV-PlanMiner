// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/RuleCreator.hpp"

#include <chrono>
#include <format>
#include <stdexcept>
#include <string>

#include "inslv/comun/progreso.hpp"

namespace inslv {

namespace {

string Cadena(const genetcode& code) {
  const std::vector<char>& bits = code.Binary(0);
  return {bits.begin(), bits.end()};
}

// Si la valoración v2 es mejor que la v en sus tres primeros valores.
bool Mejor(const Fitness& v2, const Fitness& v) {
  return (v2[0] > v[0]) or (v2[0] == v[0] and v2[1] > v[1]) or
         (v2[0] == v[0] and v2[1] == v[1] and v2[2] > v[2]);
}

// Igual, o si es igual de buena.
bool MejorOIgual(const Fitness& v2, const Fitness& v) {
  return (v2[0] > v[0]) or (v2[0] == v[0] and v2[1] > v[1]) or
         (v2[0] == v[0] and v2[1] == v[1] and v2[2] >= v[2]);
}

}  // namespace

// En el original esperaba una tecla tras los avisos de consistencia del análisis de patrones.
// Como librería no puede bloquearse esperando la entrada estándar: solo avisa.
void Pausa() {
  progreso(Verbosidad::detalle)
      << "INSLV: aviso de consistencia en el análisis de patrones (ver la salida anterior)\n";
}

int Comparar2patrones(const string& cad1, const string& cad2) {
  int n_cambios = 0;
  bool compatibles = true;
  for (int s = 0; s < cad1.length(); s++)
    if (cad1[s] == cad2[s]) {
    } else {
      if (cad1[s] == '0' and cad2[s] == '1') {
        compatibles = false;
      } else if (cad1[s] == '1' and cad2[s] == '0') {
        compatibles = false;
      } else {
      }

      n_cambios++;
    }

  if (n_cambios == 0)
    return 0;  // Son iguales
  else if (compatibles)
    return 1;
  else
    return 2;
}

int Comparar2patronesPinta(const string& cad1, const string& cad2) {
  int n_cambios = 0;
  bool compatibles = true;
  for (int s = 0; s < cad1.length(); s++)
    if (cad1[s] == cad2[s]) {
      progreso(Verbosidad::depuracion) << "_";
    } else {
      if ((cad1[s] == 'x' and cad2[s] == '0') or (cad1[s] == '0' and cad2[s] == 'x')) {
        progreso(Verbosidad::depuracion) << "X";
        compatibles = false;
      } else if (cad1[s] == '0' and cad2[s] == '1') {
        progreso(Verbosidad::depuracion) << ">";
        compatibles = false;
      } else if (cad1[s] == '1' and cad2[s] == '0') {
        progreso(Verbosidad::depuracion) << "<";
        compatibles = false;
      } else {
        progreso(Verbosidad::depuracion) << "$";
      }

      n_cambios++;
    }

  progreso(Verbosidad::depuracion) << endl;

  if (n_cambios == 0)
    return 0;  // Son iguales
  else if (compatibles)
    return 1;
  else
    return 2;
}

// Hace la union de las dos reglas y la operacion la almacena en la regla r1
void UnionReglasDescriptivas(const VectorVar& V, ruleset& R1, int r1, ruleset& R2, int r2) {
  genetcode r_1, r_2, *aux;
  aux = R1.Get_Code_Rule(r1);
  r_1 = *aux;
  aux = R2.Get_Code_Rule(r2);
  r_2 = *aux;

  int b, *nb, n, *nn, **nnn, r, *nr;
  char** mb;
  double** mr;
  string regla1, regla2, regla3;

  r_1.GetBinary(b, nb, mb);
  r_1.GetReal(r, nr, mr);

  char* s = new char[nb[0] + 1];
  for (int j = 0; j < nb[0]; j++) s[j] = mb[0][j];
  s[nb[0]] = '\0';
  regla1 = s;
  delete[] s;

  int longitud, tama = 0;

  for (int j = 0; j < V.N_Antecedente(); j++) {
    longitud = V.SizeDomain(j);
    if (mr[0][j] < mr[0][nr[0] - 1]) {
      for (int s = 0; s < longitud; s++) regla1[tama + s] = '1';
    }
    tama += longitud;
  }

  r_2.GetBinary(b, nb, mb);
  r_2.GetReal(r, nr, mr);

  s = new char[nb[0] + 1];
  for (int j = 0; j < nb[0]; j++) s[j] = mb[0][j];
  s[nb[0]] = '\0';
  regla2 = s;
  delete[] s;

  tama = 0;

  for (int j = 0; j < V.N_Antecedente(); j++) {
    longitud = V.SizeDomain(j);
    if (mr[0][j] < mr[0][nr[0] - 1]) {
      for (int s = 0; s < longitud; s++) regla2[tama + s] = '1';
    }
    tama += longitud;
  }

  string patr3 = Union(regla1, regla2);
  aux = R1.Get_Code_Rule(r1);

  for (int j = 0; j < nb[0]; j++) aux->PutValueBinary(0, j, patr3[j]);
}

string Union(const string& cad1, const string& cad2) {
  string aux = cad1;

  for (int s = 0; s < cad1.length(); s++)
    if (cad1[s] == '0' and cad2[s] == '0')
      aux[s] = '0';
    else
      aux[s] = '1';

  return aux;
}

// Hace la interseccion de las dos reglas y la operacion la almacena en la regla r1
void IntersecionReglasDiscriminantes(const VectorVar& V, ruleset& R1, int r1, ruleset& R2, int r2) {
  genetcode r_1, r_2, *aux;
  aux = R1.Get_Code_Rule(r1);
  r_1 = *aux;
  aux = R2.Get_Code_Rule(r2);
  r_2 = *aux;

  int b, *nb, n, *nn, **nnn, r, *nr;
  char** mb;
  double** mr;
  string regla1, regla2, regla3;

  r_1.GetBinary(b, nb, mb);
  r_1.GetReal(r, nr, mr);

  char* s = new char[nb[0] + 1];
  for (int j = 0; j < nb[0]; j++) s[j] = mb[0][j];
  s[nb[0]] = '\0';
  regla1 = s;
  delete[] s;

  int longitud, tama = 0;

  for (int j = 0; j < V.N_Antecedente(); j++) {
    longitud = V.SizeDomain(j);
    if (mr[0][j] < mr[0][nr[0] - 1]) {
      for (int s = 0; s < longitud; s++) regla1[tama + s] = '1';
    }
    tama += longitud;
  }

  r_2.GetBinary(b, nb, mb);
  r_2.GetReal(r, nr, mr);

  s = new char[nb[0] + 1];
  for (int j = 0; j < nb[0]; j++) s[j] = mb[0][j];
  s[nb[0]] = '\0';
  regla2 = s;
  delete[] s;

  tama = 0;

  for (int j = 0; j < V.N_Antecedente(); j++) {
    longitud = V.SizeDomain(j);
    if (mr[0][j] < mr[0][nr[0] - 1]) {
      for (int s = 0; s < longitud; s++) regla2[tama + s] = '1';
    }
    tama += longitud;
  }

  string patr3 = Interseccion(regla1, regla2);
  aux = R1.Get_Code_Rule(r1);

  for (int j = 0; j < nb[0]; j++) aux->PutValueBinary(0, j, patr3[j]);

  aux->GetReal(r, nr, mr);

  for (int j = 0; j < V.N_Antecedente(); j++) {
    if (mr[0][j] < mr[0][nr[0] - 1]) {
      mr[0][j] = mr[0][nr[0] - 1];
    }
  }
}

string Interseccion(const string& cad1, const string& cad2) {
  string aux = cad1;

  for (int s = 0; s < cad1.length(); s++)
    if (cad1[s] == '1' and cad2[s] == '1')
      aux[s] = '1';
    else
      aux[s] = '0';

  return aux;
}

int Grados_Libertad(string cad) {
  int cont = 0;
  for (int i = 0; i < cad.length(); i++)
    if (cad[i] == '?') cont++;

  return cont;
}

void AnalizarConjuntosDeReglas(const VectorVar& V, ruleset& R, ruleset& R_Descrip, ruleset& R_mix,
                               int nuevas) {
  genetcode *aux_mix, *aux_R, *aux_R_des;
  int iguales, iguales2;
  std::vector<bool> eliminar(R.N_rule(), false);

  for (int i = 0; i < nuevas; i++) {
    if (!eliminar[i]) {
      string res = Patron2Reglas(R_Descrip, R, V, i);
      string res3 = Patron1Regla(R_Descrip, V, i);
      string res4 = Patron1Regla(R, V, i);
      aux_mix = R_mix.Get_Code_Rule(i);
      int len = aux_mix->TamaBinary(0);
      char* cad = new char[len + 1];
      for (int j = 0; j < len; j++) cad[j] = aux_mix->GetValueBinary(0, j);
      cad[len] = '\0';
      string patr = cad;
      delete[] cad;

      aux_R = R.Get_Code_Rule(i);
      len = aux_R->TamaBinary(0);
      cad = new char[len + 1];
      for (int j = 0; j < len; j++) cad[j] = aux_R->GetValueBinary(0, j);
      cad[len] = '\0';
      string regla_R = cad;
      delete[] cad;

      aux_R_des = R_Descrip.Get_Code_Rule(i);
      len = aux_R_des->TamaBinary(0);
      cad = new char[len + 1];
      for (int j = 0; j < len; j++) cad[j] = aux_R_des->GetValueBinary(0, j);
      cad[len] = '\0';
      string regla_R_des = cad;
      delete[] cad;

      if (res != patr) {
        progreso(Verbosidad::depuracion) << "No coinciden los patrones para la regla " << i << endl;
        progreso(Verbosidad::depuracion) << res << endl;
        progreso(Verbosidad::depuracion) << patr << endl;
        Pausa();
      }

      if (regla_R != patr and Comparar2patrones(patr, regla_R) != 1) {
        progreso(Verbosidad::depuracion)
            << "La regla discriminante " << i << " no coincide con su patron\n";
        progreso(Verbosidad::depuracion) << regla_R << endl;
        progreso(Verbosidad::depuracion) << patr << endl;
        Pausa();
      }

      if (regla_R_des != patr and Comparar2patrones(patr, regla_R_des) != 1) {
        progreso(Verbosidad::depuracion)
            << "La regla descriptiva " << i << " no coincide con su patron\n";
        progreso(Verbosidad::depuracion) << regla_R_des << endl;
        progreso(Verbosidad::depuracion) << patr << endl;
        Pausa();
      }

      for (int j = i + 1; j < R.N_rule() and !eliminar[i]; j++) {
        if (!eliminar[j]) {
          if (R.Class_of_rule(i) == R.Class_of_rule(j)) {
            string res2 = Patron2Reglas(R_Descrip, R, V, j);

            iguales = Comparar2patrones(res2, patr);
            iguales2 = Comparar2patrones(res3, res2);
            int iguales3 = Comparar2patrones(res4, res2);

            // El caso raro de campatibildades entre patrones
            if (iguales > 1 and iguales2 != 2 and iguales3 != 2) {
              progreso(Verbosidad::depuracion)
                  << "Caso especial de incompatibilidades entre patrones y si entre reglas\n";
              progreso(Verbosidad::depuracion) << res3 << endl;
              progreso(Verbosidad::depuracion) << res4 << endl;
              progreso(Verbosidad::depuracion) << patr << endl << endl;
              progreso(Verbosidad::depuracion) << Patron1Regla(R_Descrip, V, j) << endl;
              progreso(Verbosidad::depuracion) << Patron1Regla(R, V, j) << endl;
              progreso(Verbosidad::depuracion) << res2 << endl << endl;
              Pausa();
            }

            // Los dos patrones son iguales
            if (iguales == 0) {
              eliminar[i] = true;
            }
            // Los dos patrones son compatibles
            else if (iguales == 1) {
              progreso(Verbosidad::detalle) << "\t\t\t\t\tCombinando Reglas " << i << " y " << j
                                            << " de la clase " << R.Class_of_rule(i) << endl;

              string cad1 = Patron2Reglas(R_Descrip, R, V, i);

              string cad3 = Patron2Reglas(R_Descrip, R, V, j);

              IntersecionReglasDiscriminantes(V, R, j, R, i);

              UnionReglasDescriptivas(V, R_Descrip, j, R_Descrip, i);

              string cad2 = Patron2Reglas(R_Descrip, R, V, j);

              string cad4 = Patron2Patrones(cad1, cad3);
              progreso(Verbosidad::depuracion) << cad4 << endl;
              int iguales4 = Comparar2patrones(cad2, cad4);
              if (iguales4 != 0) {
                progreso(Verbosidad::depuracion) << cad2 << endl;
                progreso(Verbosidad::depuracion) << cad4 << endl;
                Comparar2patronesPinta(cad2, cad4);
                Pausa();
              }

              aux_mix = R_mix.Get_Code_Rule(j);
              len = aux_mix->TamaBinary(0);
              for (int t = 0; t < len; t++) aux_mix->PutValueBinary(0, t, cad4[t]);

              aux_R = R.Get_Code_Rule(j);
              len = aux_R->TamaBinary(0);
              for (int t = 0; t < len; t++)
                if (cad4[t] == '?')
                  aux_R->PutValueBinary(0, t, '1');
                else
                  aux_R->PutValueBinary(0, t, cad4[t]);

              aux_R_des = R_Descrip.Get_Code_Rule(j);
              len = aux_R_des->TamaBinary(0);
              for (int t = 0; t < len; t++)
                if (cad4[t] == '?')
                  aux_R_des->PutValueBinary(0, t, '0');
                else
                  aux_R_des->PutValueBinary(0, t, cad4[t]);

              if (Patron2Reglas(R_Descrip, R, V, j) != cad4) {
                progreso(Verbosidad::depuracion)
                    << "Error al componer la descrip. y la predic. en la regla " << j << endl;
                Pausa();
              }

              eliminar[i] = true;
              R_Descrip.Modificada(j, true);
              R.Modificada(j, true);

              if (Grados_Libertad(res2) >= Grados_Libertad(patr)) {
                R_Descrip.Add_N_Mas(j, R_Descrip.Get_N_Mas(i));
                R_Descrip.Add_N_Menos(j, R_Descrip.Get_N_Menos(i));

                R.Add_N_Mas(j, R.Get_N_Mas(i));
                R.Add_N_Menos(j, R.Get_N_Menos(i));
              }
            }
          } else {
            string res2 = Patron2Reglas(R_Descrip, R, V, j);
            iguales = Comparar2patrones(res2, patr);

            if (iguales == 0) {
              progreso(Verbosidad::detalle)
                  << "las reglas " << i << " y " << j << " son contradictorias\n";
              R.Add_N_Menos(i, R.Get_N_Mas(i));
              R.Recalculate_Weight(i);
              R_Descrip.Add_N_Menos(i, R_Descrip.Get_N_Mas(i));
              R_Descrip.Recalculate_Weight(i);
            } else if (iguales == 1) {
              progreso(Verbosidad::detalle)
                  << "las reglas " << i << " y " << j << " entran en conflicto\n";
            }
          }
        }
      }
    }
  }

  std::vector<int> eliminables;
  for (int i = 0; i < R.N_rule() - 1; i++) {
    if (eliminar[i]) eliminables.push_back(i);
  }

  Filtrar_Reglas_New(R, R_Descrip, R_mix, eliminables);
}

int** Encontrar_Individuos_dados_ejemplos(const VectorVar& V, const example_set& E, int nclases,
                                          int n_individuos, double*** Tabla,
                                          int* n_ejemplos_por_clase) {
  int a;

  int** matriz = new int*[n_individuos];
  for (int i = 0; i < n_individuos; i++) matriz[i] = new int[V.N_Antecedente()];

  for (int i = 0; i < n_individuos; i++) {
    // Elegir un ejemplo de la clase i%n_clases
    if (n_ejemplos_por_clase[i % nclases] == 0) {
      for (int j = 0; j < V.N_Antecedente(); j++) matriz[i][j] = -1;
    } else {
      a = Select_Random_Individuo(E.N_Examples(), -1, 0);
      while (E.Is_Covered(a) ||
             T_Adaptacion_Variable_Etiqueta(V, Tabla, a, V.Consecuente(), i % nclases) == 0) {
        a++;
        a = a % E.N_Examples();
      }
      for (int j = 0; j < V.N_Antecedente(); j++) {
        matriz[i][j] = BetterEtiqueta(V, Tabla, j, a);
      }
    }
  }

  return matriz;
}

void Obtener_Aciertos_y_Errores_Reglas_Aprendidas(const example_set& E, int* basica_para_regla,
                                                  int* errores_para_regla, int nrule) {
  for (int i = 0; i < nrule; i++) {
    basica_para_regla[i] = 0;
    errores_para_regla[i] = 0;
  }

  for (int j = 0; j < E.N_Examples(); j++) {
    if (E.Rule_Aislada(j) == 1 and E.Rule_Used(j) >= 0) {
      basica_para_regla[E.Rule_Used(j)]++;
    } else if (E.Rule_Aislada(j) == -1 and E.Rule_Used(j) >= 0) {
      errores_para_regla[E.Rule_Used(j)]++;
    }
  }
}

void ActualizarPeso(multipoblacion& G, const VectorVar& V, example_set& E, ruleset& R,
                    double*** Tabla) {
  double n_mas, n_menos;
  for (int i = 0; i < R.N_rule(); i++) {
    if (R.Get_Modificada(i)) {
      G.PutCodigo(G.N_individuos() - 1, *R.Get_Code_Rule(i));
      Weight_Individual(G, V, E, Tabla, G.N_individuos() - 1, n_mas, n_menos);
      R.Add_Weight(i, n_mas, n_menos);
      R.Modificada(i, false);
    }
  }
}

void GA(const Evaluacion& ev, ruleset& R, ruleset& R_dis, ruleset& R_mix, double** I,
        bool primeraVez, std::vector<int>& reglas_a_eliminar, bool& Added_Rule) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  const example_set& E = ev.E;
  double*** Tabla = ev.Tabla;

  Added_Rule = false;

  const int n_clases = V.SizeDomain(V.Consecuente());

  if (R.N_rule() == 0 or primeraVez) {
    std::vector<int> n_ejemplos_por_clase(n_clases);
    E.Examples_per_Class(V.Consecuente(), n_clases, n_ejemplos_por_clase.data());
    int** sujetos = Encontrar_Individuos_dados_ejemplos(V, E, n_clases, G.N_individuos(), Tabla,
                                                        n_ejemplos_por_clase.data());
    std::vector<int> lista_tamano(V.N_Antecedente());
    for (int i = 0; i < V.N_Antecedente(); i++) lista_tamano[i] = V.SizeDomain(i);

    G.PoblacionInicial(I, sujetos, V.N_Antecedente(), lista_tamano.data());

    for (int i = 0; i < G.N_individuos(); i++) delete[] sujetos[i];
    delete[] sujetos;
  } else {
    for (int i = 0; i < G.N_individuos(); i++) G.PutModificado(i);
  }

  fitness(ev);

  for (int ik = G.N_individuos() - 2; ik < G.N_individuos(); ik++)
    Obtener_Regla_Descriptiva(ev, ik, reglas_a_eliminar);

  G.Sort();

  // La valoración del mejor individuo, el primero.
  const auto valoracion_del_mejor = [&G] {
    Fitness valoracion;
    for (int l = 0; l < G.N_Valoracion(); l++) valoracion[l] = G.ValorFitness(0, l);
    return valoracion;
  };
  Fitness mejor_anterior = valoracion_del_mejor();

  const int max_iter = 500;
  int sin_cambios = 0;
  while (sin_cambios <= max_iter) {
    if ((R.N_rule() > 1 and G.ValorFitness(0) <= 0 and Probabilidad(0.75)) or
        (R.N_rule() > 1 and Probabilidad(0.01))) {
      // En el original elegía aquí una regla que luego no usaba. Se mantiene por el número que
      // pide.
      static_cast<void>(Select_Random_Individuo(R.N_rule(), -1, -1));
      G.MutacionUniforme_Estacionario_Especial();
    } else {
      G.CruceBasedLogical_Estacionario((1.0 * sin_cambios) / max_iter);
      G.MutacionUniforme_Estacionario();
    }
    fitness(ev);
    G.Sort_4L();

    const Fitness mejor = valoracion_del_mejor();
    if (mejor == mejor_anterior) {
      sin_cambios++;
      if (sin_cambios == 2) {
        const int t = 1;
        genetcode code;
        G.Codigo(0, code);
        G.PutCodigo(t, code);

        if (G.ValorFitness(t) != -E.N_Examples() or G.ValorFitness(t, 1) != 1) {
          Obtener_Regla_Discriminante(ev, t, reglas_a_eliminar);
        } else {
          Obtener_Regla_Descriptiva(ev, t, reglas_a_eliminar);
        }
      }
    } else {
      sin_cambios = 0;
      mejor_anterior = mejor;
    }
  }

  const double v0 = G.ValorFitness(0), v1 = G.ValorFitness(0, 1), v2 = G.ValorFitness(0, 2),
               v3 = G.ValorFitness(0, 3), v4 = G.ValorFitness(0, 4);
  const double ne = -E.N_Examples();

  if (v0 <= -1 and v1 > 1 and v1 >= (-v0) + 2) {
    progreso(Verbosidad::depuracion) << "->->->->->-> El nuevo caso especial\n";
  }

  if ((v0 > -1) or (v0 <= -1 and v1 > 1 and v1 >= (-v0) + 2) or (v0 == ne and v1 > 1) or
      (v0 == ne and v1 == 1 and v2 > 0) or (v0 == ne and v1 == 1 and v2 == 0 and v3 > 0) or
      (v0 == ne and v1 == 1 and v2 == 0 and v3 == 0 and v4 > 0)) {
    genetcode code1, code2;
    Fitness valor;
    std::array<double, 2> esp, gen;
    double peso, nmas1, nmas2, nmenos1, nmenos2;
    string s_esp, s_gen;

    do {
      Obtener_Regla_Discriminante(ev, 0, reglas_a_eliminar);

      G.Codigo(0, code1);
      s_gen = Cadena(code1);

      Bondad(ev, 0, valor, reglas_a_eliminar);
      peso = Weight_Individual(G, V, E, Tabla, 0, nmas1, nmenos1);
      gen = {valor[0], valor[1]};

      Obtener_Regla_Descriptiva(ev, 0, reglas_a_eliminar);

      Bondad(ev, 0, valor, reglas_a_eliminar);
      peso = Weight_Individual(G, V, E, Tabla, 0, nmas2, nmenos2);
      esp = {valor[0], valor[1]};

      G.Codigo(0, code2);
      s_esp = Cadena(code2);
    } while (esp != gen or !ComprobacionEspGen(s_esp, s_gen));

    R.Add(code2, nmas2, nmenos2, false);
    R_dis.Add(code1, nmas1, nmenos1, false);

    const string patr = Patron2Reglas(R, R_dis, V, R.N_rule() - 1);

    if (!ComprobacionEspGen(Patron1Regla(R, V, R.N_rule() - 1),
                            Patron1Regla(R_dis, V, R_dis.N_rule() - 1))) {
      progreso(Verbosidad::depuracion) << "esp: " << endl
                                       << Patron1Regla(R, V, R.N_rule() - 1) << endl;
      progreso(Verbosidad::depuracion) << Patron1Regla(R_dis, V, R_dis.N_rule() - 1) << endl
                                       << ":gen" << endl;
      Comparar2patronesPinta(Patron1Regla(R, V, R.N_rule() - 1),
                             Patron1Regla(R_dis, V, R_dis.N_rule() - 1));
      Pausa();
    }

    const int len = code1.TamaBinary(0);
    for (int i = 0; i < len; i++) code1.PutValueBinary(0, i, patr[i]);
    R_mix.Add(code1, peso);

    Added_Rule = true;

    // Pongo la regla por defecto para la clase obtenida en la posición penultima de la poblacion
    for (int i = 0; i < len; i++) code1.PutValueBinary(0, i, '1');

    int c = code1.GetValueInteger(0, 0);

    G.PutCodigo(0, code2);

    // Pongo la regla por defecto para otra clase en la ultima posicion de la poblacion
    code1.PutValueInteger(0, 0, (c + 1) % n_clases);
    G.PutCodigo(G.N_individuos() - 1, code1);
    c = (c + 1) % n_clases;
    code1.PutValueInteger(0, 0, (c + 1) % n_clases);
    G.PutCodigo(G.N_individuos() - 2, code1);
  }
}

// Determina si se mantiene que esp es una especificacion de gen y viceversa
bool ComprobacionEspGen(string esp, string gen) {
  bool correcto = true;

  for (int i = 0; i < esp.length() and correcto; i++)
    if (esp[i] == '1' and gen[i] == '0') correcto = false;

  return correcto;
}

void fitness(const Evaluacion& ev) {
  multipoblacion& G = ev.G;
  // Las reglas que eliminaría cada individuo solo interesan al evaluar la regla que se añade.
  std::vector<int> reglas_a_eliminar;
  Fitness valor;
  for (int i = 0; i < G.N_individuos(); i++) {
    if (G.Modificado(i)) {
      Bondad(ev, i, valor, reglas_a_eliminar);
      G.Valoracion(i, valor.data());
    }
  }
}

bool Relevante(char* s, int tama) {
  int i = 0;
  while (i < tama and s[i] == '1') i++;

  return (i < tama);
}

int NumeroCeros(char* s, int tama) {
  int acu = 0;
  for (int i = 0; i < tama; i++)
    if (s[i] == '0') acu++;
  return acu;
}

void Obtener_Regla_Discriminante(const Evaluacion& ev, int elemento_i,
                                 std::vector<int>& reglas_a_eliminar) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  int r1, b1;
  double* nr1 = G.Subpoblacion_Real(elemento_i, r1);
  char* nb1 = G.Subpoblacion_Binaria(elemento_i, b1);
  const double umbral = nr1[r1 - 1];

  int tama = 0;
  for (int j = 0; j < r1 - 1; j++) {
    const int longitud = V.Variable(j).N_etiquetas();
    if (nr1[j] < umbral) {
      for (int s = 0; s < longitud; s++) nb1[tama + s] = '1';
    }
    tama += longitud;
  }

  Fitness valor, valor2;
  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valoracion(elemento_i, valor.data());

  // Quita cada atributo relevante si la regla mejora sin él.
  for (int j = 0; j < r1 - 1; j++) {
    if (nr1[j] >= umbral) {
      const double aux = nr1[j];
      nr1[j] = umbral - 0.001;

      Bondad(ev, elemento_i, valor2, reglas_a_eliminar);

      if (Mejor(valor2, valor)) {
        valor = valor2;
      } else {
        nr1[j] = aux;
      }
    }
  }

  // Admite cada etiqueta más si la regla no empeora.
  tama = 0;
  for (int j = 0; j < r1 - 1; j++) {
    const double aux2 = nr1[j];
    const int longitud = V.Variable(j).N_etiquetas();

    if (nr1[j] >= umbral) {
      for (int s = 0; s < longitud; s++) {
        if (nb1[tama + s] == '0') {
          nb1[tama + s] = '1';
          Bondad(ev, elemento_i, valor2, reglas_a_eliminar);
          if (MejorOIgual(valor2, valor)) {
            valor = valor2;
          } else {
            nb1[tama + s] = '0';
            nr1[j] = aux2;
          }
        }
      }
    }
    tama += longitud;
  }

  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valoracion(elemento_i, valor.data());
}

string Patron1Regla(ruleset& R_1, const VectorVar& V, int pos) {
  // Sin esa regla, un patrón vacío. En el original leía la regla -1 cuando el genético no
  // añadía ninguna en la primera vuelta (R_mix vacío) y el programa reventaba (HALLAZGOS).
  if (pos < 0 || pos >= R_1.N_rule()) return "";
  return Cadena(*R_1.Get_Code_Rule(pos));
}

string Patron2Patrones(string pat1, string pat2) {
  string pat3 = pat1;

  for (int i = 0; i < pat1.length(); i++) {
    if (pat1[i] == pat2[i])
      pat3[i] = pat1[i];
    else if ((pat1[i] == '1' and pat2[i] == '?') or (pat1[i] == '?' and pat2[i] == '1'))
      pat3[i] = '1';
    else if ((pat1[i] == '0' and pat2[i] == '?') or (pat1[i] == '?' and pat2[i] == '0'))
      pat3[i] = '0';
    else
      pat3[i] = '?';
  }

  return pat3;
}

string Patron2Reglas(ruleset& R_1, ruleset& R_2, const VectorVar& V, int pos) {
  genetcode r_1, r_2, *aux1, *aux2;
  aux1 = R_1.Get_Code_Rule(pos);
  r_1 = *aux1;
  aux2 = R_2.Get_Code_Rule(pos);
  r_2 = *aux2;

  int b, *nb, n, *nn, **nnn, r, *nr;
  char** mb;
  double** mr;
  string regla1, regla2, regla3;

  r_1.GetBinary(b, nb, mb);
  r_1.GetReal(r, nr, mr);

  char* s = new char[nb[0] + 1];
  for (int j = 0; j < nb[0]; j++) s[j] = mb[0][j];
  s[nb[0]] = '\0';
  regla1 = s;
  delete[] s;

  int longitud, tama = 0;

  for (int j = 0; j < V.N_Antecedente(); j++) {
    longitud = V.SizeDomain(j);
    if (mr[0][j] < mr[0][nr[0] - 1]) {
      for (int s = 0; s < longitud; s++) regla1[tama + s] = '1';
    }
    tama += longitud;
  }

  r_2.GetBinary(b, nb, mb);
  r_2.GetReal(r, nr, mr);

  s = new char[nb[0] + 1];
  for (int j = 0; j < nb[0]; j++) s[j] = mb[0][j];
  s[nb[0]] = '\0';
  regla2 = s;
  delete[] s;

  tama = 0;

  for (int j = 0; j < V.N_Antecedente(); j++) {
    longitud = V.SizeDomain(j);
    if (mr[0][j] < mr[0][nr[0] - 1]) {
      for (int s = 0; s < longitud; s++) regla2[tama + s] = '1';
    }
    tama += longitud;
  }

  regla3 = regla1;
  for (int i = 0; i < nb[0]; i++) {
    if (regla1[i] == regla2[i])
      regla3[i] = regla1[i];
    else
      regla3[i] = '?';
  }

  return regla3;
}

void Obtener_Regla_Descriptiva(const Evaluacion& ev, int elemento_i,
                               std::vector<int>& reglas_a_eliminar) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  int r1, b1;
  double* nr1 = G.Subpoblacion_Real(elemento_i, r1);
  char* nb1 = G.Subpoblacion_Binaria(elemento_i, b1);
  const double umbral = nr1[r1 - 1];

  Fitness valor, valor2;
  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valoracion(elemento_i, valor.data());

  int tama = 0;
  for (int j = 0; j < r1 - 1; j++) {
    const int longitud = V.Variable(j).N_etiquetas();

    if (nr1[j] < umbral) {
      // Añade el atributo a la regla si, quitándole alguna etiqueta, no empeora.
      const double aux = nr1[j];
      bool mejora = false;
      nr1[j] = umbral;
      for (int s = 0; s < longitud; s++) nb1[tama + s] = '1';

      for (int s = 0; s < longitud; s++) {
        nb1[tama + s] = '0';
        Bondad(ev, elemento_i, valor2, reglas_a_eliminar);

        if (MejorOIgual(valor2, valor)) {
          valor = valor2;
          mejora = true;
        } else {
          nb1[tama + s] = '1';
          nr1[j] = umbral;
        }
      }

      if (!mejora) nr1[j] = aux;
    } else {
      // Quita cada etiqueta si la regla mejora sin ella.
      for (int s = 0; s < longitud; s++)
        if (nb1[tama + s] == '1') {
          nb1[tama + s] = '0';
          Bondad(ev, elemento_i, valor2, reglas_a_eliminar);
          if (Mejor(valor2, valor)) {
            valor = valor2;
          } else {
            nb1[tama + s] = '1';
          }
        }
    }

    tama += longitud;
  }

  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valoracion(elemento_i, valor.data());
}

double Weight_Individual(multipoblacion& G, const VectorVar& V, const example_set& E,
                         double*** Tabla, int elemento_i, double& n_mas, double& n_menos) {
  int b1, r1;
  const char* nb1 = G.Subpoblacion_Binaria(elemento_i, b1);
  const string regla(nb1, b1);
  const double* nr1 = G.Subpoblacion_Real(elemento_i, r1);
  const int clase = G.ClaseIndividuo(elemento_i);

  double positivos = 0, negativos = 0, aux_p, aux_n;

  for (int j = 0; j < E.N_Examples(); j++) {
    T_AdaptacionC(V, Tabla, j, clase, aux_p, aux_n);

    const double x = T_Adaptacion_Antecedente(V, Tabla, j, regla, nr1, nr1[r1 - 1]);

    positivos = positivos + (x * aux_p);
    negativos = negativos + (x * aux_n);
  }

  n_mas = positivos;
  n_menos = negativos;

  return (positivos + 1) / (positivos + negativos + 1);
}

double Bondad(const Evaluacion& ev, int elemento_i, Fitness& valor,
              std::vector<int>& reglas_a_eliminar) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  const example_set& E = ev.E;
  const ruleset& R = ev.R;
  const int* ponderacion_por_clase = ev.ponderacion_por_clase;
  const int* basica_para_regla = ev.basica_para_regla;
  const int* errores_para_regla = ev.errores_para_regla;
  double*** Tabla = ev.Tabla;

  const int ne = E.N_Examples();
  const int nrule = R.N_rule();
  const int clase = G.ClaseIndividuo(elemento_i);

  int b1, r1;
  char* nb1 = G.Subpoblacion_Binaria(elemento_i, b1);
  const string regla(nb1, b1);
  double* nr1 = G.Subpoblacion_Real(elemento_i, r1);
  const double umbral = nr1[r1 - 1];

  int variables_irrelevantes = 0, ceros_variables = 0;
  int pos = 0;  // posicion en el cromosoma nivel de valor

  for (int j = 0; j < r1 - 1; j++) {
    const int tamanio = V.Variable(j).N_etiquetas();
    if (nr1[j] < umbral) {
      variables_irrelevantes++;
    } else if (Relevante(&(nb1[pos]), tamanio)) {
      ceros_variables += NumeroCeros(&(nb1[pos]), tamanio);
    } else {
      nr1[j] = umbral - 0.001;
      variables_irrelevantes++;
    }
    pos = pos + tamanio;
  }

  const int variables_relevantes = (r1 - 1) - variables_irrelevantes;

  // Comprobar que es una regla valida
  if (!V.Es_Valida(regla, nr1, umbral) or variables_irrelevantes >= r1 - 1) {
    valor.fill(-2 * ne);
    return 0;
  }

  // Calculo previo para obtener el peso de la regla
  std::vector<double> adaptacion(ne);
  double positivos = 0, negativos = 0, afecta = 0, aux_p, aux_n;
  for (int j = 0; j < ne; j++) {
    T_AdaptacionC(V, Tabla, j, clase, aux_p, aux_n);

    adaptacion[j] = T_Adaptacion_Antecedente(V, Tabla, j, regla, nr1, umbral);

    if (adaptacion[j] * aux_p > 0) afecta++;

    positivos = positivos + (adaptacion[j] * aux_p);
    negativos = negativos + (adaptacion[j] * aux_n);
    if (aux_p < aux_n) adaptacion[j] = -adaptacion[j];
  }

  const double peso = (positivos + 1) / (positivos + negativos + 1);
  const bool menos_infinito = !(positivos > 0);

  if (ponderacion_por_clase[clase] > 0) afecta = afecta / ponderacion_por_clase[clase];

  // Dado el peso de la regla obtener la bondad de la misma.
  std::vector<int> basica_esta_regla(nrule), errores_esta_regla(nrule);
  const auto anota = [&](std::vector<int>& cuenta, int ejemplo, int aislada) {
    if (E.Rule_Aislada(ejemplo) == aislada and E.Rule_Used(ejemplo) >= 0 and
        E.Rule_Used(ejemplo) < nrule)
      cuenta[E.Rule_Used(ejemplo)]++;
  };
  double aciertos = 0, fallos = 0;
  positivos = 0;
  negativos = 0;
  for (int j = 0; j < ne; j++) {
    const double a = adaptacion[j];
    if (a > 0) {
      // La regla gana a la mejor de otra clase.
      const bool gana =
          a * peso > E.Grade_Is_Negative_Covered(j) or
          (a * peso == E.Grade_Is_Negative_Covered(j) and peso > E.Peso_Is_Negative_Covered(j));
      if (gana) {
        if (!E.Is_Covered(j)) {
          positivos = positivos + (a * peso);
          aciertos++;
          anota(errores_esta_regla, j, -1);
        } else {
          anota(basica_esta_regla, j, 1);
        }
      }
    } else if (a < 0) {
      // La regla gana a la mejor de la clase del ejemplo.
      const bool gana =
          -a * peso > E.Grade_Is_Positive_Covered(j) or
          (-a * peso == E.Grade_Is_Positive_Covered(j) and peso > E.Peso_Is_Positive_Covered(j));
      if (gana) {
        negativos = negativos +
                    ((-a * peso) / ponderacion_por_clase[E.Class_of_example(V.Consecuente(), j)]);
        if (!E.Is_Covered(j)) {
          anota(errores_esta_regla, j, -1);
        } else {
          fallos++;
          anota(basica_esta_regla, j, 1);
        }
      }
    }
  }

  // Evaluar si es posible eliminar alguna regla ya existente
  int reglas_eliminables = 0;
  double aciertos_adicionales = 0;
  int ganancia_en_variables = variables_relevantes;
  double diferencia_peso = peso;

  reglas_a_eliminar.clear();
  for (int i = ev.tam_rule_not_removed; i < nrule; i++) {
    const int ganancia = (basica_esta_regla[i] - basica_para_regla[i]) +
                         (errores_para_regla[i] - errores_esta_regla[i]);
    if (ganancia >= 0) {
      reglas_a_eliminar.push_back(i);
      reglas_eliminables++;
      aciertos_adicionales += ganancia;
      ganancia_en_variables = R.Variables_of_rule(i) - ganancia_en_variables;
      diferencia_peso -= R.Get_Weight(i);
    }
  }

  if (ponderacion_por_clase[clase] > 0)
    positivos = positivos / ponderacion_por_clase[clase];
  else
    positivos = 0;

  if (aciertos + aciertos_adicionales - fallos > 0) {
    if (ponderacion_por_clase[clase] > 0) {
      if (positivos - negativos < 0)
        valor[0] = -0.9;
      else {
        valor[0] = positivos - negativos;
        valor[0] *= peso * peso * peso * peso * peso * peso * peso * peso * peso;
      }

      if (positivos == 0 && menos_infinito) {
        valor[0] = -2 * ne;
      }
    } else
      valor[0] = -2 * ne;
  } else if (aciertos + aciertos_adicionales > 0) {
    valor[0] = aciertos + aciertos_adicionales - fallos;
    if (valor[0] == 0)
      valor[0] = -ne;
    else if (ev.reducir_reglas) {
      if (reglas_eliminables >= (-valor[0]) + 2)
        valor[0] = aciertos + aciertos_adicionales - fallos;
      else
        valor[0] = -2 * ne;
    }
  } else
    valor[0] = -(ne + fallos);

  valor[6] = aciertos_adicionales;
  valor[5] = aciertos - fallos;
  valor[4] = 0;
  valor[3] = -variables_relevantes;
  valor[2] = afecta;
  valor[1] = reglas_eliminables;

  if (reglas_eliminables == 1 and (valor[0] == -ne or valor[0] == 0)) {
    if (aciertos_adicionales > 0)
      valor[2] = 0.1 + aciertos_adicionales;
    else
      valor[2] = diferencia_peso;
    valor[3] = -ganancia_en_variables;
    valor[4] = 0;
    valor[5] = ceros_variables;
    // En el original, con una sola regla leía el peso de una regla que no existe: memoria sin
    // inicializar o de una regla ya quitada. No decide nada: dos individuos solo empatan hasta
    // aquí si los dos pasan por esta rama, y entonces leen el mismo valor (y forzándolo a 0, 1e9 y
    // -1e9, las reglas de los conjuntos de prueba no cambian).
    valor[6] = reglas_eliminables < nrule ? -R.Get_Weight(reglas_eliminables) : 0;
  }

  return peso;
}

double Aciertos(const ruleset& R, const VectorVar& V, example_set& E) {
  const int n = E.N_Examples();
  const int conse = V.Consecuente();
  std::vector<int> marcar(n), regla_disparada(n), aislada(n);
  std::vector<double> g_pos_grado(n), g_neg_grado(n), peso_pos(n), peso_neg(n);
  double bien = 0;

  for (int i = 0; i < n; i++) {
    const vectordouble w = E.Data(i);
    const int clase_ejemplo = static_cast<int>(w.At(conse));
    const int clase = R.InferenceC(w, clase_ejemplo, g_pos_grado[i], peso_pos[i], g_neg_grado[i],
                                   peso_neg[i], regla_disparada[i], aislada[i]);
    if (clase == -1) {
      marcar[i] = -1;
    } else if (clase == w.At(conse)) {
      bien++;
      marcar[i] = 1;
    } else {
      marcar[i] = 0;
    }
  }

  E.Mark(marcar.data(), n, g_pos_grado.data(), peso_pos.data(), g_neg_grado.data(), peso_neg.data(),
         regla_disparada.data(), aislada.data());

  return bien / n;
}

std::vector<int> Reglas_Que_Disparan(const ruleset& R, const example_set& E) {
  std::vector<bool> usada(R.N_rule());

  for (int i = 0; i < E.N_Examples(); i++) {
    const vectordouble w = E.Data(i);
    double grado;
    int regla_disparada;
    if (R.InferenceC(w, grado, regla_disparada) != -1) usada[regla_disparada] = true;
  }

  std::vector<int> lista;
  for (int i = 0; i < R.N_rule(); i++) {
    if (usada[i]) lista.push_back(i);
  }
  return lista;
}

void Filtrar_Reglas_New(ruleset& R, ruleset& R_dis, ruleset& R_mix,
                        const std::vector<int>& reglas_a_eliminar) {
  for (auto i = reglas_a_eliminar.rbegin(); i != reglas_a_eliminar.rend(); ++i) {
    R.Remove(*i);
    R_dis.Remove(*i);
    R_mix.Remove(*i);
  }
}

// devuelve una matriz de nv filas (n. variables antecedente) y
// m columnas (numero de clases del consecuente +1) donde en la
// posicion 0 se encuentra la medida general sobre todas las clases.
double** ReservaParaMedidasDeInformacion(const VectorVar& V) {
  progreso(Verbosidad::depuracion)
      << "Med " << V.N_Antecedente() << " " << V.SizeDomain(V.Consecuente()) + 1 << endl;
  int n = V.TotalVariables(), conse = V.Consecuente();
  int tam_dom_conse = V.SizeDomain(conse);
  int nv, m = tam_dom_conse + 1;

  nv = V.N_Antecedente();
  // Reservo una matriz de tamano nvxm
  double** matriz = new double*[nv];
  for (int i = 0; i < nv; i++) {
    matriz[i] = new double[m];
  }

  return matriz;
}

// Libera la matriz de medidas de informacion
void LiberaMedidasDeInformacion(double**& matriz, int n) {
  for (int i = 0; i < n; i++) delete[] matriz[i];

  delete[] matriz;
}

void CalcularProbabilidadSimpleVariable(const VectorVar& V, const example_set& E, int variable,
                                        int n_casos, double* p) {
  int ne = 0;
  double suma = 0;
  double* aux = new double[n_casos];

  for (int j = 0; j < n_casos; j++) p[j] = 0;

  for (int i = 0; i < E.N_Examples(); i++) {
    if (!E.Is_Covered(i)) {
      suma = 0;
      for (int j = 0; j < n_casos; j++) {
        aux[j] = V.Domain(variable).Adaptacion(E.Data(i, variable), j);
        suma += aux[j];
      }
      // Añadir los valores a la sumatoria
      if (suma > 0) {
        for (int j = 0; j < n_casos; j++) p[j] = p[j] + (aux[j] / suma);

        ne++;
      }
    }
  }

  delete[] aux;

  if (ne > 0) {
    progreso(Verbosidad::depuracion) << "NE: " << ne << endl;

    for (int j = 0; j < n_casos; j++) p[j] = p[j] / ne;
  }
}

void CalcularProbabilidadConjunta2Variables(const VectorVar& V, const example_set& E, int var1,
                                            int var2, int n_casos1, int n_casos2, double** m) {
  int ne = 0;
  double suma;

  double** aux = new double*[n_casos1];
  for (int j = 0; j < n_casos1; j++) aux[j] = new double[n_casos2];

  for (int j = 0; j < n_casos1; j++)
    for (int k = 0; k < n_casos2; k++) m[j][k] = 0;

  for (int i = 0; i < E.N_Examples(); i++) {
    if (!E.Is_Covered(i)) {
      suma = 0;
      for (int j = 0; j < n_casos1; j++)
        for (int k = 0; k < n_casos2; k++) {
          aux[j][k] = V.Domain(var1).Adaptacion(E.Data(i, var1), j) *
                      V.Domain(var2).Adaptacion(E.Data(i, var2), k);
          suma = suma + aux[j][k];
        }

      if (suma > 0) {
        for (int j = 0; j < n_casos1; j++)
          for (int k = 0; k < n_casos2; k++) m[j][k] = m[j][k] + (aux[j][k] / suma);
      }

      ne++;
    }
  }

  if (ne > 0)
    for (int j = 0; j < n_casos1; j++)
      for (int k = 0; k < n_casos2; k++) m[j][k] = m[j][k] / ne;

  for (int j = 0; j < n_casos1; j++) delete[] aux[j];

  delete[] aux;
}

// Calcula las medidas de informacion
void MedidasDeInformacion(const VectorVar& V, const example_set& E, double** I) {
  int n = V.TotalVariables(), conse = V.Consecuente();
  int ne;

  double* px;
  double* py;
  double** pxy;
  double Inf, H;

  int nv = V.N_Antecedente();

  py = new double[V.SizeDomain(conse)];

  CalcularProbabilidadSimpleVariable(V, E, conse, V.SizeDomain(conse), py);

  progreso(Verbosidad::depuracion) << "PY" << endl;
  for (int i = 0; i < V.SizeDomain(conse); i++) progreso(Verbosidad::depuracion) << py[i] << " ";
  progreso(Verbosidad::depuracion) << endl;

  int i = 0, j = 0;
  double I1 = 0, H1 = 0, I2 = 0, H2 = 0, aux;
  while (i < nv) {
    progreso(Verbosidad::depuracion) << i + 1 << "/" << nv << endl;
    if (V.IsActiva(j) && V.IsAntecedente(j)) {
      px = new double[V.SizeDomain(j)];
      pxy = new double*[V.SizeDomain(j)];
      for (int k = 0; k < V.SizeDomain(j); k++) pxy[k] = new double[V.SizeDomain(conse)];
      CalcularProbabilidadSimpleVariable(V, E, j, V.SizeDomain(j), px);
      CalcularProbabilidadConjunta2Variables(V, E, j, conse, V.SizeDomain(j), V.SizeDomain(conse),
                                             pxy);
      I1 = 0;
      H1 = 0;
      for (int k = 0; k < V.SizeDomain(conse); k++) {
        I2 = 0;
        H2 = 0;
        for (int q = 0; q < V.SizeDomain(j); q++) {
          if (pxy[q][k] == 0)
            aux = 0;
          else

            aux = pxy[q][k] * log((px[q] * py[k]) / pxy[q][k]);

          I2 = I2 - aux;
          I1 = I1 - aux;

          if (pxy[q][k] == 0)
            aux = 0;
          else
            aux = pxy[q][k] * log(pxy[q][k]);

          H2 = H2 - aux;
          H1 = H1 - aux;
        }

        if (H2 == 0)
          I[i][k + 1] = 0;
        else
          I[i][k + 1] = (I2 / H2);
      }
      if (H1 == 0)
        I[i][0] = 0;
      else
        I[i][0] = (I1 / H1);

      V.PrintVar(j);
      progreso(Verbosidad::depuracion) << ": " << I1 << "/" << H1 << ": " << I[i][0] << endl;
      for (int k = 0; k < V.SizeDomain(conse); k++) {
        progreso(Verbosidad::depuracion) << "\tEtiqueta " << k << ": " << I[i][k + 1] << endl;
      }

      i++;

      delete[] px;
      for (int k = 0; k < V.SizeDomain(j); k++) delete[] pxy[k];
      delete[] pxy;
    } else {
      progreso(Verbosidad::depuracion) << "POS: " << i << " " << j << endl;
    }
    j++;
  }

  delete[] py;
}

// Determina la mejor particion del dominio de una funcion

double*** Crear_Tabla_Adaptaciones(const VectorVar& V, const example_set& E) {
  int n = V.TotalVariables();
  int m = E.N_Examples();
  int l;
  double valor;

  // Reservando la Memoria para las adaptaciones
  double*** Tabla = new double**[n];
  for (int i = 0; i < n; i++) {
    l = V.SizeDomain(i);
    Tabla[i] = new double*[l];
    for (int j = 0; j < l; j++) Tabla[i][j] = new double[m];
  }

  // Calculando los valores de adaptaciones de cada ejemplo a cada etiqueta de cada variable
  int valores_distinto_cero;
  for (int e = 0; e < m; e++) {
    for (int v = 0; v < n; v++) {
      valor = E.Data(e, v);
      l = V.SizeDomain(v);
      valores_distinto_cero = 0;
      for (int j = 0; j < l; j++) {
        Tabla[v][j][e] = V.Adaptacion(valor, v, j);
        if (Tabla[v][j][e] > 0 and Tabla[v][j][e] <= 1) valores_distinto_cero++;
      }

      if (valores_distinto_cero == 0)
        throw std::invalid_argument("inslv: el valor " + std::to_string(valor) +
                                    " de la variable " + std::to_string(v) +
                                    " no se adapta a ninguna de sus etiquetas");
    }
  }

  return Tabla;
}

void Destruir_Tabla_Adaptaciones(const VectorVar& V, double***& Tabla) {
  int n = V.TotalVariables();
  int l;
  for (int i = 0; i < n; i++) {
    l = V.SizeDomain(i);
    for (int j = 0; j < l; j++) delete[] Tabla[i][j];
    delete[] Tabla[i];
  }
  delete[] Tabla;
}

// Adaptacion de un ejemplo con una etiqueta de la variable
double T_Adaptacion_Variable_Etiqueta(const VectorVar& V, double*** Tabla, int ejemplo,
                                      int variable, int etiqueta) {
  return Tabla[variable][etiqueta][ejemplo];
}

// Adaptacion de un ejemplo con un variable
double T_Adaptacion_Variable(const VectorVar& V, double*** Tabla, int ejemplo, int variable,
                             string labels) {
  int l = V.SizeDomain(variable);
  double max = 0;
  for (int etiqueta = 0; etiqueta < l && max < 1; etiqueta++)
    if (labels[etiqueta] == '1')
      if (max == 0)
        max = Tabla[variable][etiqueta][ejemplo];
      else if (Tabla[variable][etiqueta][ejemplo] > 0)
        max = 1;
  return max;
}

// Adaptacion de un ejemplo al antecedente de una regla
double T_Adaptacion_Antecedente(const VectorVar& V, double*** Tabla, int ejemplo,
                                const string& regla, const double* nivel_variable, double umbral) {
  int n = V.N_Antecedente();
  double max = 1, aux;
  string sub;
  int trozo = 0, tam;
  for (int v = 0; v < n && max > 0; v++) {
    tam = V.SizeDomain(v);
    if (nivel_variable[v] >= umbral) {
      sub = regla.substr(trozo, tam);
      aux = T_Adaptacion_Variable(V, Tabla, ejemplo, v, sub);
      if (aux < max) max = aux;
    }
    trozo += tam;
  }
  return max;
}

// Adaptacion de un ejemplo con el consecuente
void T_AdaptacionC(const VectorVar& V, double*** Tabla, int ejemplo, int etiq, double& pos,
                   double& neg) {
  int conse = V.Consecuente();
  pos = Tabla[conse][etiq][ejemplo];
  if (pos == 1)
    neg = 0;
  else
    neg = 1;
}

// Mejor etiqueta dada una variable y un ejemplo
int BetterEtiqueta(const VectorVar& V, double*** Tabla, int variable, int ejemplo) {
  int l = V.SizeDomain(variable), et = 0;
  double max = 0;

  for (int etiqueta = 0; etiqueta < l && max < 1; etiqueta++) {
    if (Tabla[variable][etiqueta][ejemplo] > max) {
      max = Tabla[variable][etiqueta][ejemplo];
      et = etiqueta;
    }
  }
  return et;
}

namespace {

// Los aciertos de cada regla, para el informe (marca los ejemplos, como Aciertos).
double Aciertos_Por_Regla(const ruleset& R, const VectorVar& V, example_set& E,
                          const std::string& nomfich) {
  const double acierto = Aciertos(R, V, E);
  std::vector<int> bien_clas(R.N_rule()), mal_clas(R.N_rule());
  int bien = 0, mal = 0, nosesabe = 0;
  for (int i = 0; i < E.N_Examples(); i++) {
    if (E.Rule_Used(i) == -1) {
      nosesabe++;
    } else if (E.Is_Covered(i)) {
      bien++;
      bien_clas[E.Rule_Used(i)]++;
    } else {
      mal++;
      mal_clas[E.Rule_Used(i)]++;
    }
  }

  ofstream out(nomfich, ios::app);
  out << "Aciertos: " << bien << endl;
  out << "Errores:  " << mal << endl;
  out << "No clasificados: " << nosesabe << endl;
  for (int i = 0; i < R.N_rule(); i++) {
    out << "\tRule " << i << "(" << R.SPrint_Consequent(i) << "): " << bien_clas[i] << " / "
        << mal_clas[i] << endl;
  }
  return acierto;
}

void Escribir_Informe(const std::string& task_res, const VectorVar& V, example_set& E,
                      ruleset& R_prev, ruleset& R_prev_dis, ruleset& R_mix_prev, double tiempo) {
  const double aciertos_discriminantes = Aciertos_Por_Regla(R_prev_dis, V, E, task_res);
  const double aciertos_descriptivas = Aciertos_Por_Regla(R_prev, V, E, task_res);

  std::vector<int> frecuencia_variables(V.N_Antecedente());
  const double variables_usadas = R_prev.Frecuence_each_Variables(frecuencia_variables.data());

  ofstream out(task_res, ios::app);
  out << "----------------------------------------------------\n";
  out << "Numero de reglas: " << R_prev.N_rule() << endl;
  out << "Variables por regla: " << R_prev.Variables_per_rule() << endl;
  out << "Variables usadas: " << variables_usadas << endl;
  out << "Tiempo: " << tiempo << endl;
  out << "Condiciones: " << R_prev.Condiciones_per_RB() << endl;
  out << "Aciertos Discriminantes: " << aciertos_discriminantes << endl;
  out << "Aciertos Descriptivas: " << aciertos_descriptivas << endl;
  out << "----------------------------------------------------\n";
  out << "------------------- REGLAS -------------------------\n";
  out << "----------------------------------------------------\n";

  out << "Descriptivas: " << endl;
  out.close();
  R_prev.SaveRuleInterpreted_append(task_res);
  out.open(task_res, ios::app);
  out << "Predictivas: " << endl;
  out.close();
  R_prev_dis.SaveRuleInterpreted_append(task_res);
  out.open(task_res, ios::app);
  out << "Patrones: " << endl;

  for (int kk = 0; kk < R_mix_prev.N_rule(); kk++) {
    string res = "(0)";
    res[1] = res[1] + R_mix_prev.Class_of_rule(kk);
    res = Patron1Regla(R_mix_prev, V, kk) + res;
    out << res << endl;
  }

  out << "Frecuencia de aparicion de las " << V.N_Antecedente()
      << " distintas variables en las reglas: \n";
  for (int i = 0; i < V.N_Antecedente(); i++) {
    out << V.SPrintVar(i) << ": ";
    out << "--> " << (1.0 * frecuencia_variables[i] / R_prev.N_rule()) * 100 << " %\n";
  }
}

}  // namespace

std::vector<Regla> Aprender(const VectorVar& V, const example_set& E, const Opciones& options) {
  // El informe (el antiguo <tarea>.res) solo se escribe si se pide.
  const bool informe = !options.informe.empty();
  const std::string task_res = options.informe.string();

  V.Pinta();

  ofstream out;
  if (informe) {
    out.open(task_res);
    out << "Experiment: " << task_res << endl;
    std::time_t ahora = std::time(nullptr);
    out << std::ctime(&ahora);
    out << "===================================================\n\n";
    out.close();
  }

  srand(options.semilla);
  progreso(Verbosidad::depuracion) << "SEED " << options.semilla << endl;

  // Numero de componentes de la funcion fitness
  const int NFitness = 7;
  // El tamano del nivel de valor y el numero de clases
  int tama_valor, n_clases;
  V.Encode(tama_valor, n_clases);
  // El nivel de variable: el de cada atributo y el umbral
  const int tama_nivel = V.N_Antecedente() + 1;

  std::vector<int> ejemplos_por_clase(n_clases);

  double** Med_Inform = ReservaParaMedidasDeInformacion(V);
  {
    progreso(Verbosidad::detalle) << "Calculando medidas de informacion\n";
    const example_set E_operadores = E.N_Examples() > 10000 ? E.SubSet(1000) : E;
    MedidasDeInformacion(V, E_operadores, Med_Inform);
    progreso(Verbosidad::detalle) << "Terminado el calculo de las medidas de informacion\n";
  }

  ruleset R, R_dis, R_prev, R_prev_dis, R_mix, R_mix_prev;
  R.AddDomain(V);
  R_dis = R;
  R_prev = R;
  R_prev_dis = R;
  R_mix = R;
  R_mix_prev = R;

  multipoblacion G(n_clases, n_clases * 10, tama_valor, tama_nivel, 2.0 / tama_valor, 0, 0.01,
                   NFitness);

  const int limite = E.N_Examples();
  const int paso = static_cast<int>(options.tamano_episodio);
  // Para la línea de cada episodio (Verbosidad::episodios).
  const int total_episodios = (limite + paso - 1) / paso;
  // En cuántos episodios ha aprendido reglas, y el episodio actual.
  int it = 1, it2 = 0;
  const double percent = 0.10;
  double umbral_acierto = 0.7, acumulado = 0.0, accuracy_old = 0, acierto_anterior = 0;
  int inicio = 0;
  bool salir = false;
  const clock_t tiempo0 = clock();
  example_set E_Par;

  while (!salir) {
    int fin = inicio + paso;

    if (fin >= limite) {
      fin = limite;
      salir = true;
    }

    E_Par = E.SubSet(inicio, fin - 1);

    progreso(Verbosidad::detalle) << endl;
    progreso(Verbosidad::detalle)
        << "=================================================== Episodio de " << inicio << " a "
        << fin << " ====================================" << endl;
    const int episodio = it2 + 1, primer_ejemplo = inicio, ultimo_ejemplo = fin - 1;
    const int reglas_al_empezar = R_prev.N_rule();
    const auto empieza_el_episodio = std::chrono::steady_clock::now();

    const double consumido = 1.0 * (clock() - tiempo0) / CLOCKS_PER_SEC;
    progreso(Verbosidad::detalle) << "===================== Tiempo Consumido: " << consumido
                                  << "      ETA: "
                                  << (consumido / (it2 + 1)) * (total_episodios - it2) << endl;
    progreso(Verbosidad::detalle) << "=================================================== Tamano "
                                  << E_Par.N_Examples()
                                  << " ====================================" << endl
                                  << endl;
    progreso(Verbosidad::depuracion) << "[Paso: " << paso << "]" << endl;
    inicio = fin;

    progreso(Verbosidad::depuracion) << "Creando la tabla de adaptaciones.....\n";
    double*** Tabla = Crear_Tabla_Adaptaciones(V, E_Par);
    progreso(Verbosidad::depuracion) << "Tabla de adaptaciones creada!\n";

    progreso(Verbosidad::detalle) << "  Numero de Reglas: " << R_prev.N_rule() << "("
                                  << R_prev.N_rule(0.5) << ")" << endl;

    const double last_accuracy = accuracy_old;
    acumulado += last_accuracy;
    if (acumulado == 0) {
      umbral_acierto = 1;
    }
    progreso(Verbosidad::detalle)
        << "Aciertos de la descriptiva sobre el anterior conjunto de ejemplos : " << last_accuracy
        << endl;

    accuracy_old = Aciertos(R_prev, V, E_Par);
    progreso(Verbosidad::detalle)
        << "Aciertos de la descriptiva sobre el nuevo conjunto de ejemplos : " << accuracy_old
        << endl;
    const double acierto_al_empezar = accuracy_old;

    // El umbral sale del acierto sobre un episodio anterior: el de hace 10 cada 10 episodios y el
    // primero hasta el décimo. Los dos primeros episodios, y no hay anterior que mirar, 1.
    if (it2 >= 10 and it2 % 10 == 0) {
      example_set E_Anteriores = E.SubSet((it2 - 10) * paso, (it2 - 9) * paso - 1);
      const double acierto = Aciertos(R_prev, V, E_Anteriores);
      umbral_acierto = acierto - percent * acierto;
    } else if (it2 < 10 and it2 > 1) {
      example_set E_Anteriores = E.SubSet(0, paso - 1);
      const double acierto = Aciertos(R_prev, V, E_Anteriores);
      umbral_acierto = acierto - percent * acierto;
    } else if (it2 < 10) {
      umbral_acierto = 1;
    }

    it2++;

    progreso(Verbosidad::detalle) << "Diferencia de acierto entre los dos últimos episodios: "
                                  << acierto_anterior - last_accuracy << endl;
    acierto_anterior = last_accuracy;

    progreso(Verbosidad::detalle) << "En la iteración " << it << " el umbral se ha fijado a "
                                  << umbral_acierto << " (" << umbral_acierto * (1 + percent) << ")"
                                  << endl;

    R_prev.FixModifyToTrue();
    R_prev_dis.FixModifyToTrue();

    ActualizarPeso(G, V, E_Par, R_prev, Tabla);
    ActualizarPeso(G, V, E_Par, R_prev_dis, Tabla);

    R_prev_dis.Recalculate_Weight();
    R_prev.Recalculate_Weight();

    accuracy_old = Aciertos(R_prev, V, E_Par);
    progreso(Verbosidad::detalle) << "Aciertos despues de ajustar los pesos de las reglas: "
                                  << accuracy_old << endl;
    const double acierto_despuesAjustarPeso = accuracy_old;

    if (accuracy_old < umbral_acierto) {
      example_set E_Par_Not_Covered = E_Par;

      for (const int regla : Reglas_Que_Disparan(R_prev, E_Par_Not_Covered)) {
        R.Add(*R_prev.Get_Code_Rule(regla), R_prev.Get_N_Mas(regla), R_prev.Get_N_Menos(regla),
              false);

        const double nmas = R_prev_dis.Get_N_Mas(regla);
        const double nmenos = R_prev_dis.Get_N_Menos(regla);
        R_dis.Add(*R_prev_dis.Get_Code_Rule(regla), nmas, nmenos, false);
        R_mix.Add(*R_mix_prev.Get_Code_Rule(regla), nmas, nmenos, false);
      }

      progreso(Verbosidad::detalle)
          << "Reglas no filtradas: " << R.N_rule() << "(" << R.N_rule(0.5) << ")" << endl;

      double*** Tabla1 = Crear_Tabla_Adaptaciones(V, E_Par_Not_Covered);

      progreso(Verbosidad::detalle)
          << "\n-------------------------------------------> Nuevo numero de ejemplos: "
          << E_Par_Not_Covered.N_Examples() << endl;

      accuracy_old = Aciertos(R, V, E_Par_Not_Covered);
      progreso(Verbosidad::detalle)
          << "Aciertos de la descriptiva antes de lanzar el genetico: " << accuracy_old << endl;

      const int initial_size_of_rule = R.N_rule();
      string last_patron;
      bool primera_vez = true, reducir_reglas = false, Added_Rule;
      std::vector<int> reglas_a_eliminar;

      do {
        const int anterior_n = E_Par_Not_Covered.Not_Covered_Examples();
        E_Par_Not_Covered.Examples_per_Class(V.Consecuente(), n_clases, ejemplos_por_clase.data());

        std::vector<int> basica_para_regla(R.N_rule() + 1), errores_para_regla(R.N_rule() + 1);
        Obtener_Aciertos_y_Errores_Reglas_Aprendidas(E_Par_Not_Covered, basica_para_regla.data(),
                                                     errores_para_regla.data(), R.N_rule() + 1);

        const Evaluacion ev{G,
                            V,
                            E_Par_Not_Covered,
                            R,
                            reducir_reglas,
                            ejemplos_por_clase.data(),
                            Tabla1,
                            basica_para_regla.data(),
                            errores_para_regla.data(),
                            initial_size_of_rule};
        GA(ev, R, R_dis, R_mix, Med_Inform, primera_vez, reglas_a_eliminar, Added_Rule);

        primera_vez = false;

        Aciertos(R, V, E_Par_Not_Covered);
        const string current_patron = Patron1Regla(R_mix, V, R_mix.N_rule() - 1);
        if (current_patron == last_patron) {
          Added_Rule = false;
        }
        last_patron = current_patron;

        if (Added_Rule and !reglas_a_eliminar.empty()) {
          Filtrar_Reglas_New(R, R_dis, R_mix, reglas_a_eliminar);
          Aciertos(R, V, E_Par_Not_Covered);
        }

        if (G.ValorFitness(0) > 0 and anterior_n == E_Par_Not_Covered.Not_Covered_Examples()) {
          progreso(Verbosidad::detalle) << "+++++++++++++++++++++ ERROR ++++++++++++++++++++++\n";
        }

        if (G.ValorFitness(0) != -2 * E_Par_Not_Covered.N_Examples() and !reducir_reglas and
            !Added_Rule) {
          reducir_reglas = true;
          Added_Rule = true;
        }
      } while (Added_Rule);

      Destruir_Tabla_Adaptaciones(V, Tabla1);

      it++;

      if (it2 > 1) {
        example_set E_Anteriores = E.SubSet(it2 > 10 ? (it2 - 10) * paso : 0, inicio - 1);

        double*** Tabla2 = Crear_Tabla_Adaptaciones(V, E_Anteriores);

        ActualizarPeso(G, V, E_Anteriores, R, Tabla2);
        ActualizarPeso(G, V, E_Anteriores, R_dis, Tabla2);

        Destruir_Tabla_Adaptaciones(V, Tabla2);
      }

      R_prev.AddRuleset(R);
      R_prev_dis.AddRuleset(R_dis);
      R_mix_prev.AddRuleset(R_mix);
      progreso(Verbosidad::detalle)
          << "Aniadidas " << R.N_rule() - initial_size_of_rule << " reglas\n";

      accuracy_old = Aciertos(R_prev, V, E_Par);
      progreso(Verbosidad::detalle)
          << "Acierto antes de analizar patrones: " << accuracy_old << endl;

      if (acierto_despuesAjustarPeso < accuracy_old) {
        progreso(Verbosidad::detalle) << "............................. Analizando patrones "
                                         "entre reglas .......................\n";
        AnalizarConjuntosDeReglas(V, R_prev_dis, R_prev, R_mix_prev, R.N_rule());
        accuracy_old = Aciertos(R_prev, V, E_Par);
        progreso(Verbosidad::detalle)
            << "Acierto despues de analizar patrones: " << accuracy_old << endl;
      } else {
        for (int i = 0; i < R.N_rule(); i++) {
          R_prev.Remove(0);
          R_prev_dis.Remove(0);
          R_mix_prev.Remove(0);
        }
        accuracy_old = Aciertos(R_prev, V, E_Par);
      }
    }

    // Una línea por episodio. Solo con lo que ya se ha calculado: Aciertos marca los ejemplos y
    // llamarlo aquí cambiaría las reglas.
    if (se_escribe(Verbosidad::episodios)) {
      const std::chrono::duration<double> segundos =
          std::chrono::steady_clock::now() - empieza_el_episodio;
      progreso(Verbosidad::episodios) << std::format(
          "INSLV: episodio {} de {} (ejemplos {} a {}): reglas {} -> {}, acierto {:.3f} -> "
          "{:.3f}, {:.2f} s\n",
          episodio, total_episodios, primer_ejemplo, ultimo_ejemplo, reglas_al_empezar,
          R_prev.N_rule(), acierto_al_empezar, accuracy_old, segundos.count());
    }
    if (informe) {
      ofstream salida(task_res, std::ofstream::out | std::ofstream::app);
      salida << "*****************************************************************\n";
      for (int i = 0; i < R.N_rule(); i++) salida << i << ") " << R.SPrint(i) << endl;
      salida << endl;
    }

    while (R.N_rule() > 0) {
      R.Remove();
      R_dis.Remove();
      R_mix.Remove();
    }

    Destruir_Tabla_Adaptaciones(V, Tabla);
  }

  LiberaMedidasDeInformacion(Med_Inform, V.N_Antecedente());

  if (informe) {
    Escribir_Informe(task_res, V, E_Par, R_prev, R_prev_dis, R_mix_prev,
                     1.0 * (clock() - tiempo0) / CLOCKS_PER_SEC);
  }

  // Las descriptivas (R_prev) o sus versiones discriminantes (R_prev_dis), emparejadas regla a
  // regla.
  const ruleset& elegidas = options.reglas == TipoDeReglas::discriminantes ? R_prev_dis : R_prev;
  std::vector<Regla> reglas;
  for (int i = 0; i < elegidas.N_rule(); i++) {
    Regla regla;
    regla.antecedente = elegidas.Antecedente(i);
    regla.consecuente = elegidas.Class_of_rule(i);
    regla.cubiertos = elegidas.Get_N_Mas(i);
    regla.peso = elegidas.Get_Weight(i);
    reglas.push_back(regla);
  }
  return reglas;
}

}  // namespace inslv
