/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=all`
 */

#ifndef	_S1ap_PagingPriority_H_
#define	_S1ap_PagingPriority_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum S1ap_PagingPriority {
	S1ap_PagingPriority_priolevel1	= 0,
	S1ap_PagingPriority_priolevel2	= 1,
	S1ap_PagingPriority_priolevel3	= 2,
	S1ap_PagingPriority_priolevel4	= 3,
	S1ap_PagingPriority_priolevel5	= 4,
	S1ap_PagingPriority_priolevel6	= 5,
	S1ap_PagingPriority_priolevel7	= 6,
	S1ap_PagingPriority_priolevel8	= 7
	/*
	 * Enumeration is extensible
	 */
} e_S1ap_PagingPriority;

/* S1ap-PagingPriority */
typedef long	 S1ap_PagingPriority_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_S1ap_PagingPriority;
asn_struct_free_f S1ap_PagingPriority_free;
asn_struct_print_f S1ap_PagingPriority_print;
asn_constr_check_f S1ap_PagingPriority_constraint;
ber_type_decoder_f S1ap_PagingPriority_decode_ber;
der_type_encoder_f S1ap_PagingPriority_encode_der;
xer_type_decoder_f S1ap_PagingPriority_decode_xer;
xer_type_encoder_f S1ap_PagingPriority_encode_xer;
oer_type_decoder_f S1ap_PagingPriority_decode_oer;
oer_type_encoder_f S1ap_PagingPriority_encode_oer;
per_type_decoder_f S1ap_PagingPriority_decode_uper;
per_type_encoder_f S1ap_PagingPriority_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _S1ap_PagingPriority_H_ */
#include <asn_internal.h>
