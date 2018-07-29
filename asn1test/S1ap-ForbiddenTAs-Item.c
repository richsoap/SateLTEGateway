/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=all`
 */

#include "S1ap-ForbiddenTAs-Item.h"

asn_TYPE_member_t asn_MBR_S1ap_ForbiddenTAs_Item_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct S1ap_ForbiddenTAs_Item, pLMN_Identity),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_PLMNidentity,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"pLMN-Identity"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct S1ap_ForbiddenTAs_Item, forbiddenTACs),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_ForbiddenTACs,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"forbiddenTACs"
		},
	{ ATF_POINTER, 1, offsetof(struct S1ap_ForbiddenTAs_Item, iE_Extensions),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_IE_Extensions,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"iE-Extensions"
		},
};
static const int asn_MAP_S1ap_ForbiddenTAs_Item_oms_1[] = { 2 };
static const ber_tlv_tag_t asn_DEF_S1ap_ForbiddenTAs_Item_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_S1ap_ForbiddenTAs_Item_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* pLMN-Identity */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* forbiddenTACs */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* iE-Extensions */
};
asn_SEQUENCE_specifics_t asn_SPC_S1ap_ForbiddenTAs_Item_specs_1 = {
	sizeof(struct S1ap_ForbiddenTAs_Item),
	offsetof(struct S1ap_ForbiddenTAs_Item, _asn_ctx),
	asn_MAP_S1ap_ForbiddenTAs_Item_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_S1ap_ForbiddenTAs_Item_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_S1ap_ForbiddenTAs_Item = {
	"S1ap-ForbiddenTAs-Item",
	"S1ap-ForbiddenTAs-Item",
	&asn_OP_SEQUENCE,
	asn_DEF_S1ap_ForbiddenTAs_Item_tags_1,
	sizeof(asn_DEF_S1ap_ForbiddenTAs_Item_tags_1)
		/sizeof(asn_DEF_S1ap_ForbiddenTAs_Item_tags_1[0]), /* 1 */
	asn_DEF_S1ap_ForbiddenTAs_Item_tags_1,	/* Same as above */
	sizeof(asn_DEF_S1ap_ForbiddenTAs_Item_tags_1)
		/sizeof(asn_DEF_S1ap_ForbiddenTAs_Item_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_S1ap_ForbiddenTAs_Item_1,
	3,	/* Elements count */
	&asn_SPC_S1ap_ForbiddenTAs_Item_specs_1	/* Additional specs */
};

