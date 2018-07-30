/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -no-gen-OER -no-gen-example -pdu=all`
 */

#include "S1ap-CSFallbackIndicator.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static asn_per_constraints_t asn_PER_type_S1ap_CSFallbackIndicator_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  0,  0,  0,  0 }	/* (0..0,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_S1ap_CSFallbackIndicator_value2enum_1[] = {
	{ 0,	20,	"cs-fallback-required" },
	{ 1,	25,	"cs-fallback-high-priority" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_S1ap_CSFallbackIndicator_enum2value_1[] = {
	1,	/* cs-fallback-high-priority(1) */
	0	/* cs-fallback-required(0) */
	/* This list is extensible */
};
static const asn_INTEGER_specifics_t asn_SPC_S1ap_CSFallbackIndicator_specs_1 = {
	asn_MAP_S1ap_CSFallbackIndicator_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_S1ap_CSFallbackIndicator_enum2value_1,	/* N => "tag"; sorted by N */
	2,	/* Number of elements in the maps */
	2,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_S1ap_CSFallbackIndicator_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_S1ap_CSFallbackIndicator = {
	"S1ap-CSFallbackIndicator",
	"S1ap-CSFallbackIndicator",
	&asn_OP_NativeEnumerated,
	asn_DEF_S1ap_CSFallbackIndicator_tags_1,
	sizeof(asn_DEF_S1ap_CSFallbackIndicator_tags_1)
		/sizeof(asn_DEF_S1ap_CSFallbackIndicator_tags_1[0]), /* 1 */
	asn_DEF_S1ap_CSFallbackIndicator_tags_1,	/* Same as above */
	sizeof(asn_DEF_S1ap_CSFallbackIndicator_tags_1)
		/sizeof(asn_DEF_S1ap_CSFallbackIndicator_tags_1[0]), /* 1 */
	{ 0, &asn_PER_type_S1ap_CSFallbackIndicator_constr_1, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_S1ap_CSFallbackIndicator_specs_1	/* Additional specs */
};

