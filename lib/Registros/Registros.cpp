#include <Arduino.h>
#include "Registros.h"

Registros::Registros(){
_Registro = 0;

}

int Registros::GetRegistro (void) {
    return _Registro;
}

void Registros::UpdateRegistro(int registro){
    _Registro = registro;
}