/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=all`
 */

#include "S1ap-OverloadResponse.h"

static asn_oer_constraints_t asn_OER_type_S1ap_OverloadResponse_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_S1ap_OverloadResponse_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  0,  0,  0,  0 }	/* (0..0,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_S1ap_OverloadResponse_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct S1ap_OverloadResponse, choice.overloadAction),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_OverloadAction,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"overloadAction"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_S1ap_OverloadResponse_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* overloadAction */
};
static asn_CHOICE_specifics_t asn_SPC_S1ap_OverloadResponse_specs_1 = {
	sizeof(struct S1ap_OverloadResponse),
	offsetof(struct S1ap_OverloadResponse, _asn_ctx),
	offsetof(struct S1ap_OverloadResponse, present),
	sizeof(((struct S1ap_OverloadResponse *)0)->present),
	asn_MAP_S1ap_OverloadResponse_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0,
	1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_S1ap_OverloadResponse = {
	"S1ap-OverloadResponse",
	"S1ap-OverloadResponse",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ &asn_OER_type_S1ap_OverloadResponse_constr_1, &asn_PER_type_S1ap_OverloadResponse_constr_1, CHOICE_constraint },
	asn_MBR_S1ap_OverloadResponse_1,
	1,	/* Elements count */
	&asn_SPC_S1ap_OverloadResponse_specs_1	/* Additional specs */
};

