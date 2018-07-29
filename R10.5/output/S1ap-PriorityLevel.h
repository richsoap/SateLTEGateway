/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=all`
 */

#ifndef	_S1ap_PriorityLevel_H_
#define	_S1ap_PriorityLevel_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum S1ap_PriorityLevel {
	S1ap_PriorityLevel_spare	= 0,
	S1ap_PriorityLevel_highest	= 1,
	S1ap_PriorityLevel_lowest	= 14,
	S1ap_PriorityLevel_no_priority	= 15
} e_S1ap_PriorityLevel;

/* S1ap-PriorityLevel */
typedef long	 S1ap_PriorityLevel_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_S1ap_PriorityLevel_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_S1ap_PriorityLevel;
asn_struct_free_f S1ap_PriorityLevel_free;
asn_struct_print_f S1ap_PriorityLevel_print;
asn_constr_check_f S1ap_PriorityLevel_constraint;
ber_type_decoder_f S1ap_PriorityLevel_decode_ber;
der_type_encoder_f S1ap_PriorityLevel_encode_der;
xer_type_decoder_f S1ap_PriorityLevel_decode_xer;
xer_type_encoder_f S1ap_PriorityLevel_encode_xer;
oer_type_decoder_f S1ap_PriorityLevel_decode_oer;
oer_type_encoder_f S1ap_PriorityLevel_encode_oer;
per_type_decoder_f S1ap_PriorityLevel_decode_uper;
per_type_encoder_f S1ap_PriorityLevel_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _S1ap_PriorityLevel_H_ */
#include <asn_internal.h>
