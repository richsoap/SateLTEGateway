/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=all`
 */

#ifndef	_S1ap_Source_ToTarget_TransparentContainer_H_
#define	_S1ap_Source_ToTarget_TransparentContainer_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* S1ap-Source-ToTarget-TransparentContainer */
typedef OCTET_STRING_t	 S1ap_Source_ToTarget_TransparentContainer_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_S1ap_Source_ToTarget_TransparentContainer;
asn_struct_free_f S1ap_Source_ToTarget_TransparentContainer_free;
asn_struct_print_f S1ap_Source_ToTarget_TransparentContainer_print;
asn_constr_check_f S1ap_Source_ToTarget_TransparentContainer_constraint;
ber_type_decoder_f S1ap_Source_ToTarget_TransparentContainer_decode_ber;
der_type_encoder_f S1ap_Source_ToTarget_TransparentContainer_encode_der;
xer_type_decoder_f S1ap_Source_ToTarget_TransparentContainer_decode_xer;
xer_type_encoder_f S1ap_Source_ToTarget_TransparentContainer_encode_xer;
oer_type_decoder_f S1ap_Source_ToTarget_TransparentContainer_decode_oer;
oer_type_encoder_f S1ap_Source_ToTarget_TransparentContainer_encode_oer;
per_type_decoder_f S1ap_Source_ToTarget_TransparentContainer_decode_uper;
per_type_encoder_f S1ap_Source_ToTarget_TransparentContainer_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _S1ap_Source_ToTarget_TransparentContainer_H_ */
#include <asn_internal.h>
