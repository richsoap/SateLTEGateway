/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=all`
 */

#ifndef	_S1ap_CellIdentity_H_
#define	_S1ap_CellIdentity_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* S1ap-CellIdentity */
typedef BIT_STRING_t	 S1ap_CellIdentity_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_S1ap_CellIdentity_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_S1ap_CellIdentity;
asn_struct_free_f S1ap_CellIdentity_free;
asn_struct_print_f S1ap_CellIdentity_print;
asn_constr_check_f S1ap_CellIdentity_constraint;
ber_type_decoder_f S1ap_CellIdentity_decode_ber;
der_type_encoder_f S1ap_CellIdentity_encode_der;
xer_type_decoder_f S1ap_CellIdentity_decode_xer;
xer_type_encoder_f S1ap_CellIdentity_encode_xer;
oer_type_decoder_f S1ap_CellIdentity_decode_oer;
oer_type_encoder_f S1ap_CellIdentity_encode_oer;
per_type_decoder_f S1ap_CellIdentity_decode_uper;
per_type_encoder_f S1ap_CellIdentity_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _S1ap_CellIdentity_H_ */
#include <asn_internal.h>
