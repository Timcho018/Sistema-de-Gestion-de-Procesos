#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <clocale>

using namespace std;

// ESTRUCTURA DE PROCESO
struct Proceso {
    int id;
    string nombre;
    int prioridad; // 1 (alta) a 5 (baja)
    bool enEjecucion;

    Proceso(int i, string n, int p) : id(i), nombre(n), prioridad(p), enEjecucion(false) {}
};


// LISTA ENLAZADA: Gestor de Procesos
class ListaProcesos {
private:
    struct Nodo {
        Proceso dato;
        Nodo* siguiente;
        Nodo(const Proceso& p) : dato(p), siguiente(NULL) {} 
    };
    Nodo* cabeza;
    int contadorID;

public:
    ListaProcesos() : cabeza(NULL), contadorID(1) {}
    ~ListaProcesos() { vaciar(); }

    void vaciar() {
        while (cabeza) {
            Nodo* temp = cabeza;
            cabeza = cabeza->siguiente;
            delete temp;
        }
    }

    int generarID() { return contadorID++; }

    Nodo* buscarPorID(int id) {
        Nodo* actual = cabeza;
        while (actual) {
            if (actual->dato.id == id) return actual;
            actual = actual->siguiente;
        }
        return NULL;
    }

    bool existe(int id) { return buscarPorID(id) != NULL; }

    Proceso* obtener(int id) {
        Nodo* n = buscarPorID(id);
        return n ? &(n->dato) : NULL;
    }

    void insertar(const string& nombre, int prioridad) {
        if (prioridad < 1 || prioridad > 5) return;
        Proceso p(generarID(), nombre, prioridad);
        Nodo* nuevo = new Nodo(p);
        nuevo->siguiente = cabeza;
        cabeza = nuevo;
        cout << "Proceso '" << nombre << "' (ID: " << p.id << ") creado.\n";
    }

    bool eliminar(int id) {
        if (!cabeza) return false;
        if (cabeza->dato.id == id) {
            Nodo* temp = cabeza;
            cabeza = cabeza->siguiente;
            delete temp;
            cout << "Proceso ID " << id << " eliminado.\n";
            return true;
        }
        Nodo* actual = cabeza;
        while (actual->siguiente && actual->siguiente->dato.id != id) {
            actual = actual->siguiente;
        }
        if (!actual->siguiente) return false;
        Nodo* temp = actual->siguiente;
        actual->siguiente = temp->siguiente;
        delete temp;
        cout << "Proceso ID " << id << " eliminado.\n";
        return true;
    }

    bool modificarPrioridad(int id, int nuevaPrio) {
        if (nuevaPrio < 1 || nuevaPrio > 5) {
            cout << "Prioridad inv lida (1-5).\n";
            return false;
        }
        Nodo* n = buscarPorID(id);
        if (!n) return false;
        n->dato.prioridad = nuevaPrio;
        cout << "Prioridad actualizada a " << nuevaPrio << ".\n";
        return true;
    }

    void mostrar() const {
        if (!cabeza) {
            cout << "No hay procesos.\n";
            return;
        }
        cout << "\n--- PROCESOS ---\n";
        Nodo* actual = cabeza;
        while (actual) {
            cout << "ID: " << actual->dato.id
                 << " | Nombre: " << actual->dato.nombre
                 << " | Prioridad: " << actual->dato.prioridad
                 << " | Estado: " << (actual->dato.enEjecucion ? "Ejecutando" : "Listo")
                 << "\n";
            actual = actual->siguiente;
        }
        cout << "----------------\n";
    }

    // Para cola y pila: devolver prioridad por ID
    int obtenerPrioridad(int id) const {
        Nodo* actual = cabeza;
        while (actual) {
            if (actual->dato.id == id) return actual->dato.prioridad;
            actual = actual->siguiente;
        }
        return -1; // no existe
    }

    string obtenerNombre(int id) const {
        Nodo* actual = cabeza;
        while (actual) {
            if (actual->dato.id == id) return actual->dato.nombre;
            actual = actual->siguiente;
        }
        return "[Eliminado]";
    }

