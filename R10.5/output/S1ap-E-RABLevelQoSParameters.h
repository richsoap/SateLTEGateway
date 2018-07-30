/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=all`
 */

#ifndef	_S1ap_E_RABLevelQoSParameters_H_
#define	_S1ap_E_RABLevelQoSParameters_H_


#include <asn_application.h>

/* Including external dependencies */
#include "S1ap-QCI.h"
#include "S1ap-AllocationAndRetentionPriority.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct S1ap_GBR_QosInformation;
struct S1ap_IE_Extensions;

/* S1ap-E-RABLevelQoSParameters */
typedef struct S1ap_E_RABLevelQoSParameters {
	S1ap_QCI_t	 qCI;
	S1ap_AllocationAndRetentionPriority_t	 allocationRetentionPriority;
	struct S1ap_GBR_QosInformation	*gbrQosInformation	/* OPTIONAL */;
	struct S1ap_IE_Extensions	*iE_Extensions	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} S1ap_E_RABLevelQoSParameters_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_S1ap_E_RABLevelQoSParameters;
extern asn_SEQUENCE_specifics_t asn_SPC_S1ap_E_RABLevelQoSParameters_specs_1;
extern asn_TYPE_member_t asn_MBR_S1ap_E_RABLevelQoSParameters_1[4];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "S1ap-GBR-QosInformation.h"
#include "S1ap-IE-Extensions.h"

#endif	/* _S1ap_E_RABLevelQoSParameters_H_ */
#include <asn_internal.h>