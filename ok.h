#ifndef _ok_h_
#define _ok_h_

#include <pcsclite.h>

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */

/* librfid */
#include "librfid/rfid.h"
#include "librfid/rfid_scan.h"
#include "librfid/rfid_reader.h"
#include "librfid/rfid_layer2.h"
#include "librfid/rfid_protocol.h"
#include "librfid/rfid_layer2_iso14443a.h"
#include "librfid/rfid_protocol_tcl.h"

#include <pcsclite.h>
#include <ifdhandler.h>
#include <debuglog.h>

#ifdef __cplusplus
extern "C"
{
#endif


static int reader_init(void);
static int l2_init(int layer2);
static int l3_init(int protocol);

RESPONSECODE initLib();
void closeLib();
RESPONSECODE sendData(PUCHAR, DWORD, PUCHAR, PDWORD, int);
void readUID(PDWORD, PUCHAR);
int readPresence();

#ifdef __cplusplus
}
#endif

#endif
