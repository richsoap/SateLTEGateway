/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=all`
 */

#ifndef	_S1ap_ENBX2GTPTLAs_H_
#define	_S1ap_ENBX2GTPTLAs_H_


#include <asn_application.h>

/* Including external dependencies */
#include "S1ap-TransportLayerAddress.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* S1ap-ENBX2GTPTLAs */
typedef struct S1ap_ENBX2GTPTLAs {
	A_SEQUENCE_OF(S1ap_TransportLayerAddress_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} S1ap_ENBX2GTPTLAs_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_S1ap_ENBX2GTPTLAs;
extern asn_SET_OF_specifics_t asn_SPC_S1ap_ENBX2GTPTLAs_specs_1;
extern asn_TYPE_member_t asn_MBR_S1ap_ENBX2GTPTLAs_1[1];
extern asn_per_constraints_t asn_PER_type_S1ap_ENBX2GTPTLAs_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _S1ap_ENBX2GTPTLAs_H_ */
#include <asn_internal.h>
