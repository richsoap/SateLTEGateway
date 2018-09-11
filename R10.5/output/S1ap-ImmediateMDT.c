/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "S1AP-IEs.asn"
 * 	`asn1c -D ./output -pdu=auto`
 */

#include "S1ap-ImmediateMDT.h"

asn_TYPE_member_t asn_MBR_S1ap_ImmediateMDT_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct S1ap_ImmediateMDT, measurementsToActivate),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_MeasurementsToActivate,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"measurementsToActivate"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct S1ap_ImmediateMDT, reportingTriggerMDT),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_ReportingTriggerMDT,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"reportingTriggerMDT"
		},
	{ ATF_POINTER, 3, offsetof(struct S1ap_ImmediateMDT, thresholdeventA2),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_ThresholdEventA2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"thresholdeventA2"
		},
	{ ATF_POINTER, 2, offsetof(struct S1ap_ImmediateMDT, periodicReportingMDT),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_PeriodicReportingMDT,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"periodicReportingMDT"
		},
	{ ATF_POINTER, 1, offsetof(struct S1ap_ImmediateMDT, iE_Extensions),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1ap_IE_Extensions,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"iE-Extensions"
		},
};
static const int asn_MAP_S1ap_ImmediateMDT_oms_1[] = { 2, 3, 4 };
static const ber_tlv_tag_t asn_DEF_S1ap_ImmediateMDT_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_S1ap_ImmediateMDT_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* measurementsToActivate */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* reportingTriggerMDT */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* thresholdeventA2 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* periodicReportingMDT */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* iE-Extensions */
};
asn_SEQUENCE_specifics_t asn_SPC_S1ap_ImmediateMDT_specs_1 = {
	sizeof(struct S1ap_ImmediateMDT),
	offsetof(struct S1ap_ImmediateMDT, _asn_ctx),
	asn_MAP_S1ap_ImmediateMDT_tag2el_1,
	5,	/* Count of tags in the map */
	asn_MAP_S1ap_ImmediateMDT_oms_1,	/* Optional members */
	3, 0,	/* Root/Additions */
	5,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_S1ap_ImmediateMDT = {
	"S1ap-ImmediateMDT",
	"S1ap-ImmediateMDT",
	&asn_OP_SEQUENCE,
	asn_DEF_S1ap_ImmediateMDT_tags_1,
	sizeof(asn_DEF_S1ap_ImmediateMDT_tags_1)
		/sizeof(asn_DEF_S1ap_ImmediateMDT_tags_1[0]), /* 1 */
	asn_DEF_S1ap_ImmediateMDT_tags_1,	/* Same as above */
	sizeof(asn_DEF_S1ap_ImmediateMDT_tags_1)
		/sizeof(asn_DEF_S1ap_ImmediateMDT_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_S1ap_ImmediateMDT_1,
	5,	/* Elements count */
	&asn_SPC_S1ap_ImmediateMDT_specs_1	/* Additional specs */
};

