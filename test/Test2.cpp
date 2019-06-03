/**
 * Programa de prueba de la parte avanzada 1 de la práctica 3
 *
 * Los errores contemplados son:
 *   - ID no corresponde -> ignorar el mensaje, no devolver nada
 *   - CRC incorrecto -> ignorar el mensaje, no devolver nada
 *   - Función no implementada -> error 1
 *   - Fuera de rango -> error 2
 *   - Dato inválido -> error 3
 *
 * \author Alberto Hamilton Castro
 */

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "../ModbusServer.hpp"

#define ID_DEV  6

//si vale 0 se haran todas las pruebas aunque fallen
//si vale 1 se parará desde que falle la primera
int ExitIfFail = 1;

#define BVEC(v) vector<byte>( v, v + sizeof(v)/sizeof(byte) )

typedef unsigned char byte;

using namespace std;
using namespace modbus;

ostream& operator<<(ostream &os, const vector<byte> &v) {
  os << "[("  << v.size() << ") " << setfill('0') << hex ;
  for ( unsigned int ba = 0; ba < v.size(); ba++)
    os << setw(2) << (int)v[ba] << ' ';
  os << "] " << dec;
  return os;
}

int Test(ModbusServer &mbs, string title, vector<byte> petition, vector<byte> expected) {
    vector<byte> response = mbs.Petition(petition);
    if(response != expected) {
        std::cout << Util::Color(BOLD, "[ ") << Util::Color(BOLD, Util::Color(RED, "FAILURE")) << Util::Color(BOLD, " ]");
        std::cout << " - Test '" << title << "'" << std::endl;
        std::cout << "Sent = " << petition << std::endl;
        std::cout << "Expected = " << expected << std::endl;
        std::cout << "Received = " << response << std::endl;
        if (ExitIfFail) exit(1);
        return 0;
    }
    std::cout << Util::Color(BOLD, "[ ") << Util::Color(BOLD, Util::Color(GREEN, "SUCCESS")) << Util::Color(BOLD, " ]");
    std::cout << " - Test '" << title << "'" << std::endl;
    return 1;
}

