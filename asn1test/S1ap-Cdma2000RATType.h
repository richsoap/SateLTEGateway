/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -no-gen-OER -no-gen-example -pdu=all`
 */

#ifndef	_S1ap_Cdma2000RATType_H_
#define	_S1ap_Cdma2000RATType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum S1ap_Cdma2000RATType {
	S1ap_Cdma2000RATType_hRPD	= 0,
	S1ap_Cdma2000RATType_onexRTT	= 1
	/*
	 * Enumeration is extensible
	 */
} e_S1ap_Cdma2000RATType;

/* S1ap-Cdma2000RATType */
typedef long	 S1ap_Cdma2000RATType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_S1ap_Cdma2000RATType;
asn_struct_free_f S1ap_Cdma2000RATType_free;
asn_struct_print_f S1ap_Cdma2000RATType_print;
asn_constr_check_f S1ap_Cdma2000RATType_constraint;
ber_type_decoder_f S1ap_Cdma2000RATType_decode_ber;
der_type_encoder_f S1ap_Cdma2000RATType_encode_der;
xer_type_decoder_f S1ap_Cdma2000RATType_decode_xer;
xer_type_encoder_f S1ap_Cdma2000RATType_encode_xer;
per_type_decoder_f S1ap_Cdma2000RATType_decode_uper;
per_type_encoder_f S1ap_Cdma2000RATType_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _S1ap_Cdma2000RATType_H_ */
#include <asn_internal.h>