    void guardar(ofstream& out) const {
        // Contar nodos
        int count = 0;
        Nodo* actual = cabeza;
        while (actual) {
            count++;
            actual = actual->siguiente;
        }
        out << count << "\n";
        actual = cabeza;
        while (actual) {
            out << actual->dato.id << "\n"
                << actual->dato.nombre << "\n"
                << actual->dato.prioridad << "\n"
                << (actual->dato.enEjecucion ? "1" : "0") << "\n";
            actual = actual->siguiente;
        }
    }

    void cargar(ifstream& in) {
        vaciar();
        int n;
        in >> n;
        in.ignore();
        for (int i = 0; i < n; ++i) {
            int id, prio, exec;
            string nombre;
            in >> id;
            in.ignore();
            getline(in, nombre);
            in >> prio >> exec;
            Proceso p(id, nombre, prio);
            p.enEjecucion = (exec == 1);
            Nodo* nuevo = new Nodo(p);
            nuevo->siguiente = cabeza;
            cabeza = nuevo;
            if (id >= contadorID) contadorID = id + 1;
        }
    }
};

// COLA DE PRIORIDAD (solo almacena IDs)
class ColaPrioridad {
private:
    struct Nodo {
        int idProceso;
        Nodo* siguiente;
        Nodo(int id) : idProceso(id), siguiente(NULL) {}
    };
    Nodo* frente;
    ListaProcesos& lista;

public:
    ColaPrioridad(ListaProcesos& l) : frente(NULL), lista(l) {}

    ~ColaPrioridad() { vaciar(); }

    void vaciar() {
        while (frente) {
            Nodo* temp = frente;
            frente = frente->siguiente;
            delete temp;
        }
    }

    bool estaEnCola(int id) {
        Nodo* actual = frente;
        while (actual) {
            if (actual->idProceso == id) return true;
            actual = actual->siguiente;
        }
        return false;
    }

    void encolar(int id) {
        if (!lista.existe(id)) {
            cout << "Proceso ID " << id << " no existe.\n";
            return;
        }
        if (estaEnCola(id)) {
            cout << "El proceso ya est  en la cola.\n";
            return;
        }

        Nodo* nuevo = new Nodo(id);
        if (!frente || lista.obtenerPrioridad(id) < lista.obtenerPrioridad(frente->idProceso)) {
            nuevo->siguiente = frente;
            frente = nuevo;
        } else {
            Nodo* actual = frente;
            while (actual->siguiente &&
                   lista.obtenerPrioridad(actual->siguiente->idProceso) <= lista.obtenerPrioridad(id)) {
                actual = actual->siguiente;
            }
            nuevo->siguiente = actual->siguiente;
            actual->siguiente = nuevo;
        }
        lista.obtener(id)->enEjecucion = true;
        cout << "'" << lista.obtenerNombre(id) << "' encolado.\n";
    }

    bool desencolar() {
        if (!frente) {
            cout << "Cola vac a.\n";
            return false;
        }
        int id = frente->idProceso;
        string nombre = lista.obtenerNombre(id);
        lista.obtener(id)->enEjecucion = false;

        Nodo* temp = frente;
        frente = frente->siguiente;
        delete temp;

        cout << "Ejecutado: " << nombre << " (ID: " << id << ")\n";
        return true;
    }

    void mostrar() const {
        if (!frente) {
            cout << "Cola de CPU vac a.\n";
            return;
        }
        cout << "\n--- COLA DE CPU ---\n";
        Nodo* actual = frente;
        int pos = 1;
        while (actual) {
            cout << pos++ << ". " << lista.obtenerNombre(actual->idProceso)
                 << " (ID: " << actual->idProceso
                 << ", Prioridad: " << lista.obtenerPrioridad(actual->idProceso) << ")\n";
            actual = actual->siguiente;
        }
        cout << "--------------------\n";
    }

    void guardar(ofstream& out) const {
        // Contar nodos
        int count = 0;
        Nodo* actual = frente;
        while (actual) {
            count++;
            actual = actual->siguiente;
        }
        out << count << "\n";
        actual = frente;
        while (actual) {
            out << actual->idProceso << "\n";
            actual = actual->siguiente;
        }
    }