int main (int argc, char *argv[]) {
    if (argc > 1) {
        char* ptfin;
        long int val = strtol(argv[1], &ptfin, 10);
        if (ptfin == argv[1]) {
            std::cerr << "Primer parámetro '" << argv[1] << "' no parece entero" << std::endl;
            exit(2);
        }
        if (val != 0 && val != 1) {
            std::cerr << "El parametro debe ser 0 ó 1" << std::endl;
            exit(2);
        }
        ExitIfFail = val;
    }
    ModbusServer mbs(ID_DEV);
    int total = 0, ok = 0;

  // ==========================================================
  //Código generado automaticamente
  {
    total++;
    byte peti[] = { 0x08, 0x01, 0x00, 0x00, 0x00, 0x14, 0x3C, 0x9C };
    byte resp[] = {};
    ok += Test(mbs, "Leemos las salidas digitales con ID incorrecto", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x04, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0x98 };
    byte resp[] = {};
    ok += Test(mbs, "Leemos las salidas analogicas con ID incorrecto", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00 };
    byte resp[] = {};
    ok += Test(mbs, "Error CRC incorrecto, leyendo las salidas digitales de la 1 a la 20", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x06, 0x00, 0x04, 0x16, 0x10, 0x00, 0x00 };
    byte resp[] = {};
    ok += Test(mbs, "Error CRC incorrecto, forzando salida analogica unica 5 a valor 5648", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x07, 0x00, 0x00, 0x00, 0x01, 0x74, 0x7D };
    byte resp[] = { 0x06, 0x87, 0x01, 0x33, 0xF1 };
    ok += Test(mbs, "Funcion inválida", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x09, 0x00, 0x00, 0x00, 0x01, 0x1D, 0xBC };
    byte resp[] = { 0x06, 0x89, 0x01, 0x37, 0x91 };
    ok += Test(mbs, "Funcion inválida", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x03, 0x00, 0x0B, 0x00, 0x01, 0xF4, 0x7F };
    byte resp[] = { 0x06, 0x83, 0x02, 0x71, 0x30 };
    ok += Test(mbs, "Error, lectura analogica en 11", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x03, 0x00, 0x0A, 0x00, 0x06, 0xE4, 0x7D };
    byte resp[] = { 0x06, 0x83, 0x02, 0x71, 0x30 };
    ok += Test(mbs, "Error, lectura analogica de 10 a 15", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x03, 0x00, 0x01, 0x00, 0x0B, 0x54, 0x7A };
    byte resp[] = { 0x06, 0x83, 0x02, 0x71, 0x30 };
    ok += Test(mbs, "Error, lectura analogica de 1 a 11", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x01, 0x00, 0x15, 0x00, 0x01, 0xED, 0xB9 };
    byte resp[] = { 0x06, 0x81, 0x02, 0x70, 0x50 };
    ok += Test(mbs, "Error, Leemos las salida digital 11", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x01, 0x00, 0x01, 0x00, 0x15, 0xAD, 0xB2 };
    byte resp[] = { 0x06, 0x81, 0x02, 0x70, 0x50 };
    ok += Test(mbs, "Error, Leemos las salida digital de 1 a 21", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x01, 0x00, 0x0F, 0x00, 0x0B, 0x4C, 0x79 };
    byte resp[] = { 0x06, 0x81, 0x02, 0x70, 0x50 };
    ok += Test(mbs, "Error, Leemos las salida digital de 15 a 25", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x05, 0x00, 0x15, 0xFF, 0x00, 0x9C, 0x49 };
    byte resp[] = { 0x06, 0x85, 0x02, 0x72, 0x90 };
    ok += Test(mbs, "Error, escritura digital única en 21", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x05, 0x00, 0x32, 0xFF, 0x00, 0x2C, 0x42 };
    byte resp[] = { 0x06, 0x85, 0x02, 0x72, 0x90 };
    ok += Test(mbs, "Error, escritura digital única en 50", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x0F, 0x00, 0x15, 0x00, 0x01, 0x01, 0x01, 0xA3, 0x72 };
    byte resp[] = { 0x06, 0x8F, 0x02, 0x74, 0x30 };
    ok += Test(mbs, "Error, escritura digital multiple en 21", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x0F, 0x00, 0x01, 0x00, 0x15, 0x03, 0x00, 0x00, 0x00, 0xCC, 0xAF };
    byte resp[] = { 0x06, 0x8F, 0x02, 0x74, 0x30 };
    ok += Test(mbs, "Error, escritura digital multiple de 1 a 21", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x0F, 0x00, 0x0A, 0x00, 0x10, 0x02, 0x00, 0x00, 0xC4, 0xBA };
    byte resp[] = { 0x06, 0x8F, 0x02, 0x74, 0x30 };
    ok += Test(mbs, "Error, escritura digital multiple de 10 a 25", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x06, 0x00, 0x0B, 0x00, 0x01, 0x38, 0x7F };
    byte resp[] = { 0x06, 0x86, 0x02, 0x72, 0x60 };
    ok += Test(mbs, "Error, estritura analogica única en 11", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x06, 0x00, 0x1E, 0x00, 0x01, 0x29, 0xBB };
    byte resp[] = { 0x06, 0x86, 0x02, 0x72, 0x60 };
    ok += Test(mbs, "Error, estritura analogica única en 30", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x10, 0x00, 0x0B, 0x00, 0x01, 0x02, 0x00, 0x01, 0x40, 0xDB };
    byte resp[] = { 0x06, 0x90, 0x02, 0x7C, 0x00 };
    ok += Test(mbs, "Error, escritura analogica múltiple en 11", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x10, 0x00, 0x01, 0x00, 0x0B, 0x16, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x00, 0x0B, 0x45, 0x75 };
    byte resp[] = { 0x06, 0x90, 0x02, 0x7C, 0x00 };
    ok += Test(mbs, "Error, escritura analogica múltiple de 1 a 11", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x10, 0x00, 0x02, 0x00, 0x10, 0x20, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x00, 0x0B, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x0E, 0x00, 0x0F, 0x00, 0x10, 0x00, 0x11, 0x00, 0x12, 0x00, 0x13, 0x00, 0x14, 0x37, 0xE2 };
    byte resp[] = { 0x06, 0x90, 0x02, 0x7C, 0x00 };
    ok += Test(mbs, "Error, escritura analogica múltiple de 5 a 20", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x05, 0x00, 0x01, 0xFF, 0xFF, 0x9C, 0x0D };/////////////-----------------------------------
    byte resp[] = { 0x06, 0x85, 0x03, 0xB3, 0x50 };
    ok += Test(mbs, "Forzamos única salida digital 1 con valor incorrecto FF FF", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x05, 0x00, 0x05, 0x01, 0x00, 0xDD, 0xEC };
    byte resp[] = { 0x06, 0x85, 0x03, 0xB3, 0x50 };
    ok += Test(mbs, "Forzamos única salida digital 5 con valor incorrecto 01 00", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x04, 0x00, 0x00, 0x00, 0xAD, 0x30 };
    byte resp[] = { 0x06, 0x84, 0x03, 0xB2, 0xC0 };
    ok += Test(mbs, "Paquete Leer todas las entradas analogicas mal formado", BVEC(peti), BVEC(resp) );
  }
  {
    total++;
    byte peti[] = { 0x06, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x02, 0xF4, 0x19 };
    byte resp[] = { 0x06, 0x90, 0x03, 0xBD, 0xC0 };
    ok += Test(mbs, "Escritura analogica múltiple de 1 a 2, mal formada", BVEC(peti), BVEC(resp) );
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // Conclusiones
    std::cout << "\nTests = " << total << " | Succesful: " << Util::Color(BOLD, Util::Color(GREEN, std::to_string(ok))) << " | Failed: " << Util::Color(BOLD, Util::Color(RED, std::to_string(total-ok))) << std::endl;
    if(total == ok) std::cout << Util::Color(BOLD, Util::Color(GREEN, "ALL TESTS SUCCESSFUL"))  << endl;
}