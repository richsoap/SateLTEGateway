#include <stdio.h>
#include <sys/types.h>
#include "S1AP-PDU.h"
#include "ANY.h"
#include "S1ap-IE.h"
#include "S1ap-Global-ENB-ID.h"
#define BUFFER_LENGTH 8
static int write_out(const void *buffer, size_t size, void *app_key) {
	FILE*out_fp = app_key;
	size_t wrote = fwrite(buffer, 1, size, out_fp);
	return wrote == size? 0 : 1;
}
int main() {
	S1ap_IE_t *item0;
	asn_enc_rval_t ec;
	item0->id = S1ap_ProtocolIE_ID_id_Global_ENB_ID;
	item0->criticality = S1ap_Criticality_reject;
	S1ap_Global_ENB_ID_t* global_enb_id;
	global_enb_id = calloc(1,sizeof(S1ap_Global_ENB_ID_t));
	uint8_t macroid[] = {0x1a,0x2d,0x00};
	global_enb_id->eNB_ID.choice.macroENB_ID.buf = macroid;
	global_enb_id->eNB_ID.choice.macroENB_ID.size = 3;
	global_enb_id->eNB_ID.choice.macroENB_ID.bits_unused = 4;
	uint8_t plmn[] = {0x00,0xf1,0x10};
	global_enb_id->pLMNidentity.buf = plmn;
	global_enb_id->pLMNidentity.size = 3;
	ANY_t* val = ANY_new_fromType(&asn_DEF_S1ap_Global_ENB_ID, global_enb_id);
	item0->value = *val;
	FILE *fp = fopen("test.txt","wb");
	ec = uper_encode(&asn_DEF_S1ap_IE, 0, item0, write_out, fp);
	fclose(fp);
	xer_fprint(stdout, &asn_DEF_S1ap_IE, item0);
	return 0;
}