    void cargar(ifstream& in, ListaProcesos& listaRef) {
        vaciar();
        int n;
        in >> n;
        for (int i = 0; i < n; ++i) {
            int id;
            in >> id;
            if (listaRef.existe(id)) {
                encolar(id); // reutiliza l gica de inserci n segura
            }
        }
    }
};

// ================================
// PILA DE MEMORIA (solo almacena IDs)
// ================================
class PilaMemoria {
private:
    struct Nodo {
        int idProceso;
        Nodo* siguiente;
        Nodo(int id) : idProceso(id), siguiente(NULL) {}
    };
    Nodo* tope;
    ListaProcesos& lista;
    static const int MAX_BLOQUES = 10;
    int bloquesUsados;

public:
    PilaMemoria(ListaProcesos& l) : tope(NULL), lista(l), bloquesUsados(0) {}
    ~PilaMemoria() { vaciar(); }

    void vaciar() {
        while (tope) {
            Nodo* temp = tope;
            tope = tope->siguiente;
            delete temp;
        }
        bloquesUsados = 0;
    }

    bool asignar(int id) {
        if (!lista.existe(id)) {
            cout << "Proceso ID " << id << " no existe.\n";
            return false;
        }
        if (bloquesUsados >= MAX_BLOQUES) {
            cout << "Memoria llena (m x. " << MAX_BLOQUES << " bloques).\n";
            return false;
        }
        Nodo* nuevo = new Nodo(id);
        nuevo->siguiente = tope;
        tope = nuevo;
        bloquesUsados++;
        cout << "Memoria asignada a '" << lista.obtenerNombre(id) << "'. (" << bloquesUsados << "/" << MAX_BLOQUES << ")\n";
        return true;
    }

    bool liberar() {
        if (!tope) {
            cout << "No hay bloques asignados.\n";
            return false;
        }
        int id = tope->idProceso;
        string nombre = lista.obtenerNombre(id);
        Nodo* temp = tope;
        tope = tope->siguiente;
        delete temp;
        bloquesUsados--;
        cout << "Memoria liberada de '" << nombre << "'. Bloques libres: " << (MAX_BLOQUES - bloquesUsados) << "\n";
        return true;
    }

    void mostrar() const {
        cout << "\n--- MEMORIA ---\n";
        cout << "Usados: " << bloquesUsados << " / " << MAX_BLOQUES << "\n";
        if (tope) {
            cout << "Bloques (LIFO):\n";
            Nodo* actual = tope;
            int i = 1;
            while (actual) {
                cout << i++ << ". " << lista.obtenerNombre(actual->idProceso) << " (ID: " << actual->idProceso << ")\n";
                actual = actual->siguiente;
            }
        }
        cout << "---------------\n";
    }

    void guardar(ofstream& out) const {
        // Contar nodos
        int count = 0;
        Nodo* actual = tope;
        while (actual) {
            count++;
            actual = actual->siguiente;
        }
        out << count << "\n";
        actual = tope;
        while (actual) {
            out << actual->idProceso << "\n";
            actual = actual->siguiente;
        }
    }

    void cargar(ifstream& in, ListaProcesos& listaRef) {
        vaciar();
        int n;
        in >> n;
        for (int i = 0; i < n; ++i) {
            int id;
            in >> id;
            if (listaRef.existe(id)) {
                // Reasignar manualmente sin pasar por validaci n (estado previo)
                Nodo* nuevo = new Nodo(id);
                nuevo->siguiente = tope;
                tope = nuevo;
                bloquesUsados++;
            }
        }
    }
};

