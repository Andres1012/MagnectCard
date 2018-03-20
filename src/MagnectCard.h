/*
   Lector.h

    Created on: Apr 4, 2017
        Author: Andres
*/

#ifndef LECTOR_H_
#define LECTOR_H_

#include "Arduino.h"


#define SW1			32
#define BS          33
#define DATA        25
#define CP          26
#define STROBE      27




class MagnectCard {
  public:
    MagnectCard();
	MagnectCard(int _sw1, int _bs, int _data, int _cp, int _strobe);
    ~MagnectCard();
    void init(void);
	void init(int _sw1, int _bs, int _data, int _cp, int _strobe);
    String getLectorID(void);
    bool available(void);
    char LECTOR_dato(void);
    void setAttachInterrupt(uint8_t pin, void (*callBack)(void), byte modePin)
    {
      attachInterrupt(digitalPinToInterrupt(pin), callBack, modePin);
    }
  private:
    char Lector_ID[11];
    unsigned char Flag_Listo = 0;
    unsigned char Flag_Inicio = 0;
    static byte state;
	int sw1;
	int bs;
	int data;
	int cp;
	int strobe;
    static void onPinChange(void);
};

#endif /* LECTOR_H_ */

