/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=all`
 */

#ifndef	_S1ap_Bearers_SubjectToStatusTransfer_Item_H_
#define	_S1ap_Bearers_SubjectToStatusTransfer_Item_H_


#include <asn_application.h>

/* Including external dependencies */
#include "S1ap-E-RAB-ID.h"
#include "S1ap-COUNTvalue.h"
#include "S1ap-ReceiveStatusofULPDCPSDUs.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct S1ap_IE_Extensions;

/* S1ap-Bearers-SubjectToStatusTransfer-Item */
typedef struct S1ap_Bearers_SubjectToStatusTransfer_Item {
	S1ap_E_RAB_ID_t	 e_RAB_ID;
	S1ap_COUNTvalue_t	 uL_COUNTvalue;
	S1ap_COUNTvalue_t	 dL_COUNTvalue;
	S1ap_ReceiveStatusofULPDCPSDUs_t	*receiveStatusofULPDCPSDUs	/* OPTIONAL */;
	struct S1ap_IE_Extensions	*iE_Extensions	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} S1ap_Bearers_SubjectToStatusTransfer_Item_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_S1ap_Bearers_SubjectToStatusTransfer_Item;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "S1ap-IE-Extensions.h"

#endif	/* _S1ap_Bearers_SubjectToStatusTransfer_Item_H_ */
#include <asn_internal.h>
