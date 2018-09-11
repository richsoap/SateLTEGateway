#include <stdio.h>
#include <sys/types.h>
#include "S1AP-PDU.h"
#include "ANY.h"
#include "S1ap-IE.h"
#include "S1ap-Global-ENB-ID.h"
#include "S1ap-ENBname.h"
#include "S1ap-PLMNidentity.h"
#include "S1ap-SupportedTAs-Item.h"
#include "S1ap-SupportedTAs.h"
#include <ALIGN.h>
#define BUFFER_LENGTH 8
static int write_out(const void *buffer, size_t size, void *app_key) {
	FILE*out_fp = app_key;
	printf("size is %d\n", (int)size);
	//size_t wrote = fwrite(buffer, 1, size, out_fp);
	size_t wrote = size;
	uint8_t* tmp = (uint8_t*) buffer;
	uint8_t* tarbuffer = calloc(size, sizeof(uint8_t));
	wrote = asn_squeeze_water(buffer, size, tarbuffer);
	printf("\nsrc--");
	for(int i = 0;i < size;i ++)
		printf("%02x:", tmp[i]);
	printf("\ntar--");
	for(int i = 0;i < wrote;i ++)
		printf("%02x:", tarbuffer[i]);
	//return wrote == size? 0 : 1;
	return 0;
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
	uint8_t macroid[] = {0x1a,0x2d,0x00};
	global_enb_id->eNB_ID.present = S1ap_ENB_ID_PR_macroENB_ID;
	global_enb_id->eNB_ID.choice.macroENB_ID.buf = macroid;
	global_enb_id->eNB_ID.choice.macroENB_ID.size = 3;
	global_enb_id->eNB_ID.choice.macroENB_ID.bits_unused = 4;
	uint8_t plmn[] = {0x00,0xf1,0x10};
	OCTET_STRING_fromBuf(&(global_enb_id->pLMNidentity), plmn, 3);
	ANY_fromType_per(&(item0->value), &asn_DEF_S1ap_Global_ENB_ID, global_enb_id);
	free(global_enb_id);
	return item0;	
}

S1ap_IE_t* getItem1() {
	S1ap_IE_t *item;
	asn_enc_rval_t ec;
	item = calloc(1, sizeof(S1ap_IE_t));
	assert(item);
	item->id = S1ap_ProtocolIE_ID_id_eNBname;
	item->criticality = S1ap_Criticality_ignore;
	S1ap_ENBname_t* name;
	name = calloc(1, sizeof(S1ap_ENBname_t));
	uint8_t namebuf[] = {0x65,0x6e,0x62,0x31,0x61,0x32,0x64,0x30}; // enb1a2d0
	//uint8_t namebuf[] = {'e','n','b','1','a','2','d','0'};
	OCTET_STRING_fromBuf(name, namebuf, 8);
	ANY_fromType_per(&(item->value), &asn_DEF_S1ap_ENBname, name);
	//ANY_fromType_per(&(item->value), &asn_DEF_S1ap_ENBname, &name);
	free(name);
	return item;
}
S1ap_IE_t* getItem2() {
	int result;
	S1ap_IE_t *item;
	asn_enc_rval_t ec;
	item = calloc(1, sizeof(S1ap_IE_t));
	assert(item);
	item->id = S1ap_ProtocolIE_ID_id_SupportedTAs;
	item->criticality = S1ap_Criticality_reject;
	///////////
	uint8_t plmn[] = {0x00,0xf1,0x10};
	S1ap_PLMNidentity_t* pLMNidentity;
	pLMNidentity = calloc(1,sizeof(S1ap_PLMNidentity_t));
	OCTET_STRING_fromBuf(pLMNidentity, plmn, 3);
	S1ap_SupportedTAs_Item_t* supported_Item;
	supported_Item = calloc(1,sizeof(S1ap_SupportedTAs_Item_t));
	uint8_t tacbuf[] = {0x01};
	OCTET_STRING_fromBuf(&(supported_Item->tAC), tacbuf,1);
	result = asn_set_add(&(supported_Item->broadcastPLMNs), pLMNidentity);
	printf("bPLMNs add result is %d", result);
	S1ap_SupportedTAs_t* supported_TAs;
	supported_TAs = calloc(1, sizeof(S1ap_SupportedTAs_t));
	result = asn_set_add(&(supported_TAs->list), supported_Item);
	printf("list add result is %d", result);
	ANY_fromType_per(&(item->value), &asn_DEF_S1ap_SupportedTAs, supported_TAs);
	xer_fprint(stdout, &asn_DEF_S1ap_SupportedTAs, supported_TAs);
	xer_fprint(stdout, &asn_DEF_S1ap_SupportedTAs_Item, supported_Item);
	uper_encode(&asn_DEF_S1ap_SupportedTAs, 0, supported_TAs, write_out, 0);
/*	free(supported_TAs);
	free(pLMNidentity);
	free(supported_Item);
	
*/	return item;
}
int main() {
	S1ap_IE_t* item = getItem2();
	asn_enc_rval_t ec;
	FILE *fp = fopen("test.txt","wb");
	//ec = der_encode(&asn_DEF_S1ap_IE, item, write_out, fp);
	ec = uper_encode(&asn_DEF_S1ap_IE, 0, item, write_out, fp);
	fclose(fp);
	printf("\n%d\n", (int)ec.encoded);
	xer_fprint(stdout, &asn_DEF_S1ap_IE, item);
	return 0;
}
