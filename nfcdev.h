#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <nfc/nfc.h>
#include <nfc/nfc-emulation.h>
#include <nfc/nfc-types.h>

#include "mifare.c"
//*********************************************************************************  
//CUSTOM KEY
static char mykeysA[] = {
0XFE,0XDC,0XBA,0X45,0X67,0X89, 
};

//DATA FOR BLOCK
static char format[] = {
0XFE,0XDC,0XBA,0X45,0X67,0X89,0xff,0x07,0x80,0x69,0xff,0xff,0xff,0xff,0xff,0xff,
};

//Default KEY
static char blankkeys[] = {
0xff,0xff,0xff,0xff,0xff,0xff, 
};



#define MAX_TARGET_COUNT 1
//*********************************************************************************
nfc_device *pnd;
nfc_target nt;
// Allocate only a pointer to nfc_context
nfc_context *context;
static mifare_param mp;
const char *acLibnfcVersion ;
static size_t num_keys = sizeof(mykeysA) / 6;
//*********************************************************************************
static char aplay_str[] = "aplay -t wav --test-nowait /var/www/media/beep.wav" ;
//*********************************************************************************
void pn532init(void) ;
void print_hex(const uint8_t *pbtData, const size_t szBytes) ;
int  poll_mifare(void) ;
void mifare_read(void) ;
void mifare_write(void) ;
void pn532close(void) ;
//*********************************************************************************	
void pn532init(void) 
{
	  // Initialize libnfc and set the nfc_context
	nfc_init(&context);
	if (context == NULL) {
		printf("Unable to init libnfc (malloc)\n");
		exit(EXIT_FAILURE);
	}

	// Display libnfc version
	acLibnfcVersion = nfc_version();
	printf("uses libnfc %s\n", acLibnfcVersion);	
	  
	// Open, using the first available NFC device which can be in order of selection:
	//   - default device specified using environment variable or
	//   - first specified device in libnfc.conf (/etc/nfc) or
	//   - first specified device in device-configuration directory (/etc/nfc/devices.d) or
	//   - first auto-detected (if feature is not disabled in libnfc.conf) device
    pnd = nfc_open(context, NULL);
	if (pnd == NULL) {
		printf("ERROR: %s\n", "Unable to open NFC device.");
		exit(EXIT_FAILURE);
	}
	// Set opened NFC device to initiator mode
	if (nfc_initiator_init(pnd) < 0) {
		nfc_perror(pnd, "nfc_initiator_init");
		exit(EXIT_FAILURE);
	}

	printf("NFC reader: %s opened\n", nfc_device_get_name(pnd));
}
//******************************************************************************
void print_hex(const uint8_t *pbtData, const size_t szBytes)
{
  size_t  szPos;

  for (szPos = 0; szPos < szBytes; szPos++) {
    printf("%02x  ", pbtData[szPos]);
  }
  printf("\n");
}
//******************************************************************************
void pn532close(void)
{
  // Close NFC device
  nfc_close(pnd);
  // Release the context
  nfc_exit(context);
}
//******************************************************************************
int poll_mifare(void)
{
  // Poll for a ISO14443A (MIFARE) tag
	int res = -1 ;
	
	const nfc_modulation nmMifare = {
		.nmt = NMT_ISO14443A,
		.nbr = NBR_106,
	};
	
	nfc_target ant[MAX_TARGET_COUNT];
	
	if( nfc_initiator_list_passive_targets(pnd , nmMifare , ant , MAX_TARGET_COUNT) > 0  )	{
	  if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) > 0) {
		/*printf("The following (NFC) ISO14443A tag was found:\n");
		printf("    ATQA (SENS_RES): ");
		print_hex(nt.nti.nai.abtAtqa, 2);
		printf("       UID (NFCID%c): ", (nt.nti.nai.abtUid[0] == 0x08 ? '3' : '1')) ;
		print_hex(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
		printf("      SAK (SEL_RES): ");
		print_hex(&nt.nti.nai.btSak, 1);
		if (nt.nti.nai.szAtsLen) {
		  printf("          ATS (ATR): ");
		  print_hex(nt.nti.nai.abtAts, nt.nti.nai.szAtsLen);
		}*/
		res = 1 ;
	  }
	}
	
	return res ;
}
//******************************************************************************
bool authenticate(int block, int keynum, bool keyB) {
        memcpy(mp.mpa.abtAuthUid, nt.nti.nai.abtUid,4);
        memcpy(mp.mpa.abtKey, &mykeysA[keynum], 6);

        bool res = nfc_initiator_mifare_cmd(pnd, (keyB ? MC_AUTH_B : MC_AUTH_A), block, &mp);
        if (res) {
                printf("Authentication succcessful on block %d\n",block);
        } else {
                printf("Authentication failed on block %d\n",block);
        }
        return res;
}
//******************************************************************************
bool writeblock(int block, char *data) {
        memcpy(mp.mpv.abtValue,data,16);
        bool res = nfc_initiator_mifare_cmd(pnd, MC_WRITE, block, &mp);
        if (res) {
                printf("Wrote to block %d successfully\n", block);
        } else {
                printf("Could not write to block %d\n",block);
        }
		return res ;
}
//******************************************************************************
bool readblocks(int start, int end) {
        int block ;
		bool res ;
        for (block=start; block<(end+1); block++) {
                res = nfc_initiator_mifare_cmd(pnd, MC_READ, block, &mp);
                if (res) {
                        printf("Block %d data: ",block);
                        //print_hex(mp.mpd.abtData,16);
                } else {
                        printf("Reading block %d failed\n",block);
                }
        }
		return res ;
}
//******************************************************************************
bool formatcard(void)
{
	uint8_t trailer ;
	uint8_t ii ;
	bool res , final_res = true;
		
	for( ii = 0 ; ii < 16 ; ii++ )	{
		trailer = ( ii * 4 ) + 3 ;
		
		memcpy(mp.mpa.abtAuthUid, nt.nti.nai.abtUid,4);
        memcpy(mp.mpa.abtKey, &blankkeys , 6);
        res = nfc_initiator_mifare_cmd(pnd, MC_AUTH_A, trailer, &mp);
		
        if (res) {
            printf("Authentication succcessful on block %d\n",trailer);
			if(writeblock( trailer, format ) == false)
				final_res = false ;
        } else {
            printf("Authentication failed on block %d\n",trailer);
			final_res = false ;
        }
	}
	return final_res ;
}
//******************************************************************************

