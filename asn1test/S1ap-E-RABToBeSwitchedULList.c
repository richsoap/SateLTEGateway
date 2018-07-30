/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -no-gen-OER -no-gen-example -pdu=all`
 */

#include "S1ap-E-RABToBeSwitchedULList.h"

static asn_per_constraints_t asn_PER_type_S1ap_E_RABToBeSwitchedULList_constr_1 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 8,  8,  1,  256 }	/* (SIZE(1..256)) */,
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_S1ap_E_RABToBeSwitchedULList_1[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_S1ap_IE,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_S1ap_E_RABToBeSwitchedULList_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_S1ap_E_RABToBeSwitchedULList_specs_1 = {
	sizeof(struct S1ap_E_RABToBeSwitchedULList),
	offsetof(struct S1ap_E_RABToBeSwitchedULList, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
asn_TYPE_descriptor_t asn_DEF_S1ap_E_RABToBeSwitchedULList = {
	"S1ap-E-RABToBeSwitchedULList",
	"S1ap-E-RABToBeSwitchedULList",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_S1ap_E_RABToBeSwitchedULList_tags_1,
	sizeof(asn_DEF_S1ap_E_RABToBeSwitchedULList_tags_1)
		/sizeof(asn_DEF_S1ap_E_RABToBeSwitchedULList_tags_1[0]), /* 1 */
	asn_DEF_S1ap_E_RABToBeSwitchedULList_tags_1,	/* Same as above */
	sizeof(asn_DEF_S1ap_E_RABToBeSwitchedULList_tags_1)
		/sizeof(asn_DEF_S1ap_E_RABToBeSwitchedULList_tags_1[0]), /* 1 */
	{ 0, &asn_PER_type_S1ap_E_RABToBeSwitchedULList_constr_1, SEQUENCE_OF_constraint },
	asn_MBR_S1ap_E_RABToBeSwitchedULList_1,
	1,	/* Single element */
	&asn_SPC_S1ap_E_RABToBeSwitchedULList_specs_1	/* Additional specs */
};

