/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -no-gen-OER -no-gen-example -pdu=all`
 */

#ifndef	_S1ap_EmergencyAreaID_H_
#define	_S1ap_EmergencyAreaID_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* S1ap-EmergencyAreaID */
typedef OCTET_STRING_t	 S1ap_EmergencyAreaID_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_S1ap_EmergencyAreaID_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_S1ap_EmergencyAreaID;
asn_struct_free_f S1ap_EmergencyAreaID_free;
asn_struct_print_f S1ap_EmergencyAreaID_print;
asn_constr_check_f S1ap_EmergencyAreaID_constraint;
ber_type_decoder_f S1ap_EmergencyAreaID_decode_ber;
der_type_encoder_f S1ap_EmergencyAreaID_encode_der;
xer_type_decoder_f S1ap_EmergencyAreaID_decode_xer;
xer_type_encoder_f S1ap_EmergencyAreaID_encode_xer;
per_type_decoder_f S1ap_EmergencyAreaID_decode_uper;
per_type_encoder_f S1ap_EmergencyAreaID_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _S1ap_EmergencyAreaID_H_ */
#include <asn_internal.h>
