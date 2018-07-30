/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -no-gen-OER -no-gen-example -pdu=all`
 */

#ifndef	_S1ap_CancelledCellinTAI_H_
#define	_S1ap_CancelledCellinTAI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct S1ap_CancelledCellinTAI_Item;

/* S1ap-CancelledCellinTAI */
typedef struct S1ap_CancelledCellinTAI {
	A_SEQUENCE_OF(struct S1ap_CancelledCellinTAI_Item) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} S1ap_CancelledCellinTAI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_S1ap_CancelledCellinTAI;
extern asn_SET_OF_specifics_t asn_SPC_S1ap_CancelledCellinTAI_specs_1;
extern asn_TYPE_member_t asn_MBR_S1ap_CancelledCellinTAI_1[1];
extern asn_per_constraints_t asn_PER_type_S1ap_CancelledCellinTAI_constr_1;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "S1ap-CancelledCellinTAI-Item.h"

#endif	/* _S1ap_CancelledCellinTAI_H_ */
#include <asn_internal.h>
