/*
   Lector.c

	Created on: Apr 4, 2017
		Author: Andres
*/

#include "MagnectCard.h"

//volatile byte state=0;
byte MagnectCard::state = 0;

MagnectCard::MagnectCard() {
	init();
}
MagnectCard::~MagnectCard() {

}
MagnectCard::MagnectCard(int _sw1, int _bs, int _data, int _cp, int _strobe) {
	init(_sw1, _bs, _data, _cp, _strobe);
}
void MagnectCard::init(int _sw1, int _bs, int _data, int _cp, int _strobe) {
	sw1 = _sw1;
	bs = _bs;
	data = _data;
	cp = _cp;
	strobe = _strobe;
	pinMode(sw1, INPUT_PULLUP);
	pinMode(bs, INPUT_PULLUP);
	pinMode(data, INPUT_PULLUP);
	pinMode(cp, INPUT_PULLUP);
	pinMode(strobe, INPUT_PULLUP);
}

/*Default config pins to ESP32.*/
void MagnectCard::init(void) {
	sw1 = SW1;
	bs = BS;
	data = DATA;
	cp = CP;
	strobe = STROBE;
	pinMode(SW1, INPUT_PULLUP);
	pinMode(BS, INPUT_PULLUP);
	pinMode(DATA, INPUT_PULLUP);
	pinMode(CP, INPUT_PULLUP);
	pinMode(STROBE, INPUT_PULLUP);
	//attachInterrupt(STROBE,STROBE_OUT, FALLING);
	// attachInterrupt(STROBE, leftInterruptHandler, FALLING);
}

String MagnectCard::getLectorID(void) {
	Lector_ID[strlen(Lector_ID) - 1] = 0;
	return String(Lector_ID);
}


bool MagnectCard::available(void) {
	return LECTOR_dato() == 2 ? true : false;
}




/******************************************************************************
   LECTOR_Dato(void)

   entrada: -----
   salida : 0. No lee tarjeta
	  1. Tarjeta invalida
	  2. Tarjeta valida
	  3. Error de lectura (paridad)
	  4. Error No coincide 10mo digito con la funcion xor
******************************************************************************/

char MagnectCard::LECTOR_dato(void) {


	signed char cntr_lector;
	unsigned char cntr_bits;
	unsigned char r = 0;
	unsigned char dato;
	unsigned char dato_1;
	unsigned char dato_2;
	char Lector_ID_S[25];
	unsigned char paridad;
	unsigned char aux_p;
	unsigned char Flag_Inicio = 0;

	if (digitalRead(sw1) == LOW) {
		delay(50);
		while (digitalRead(bs) == LOW) {
			delay(20);
			yield();
		}
		Flag_Inicio = 1;
	}
	if (Flag_Inicio) {
		cntr_lector = 24;
		for (r = 0; r < 25; r++) Lector_ID_S[r] = 0;
		for (r = 0; r < 11; r++) Lector_ID[r] = 0;
		setAttachInterrupt(strobe, onPinChange, FALLING);
		do {
			cntr_bits = 0;
			do {
				Lector_ID_S[cntr_lector] = (Lector_ID_S[cntr_lector] >> 1);
				do {
					if (digitalRead(sw1) == HIGH) {
						break;
					}
					//         delay(1);
				} while (state <= 0); // Cuando hay un falling edge del strobe la bandera se pone a 1
				state = 0;
				if (digitalRead(data) == LOW) Lector_ID_S[cntr_lector] |= 0x80;
				else Lector_ID_S[cntr_lector] &= ~0x80;
				cntr_bits++;
			} while ((cntr_bits < 8) && (digitalRead(sw1) == LOW));
			cntr_lector--;
			if (cntr_lector < 0) cntr_lector = 0;
		} while ((digitalRead(sw1) == LOW) && (cntr_lector > 0));

		Flag_Inicio = 0;
		detachInterrupt(strobe);
		// Inicia busqueda del start centinel = 0x1a (con el bit de paridad)
		cntr_lector = 0;

		cntr_bits = 0;
		dato = 0;
		do {
			dato = dato << 1;
			if ((Lector_ID_S[cntr_lector] & 0x80) == 0x80) dato |= 0x01;
			else dato &= ~0x01;
			Lector_ID_S[cntr_lector] = Lector_ID_S[cntr_lector] << 1;
			cntr_bits++;
			if (cntr_bits > 7) {
				cntr_bits = 0;
				cntr_lector++;
			}
			dato &= 0x1f;
			//      if((dato == 0x1a) && (cntr_lector < 12)) dato = 0x00;
			if ((dato == 0x1a) && (cntr_lector < 10)) dato = 0x00;
		} while ((cntr_lector < 25) && (dato != 0x1a));


		// Checa el contador para determinar si la lectura fue correcta o no.
		// Si se checaron los 25 bytes recibidos, es indicativo que no se encontro
		// el start centinel y por lo tanto regresa un estado de tarjeta invalida
		if (cntr_lector > 24) {
			return (1);
		}

		// Busqueda ID
		dato = 0;
		dato_1 = 0;
		dato_2 = Lector_ID_S[cntr_lector];
		paridad = 1;
		do {
			Lector_ID[dato] = Lector_ID[dato] >> 1;
			if ((dato_2 & 0x80) == 0x80) Lector_ID[dato] |= 0x10;
			else {
				Lector_ID[dato] &= ~0x10;
				if (dato_1 != 4) paridad ^= 0x01;
			}
			dato_2 = dato_2 << 1;
			dato_1++;
			if (dato_1 > 4) {
				aux_p = Lector_ID[dato] >> 4;
				if (aux_p != paridad) {
					return (3);
				}
				Lector_ID[dato] = (Lector_ID[dato] & 0x0f) + '0';   // conversion a ASCCI
				dato_1 = 0;
				paridad = 1;
				dato++;
			}
			cntr_bits++;
			if (cntr_bits > 7) {
				cntr_bits = 0;
				cntr_lector++;
				dato_2 = Lector_ID_S[cntr_lector];
			}
		} while ((cntr_lector < 25) && (dato < 10));
		Flag_Listo = 1;

			for (r = 0; r < 9; r++) {
				if ((Lector_ID[r] <= 0x2F) || (Lector_ID[r] >= 0x3A)) // Si es uncaracter no valido regresa error.
				{
					return (3);
				}

			}
		return (2);
	}
	return (0);
}

void MagnectCard::onPinChange(void) {
	state++;
	// handler here
}