// ================================
// FUNCIONES DE ENTRADA SEGURA (COMPATIBLES CON C++98)
// ================================
int leerEntero(const string& msg, int min = -2147483647, int max = 2147483647) {
    int v;
    while (true) {
        cout << msg;
        if (cin >> v && v >= min && v <= max) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return v;
        }
        cout << "Valor inv lido. Intente de nuevo.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string leerCadena(const string& msg) {
    string s;
    while (true) {
        cout << msg;
        getline(cin, s);
        if (!s.empty()) {
            size_t start = s.find_first_not_of(" \t");
            size_t end = s.find_last_not_of(" \t");
            if (start != string::npos) {
                return s.substr(start, end - start + 1);
            }
        }
        cout << "No puede estar vac o.\n";
    }
}

// ================================
// FUNCI N PRINCIPAL
// ================================
int main() {
	setlocale(LC_ALL, "Spanish");
    ListaProcesos lista;
    ColaPrioridad cola(lista);
    PilaMemoria pila(lista);

    // --- CARGAR ESTADO ---
    ifstream fin("sistema_procesos.dat");
    if (fin.is_open()) {
        cout << "Cargando estado del sistema...\n";
        lista.cargar(fin);

        // Cargar cola
        cola.cargar(fin, lista);

        // Cargar memoria
        pila.cargar(fin, lista);

        fin.close();
        cout << "Estado cargado.\n\n";
    }

    int opcion;
    do {
        cout << "\n=== SISTEMA DE GESTI N DE PROCESOS ===\n";
        cout << "1. Gestor de Procesos\n";
        cout << "2. Planificador de CPU\n";
        cout << "3. Gestor de Memoria\n";
        cout << "4. Guardar y Salir\n";
        cout << "=====================================\n";
        opcion = leerEntero("Opci n: ", 1, 4);

        if (opcion == 1) {
            int sub;
            do {
                cout << "\n[1] Agregar\n[2] Eliminar\n[3] Modificar prioridad\n[4] Mostrar\n[5] Volver\n";
                sub = leerEntero("Opci n: ", 1, 5);
                if (sub == 1) {
                    string nom = leerCadena("Nombre: ");
                    int prio = leerEntero("Prioridad (1-5): ", 1, 5);
                    lista.insertar(nom, prio);
                } else if (sub == 2) {
                    int id = leerEntero("ID a eliminar: ");
                    if (lista.existe(id)) lista.eliminar(id);
                    else cout << "ID no encontrado.\n";
                } else if (sub == 3) {
                    int id = leerEntero("ID: ");
                    if (!lista.existe(id)) { cout << "No existe.\n"; continue; }
                    int p = leerEntero("Nueva prioridad (1-5): ", 1, 5);
                    lista.modificarPrioridad(id, p);
                } else if (sub == 4) {
                    lista.mostrar();
                }
            } while (sub != 5);
        }
        else if (opcion == 2) {
            int sub;
            do {
                cout << "\n[1] Encolar\n[2] Ejecutar\n[3] Mostrar cola\n[4] Volver\n";
                sub = leerEntero("Opci n: ", 1, 4);
                if (sub == 1) {
                    lista.mostrar();
                    int id = leerEntero("ID del proceso: ");
                    cola.encolar(id);
                } else if (sub == 2) {
                    cola.desencolar();
                } else if (sub == 3) {
                    cola.mostrar();
                }
            } while (sub != 4);
        }
        else if (opcion == 3) {
            int sub;
            do {
                cout << "\n[1] Asignar\n[2] Liberar\n[3] Estado\n[4] Volver\n";
                sub = leerEntero("Opci n: ", 1, 4);
                if (sub == 1) {
                    lista.mostrar();
                    int id = leerEntero("ID del proceso: ");
                    pila.asignar(id);
                } else if (sub == 2) {
                    pila.liberar();
                } else if (sub == 3) {
                    pila.mostrar();
                }
            } while (sub != 4);
        }
        else if (opcion == 4) {
            cout << "\nGuardando...\n";
            ofstream fout("sistema_procesos.dat");
            if (fout.is_open()) {
                lista.guardar(fout);
                cola.guardar(fout);
                pila.guardar(fout);
                fout.close();
                cout << "Guardado exitoso.\n";
            } else {
                cout << "Error al guardar.\n";
            }
            cout << "Sistema cerrado.\n";
        }
    } while (opcion != 4);

    return 0;
}
