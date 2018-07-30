/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -no-gen-OER -no-gen-example -pdu=all`
 */

#include "S1ap-WarningAreaList.h"

static asn_per_constraints_t asn_PER_type_S1ap_WarningAreaList_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  2,  2,  0,  2 }	/* (0..2,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_S1ap_WarningAreaList_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct S1ap_WarningAreaList, choice.cellIDList),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_ECGIList,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"cellIDList"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct S1ap_WarningAreaList, choice.trackingAreaListforWarning),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_TAIListforWarning,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"trackingAreaListforWarning"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct S1ap_WarningAreaList, choice.emergencyAreaIDList),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_EmergencyAreaIDList,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"emergencyAreaIDList"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_S1ap_WarningAreaList_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* cellIDList */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* trackingAreaListforWarning */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* emergencyAreaIDList */
};
static asn_CHOICE_specifics_t asn_SPC_S1ap_WarningAreaList_specs_1 = {
	sizeof(struct S1ap_WarningAreaList),
	offsetof(struct S1ap_WarningAreaList, _asn_ctx),
	offsetof(struct S1ap_WarningAreaList, present),
	sizeof(((struct S1ap_WarningAreaList *)0)->present),
	asn_MAP_S1ap_WarningAreaList_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0,
	3	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_S1ap_WarningAreaList = {
	"S1ap-WarningAreaList",
	"S1ap-WarningAreaList",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ 0, &asn_PER_type_S1ap_WarningAreaList_constr_1, CHOICE_constraint },
	asn_MBR_S1ap_WarningAreaList_1,
	3,	/* Elements count */
	&asn_SPC_S1ap_WarningAreaList_specs_1	/* Additional specs */
};

