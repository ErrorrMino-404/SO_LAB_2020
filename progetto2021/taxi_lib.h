#ifndef _TX_LIB_
#define _TX_LIB_
#include "maps.h"
#include "master_lib.h"

/*allocazione taxi nella mappa*/
void allocate_taxi (int, int, int ,slot*, int);
/*quando riceve il signale di alarm invia un messaggio al master facendo
uccidere il processo taxi*/

#endif