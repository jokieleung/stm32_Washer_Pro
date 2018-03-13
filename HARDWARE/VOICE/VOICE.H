#ifndef __VOICE_H
#define __VOICE_H	 
#include "sys.h"


#define WASH_VOICE PFin(0)
#define DRY_VOICE PFin(1)
#define WASH_DRY_VOICE PFin(2)


void Voice_Init(void);
void VoiceDetect(void);



		 				    
#endif
