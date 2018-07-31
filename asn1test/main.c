#include <stdio.h>
#include <sys/types.h>
#include "S1AP-PDU.h"
#include "ANY.h"
#include "S1ap-IE.h"
#include "S1ap-Global-ENB-ID.h"
#include "S1ap-ENBname.h"
#define BUFFER_LENGTH 8
static int write_out(const void *buffer, size_t size, void *app_key) {
	FILE*out_fp = app_key;
	//printf("size is %d\n", (int)size);
	size_t wrote = fwrite(buffer, 1, size, out_fp);
	uint8_t* tmp = (uint8_t*)buffer;
	for(int i = 0;i < size;i ++)
		printf("%02x:", tmp[i]);
	return wrote == size? 0 : 1;
}
static int octet_out(const void *buffer, size_t size, void *app_key) {
	int result;
	printf("octet size is %2d\n", (int)size);
	result = OCTET_STRING_fromBuf(app_key, buffer, size);
	printf("result is %d", result);
	return result;
}

S1ap_IE_t* getItem0() {
	S1ap_IE_t *item0;
	asn_enc_rval_t ec;
	int result;
	item0 = calloc(1, sizeof(S1ap_IE_t));
	assert(item0);
	item0->id = S1ap_ProtocolIE_ID_id_Global_ENB_ID;
	item0->criticality = S1ap_Criticality_reject;
	S1ap_Global_ENB_ID_t* global_enb_id;
	global_enb_id = calloc(1,sizeof(S1ap_Global_ENB_ID_t));
	assert(global_enb_id);
	uint8_t macroid[] = {0x1a,0x2d,0x00,0x00};
	global_enb_id->eNB_ID.present = S1ap_ENB_ID_PR_macroENB_ID;
	global_enb_id->eNB_ID.choice.macroENB_ID.buf = macroid;
	global_enb_id->eNB_ID.choice.macroENB_ID.size = 4;
	global_enb_id->eNB_ID.choice.macroENB_ID.bits_unused = 4;
	uint8_t plmn[] = {0x00,0xf1,0x10};
	OCTET_STRING_fromBuf(&(global_enb_id->pLMNidentity), plmn, 3);
	//ec = uper_encode(&asn_DEF_S1ap_Global_ENB_ID, global_enb_id, octet_out, &(item0->value));
	//ANY_fromType(&(item0->value), &asn_DEF_S1ap_Global_ENB_ID, global_enb_id);
	ANY_fromType_per(&(item0->value), &asn_DEF_S1ap_Global_ENB_ID, global_enb_id);
	return item0;	
}

S1ap_IE_t* getItem1() {
	S1ap_IE_t *item;
	asn_enc_rval_t ec;
	item = calloc(1, sizeof(S1ap_IE_t));
	assert(item);
	item->id = S1ap_ProtocolIE_ID_id_eNBname;
	item->criticality = S1ap_Criticality_ignore;
	S1ap_ENBname_t name;
	name.buf = "enb1a2d0";
	name.size = 7;
	ANY_fromType(&(item->value), &asn_DEF_S1ap_ENBname, &name);
	//ANY_fromType_per(&(item->value), &asn_DEF_S1ap_ENBname, &name);
	return item;
}

int main() {
	S1ap_IE_t* item = getItem0();
	asn_enc_rval_t ec;
	FILE *fp = fopen("test.txt","wb");
	//ec = der_encode(&asn_DEF_S1ap_IE, item, write_out, fp);
	ec = uper_encode(&asn_DEF_S1ap_IE, 0, item, write_out, fp);
	fclose(fp);
	printf("\n%d\n", (int)ec.encoded);
	xer_fprint(stdout, &asn_DEF_S1ap_IE, item);
	return 0;
}
