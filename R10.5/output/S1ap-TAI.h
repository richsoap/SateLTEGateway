/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=auto`
 */

#ifndef	_S1ap_TAI_H_
#define	_S1ap_TAI_H_


#include <asn_application.h>

/* Including external dependencies */
#include "S1ap-PLMNidentity.h"
#include "S1ap-TAC.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct S1ap_IE_Extensions;

/* S1ap-TAI */
typedef struct S1ap_TAI {
	S1ap_PLMNidentity_t	 pLMNidentity;
	S1ap_TAC_t	 tAC;
	struct S1ap_IE_Extensions	*iE_Extensions	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} S1ap_TAI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_S1ap_TAI;
extern asn_SEQUENCE_specifics_t asn_SPC_S1ap_TAI_specs_1;
extern asn_TYPE_member_t asn_MBR_S1ap_TAI_1[3];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "S1ap-IE-Extensions.h"

#endif	/* _S1ap_TAI_H_ */
#include <asn_internal.h>
