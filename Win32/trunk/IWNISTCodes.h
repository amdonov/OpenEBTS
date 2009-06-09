//////////////////////////////////////////////////////////////////////
//
// IWNISTCodes.h
//
//////////////////////////////////////////////////////////////////////

#include "iwnist.h"

typedef struct IW_NIST_FIELD
{
	int nRecordType;
	int nFieldID;
	int nParentField;
	char* szNISTCode;
	char* szDescription;
}IWNISTFIELD, *PIWNISTFIELD;

extern const IWNISTFIELD g_NIST_Fields[];
extern const int g_NIST_Fields_Count;

// IWS NIST Importer version
typedef enum 
{
	Jail = 0,		// 0
	OfficerID,
	OfficerName,
	timeStamp,
	Beat,
	Notes,			// 5
	BookingNumber,
	ArrestDate,
	ArrestTime,
	AgencyID,
	Sealed,
	MasterKey,				// 10
	RecordType,
	
	ADRNUM,
	ADRDIR,			// 14
	ADRSTNAME,
	ADRSTTYPE,
	ADRSUITE,
//	ADREFFDATE,
//	ADRTERDATE,
	ADRPO,
	ADRCT,			// 24
	ADRST,
	ADRZIP,
	ADRZIP4,			// 19
	ADRBN,
	ADRC1,
	ADRGD,
	AddressType,
	ADRSH,			// 29
	ADRDT,
	ADRDU,
	ADRLF,
	ADRAN,
	ADRDI,			// 34
	ADRCN,
	
	ArrestAddress,
	ARADRDIR,	
	ARADRSTNAME,
	ARADRSTTYPE,
	ARADRSUITE,
	ARADRPO,
	ARADRCT,	
	ARADRST,	
	ARADRZIP,	
	ARADRZIP4,
	ARADRBN,

	WorkAddress,
	EMPADRDIR,	
	EMPADRSTNAME,
	EMPADRSTTYPE,
	EMPADRSUITE,
	EMPADRPO,	
	EMPADRCT,		
	EMPADRST,
	EMPADRZIP,	
	EMPADRZIP4,	
	EMPADRBN,

	RelativeAddress,
	FirstName,		// 38
	LastName,
	Middle,
	LastName_dbl,	// 41
	FirstName_dbl,
	Middle_dbl,
	Suffix,
	Title,			
	DOB,				// 46
	POB_City,
	POB_State,
	POB_County,
	POB_Country,	
	SSN1,				// 51
	SSN2,
	SSN3,
	Age,
	Height,			
	DL1_State,		// 56
	DL1_Class,
	DL1_Number,
	DL1_Expiration,
	DL2_State,		
	DL2_Class,		// 61
	DL2_Number,
	DL2_Expiration,
	Race,
	EyeColor,		
	Glasses,			// 66
	FID,
	State,
	StateID,
	Weight,			
	HairColor,		// 71
	Sex,
	HairLength,
	HairType,
	FacialHair,		// 75
	Alien,
	NameRefused,
	Alias,			// 78
	
	SMT_ID,			// 79
	NCIC,
	smtNotes,		// "Notes" was used already
	ImageNum,
	
	Thumbnail,		// 84
	Image,
	ImageNumber,
	
	UserField1,		// 87
	UserField2,
	UserField3,
	UserField4,

	UserField5,		// 91
	UserField6,
	UserField7,
	UserField8,

	UserField9,		// 95
	UserField10,
	UserField11,
	UserField12,
	
	Charge,			// 99
	Classification,

// XImage Extensions
	
	Station1,
	Station10,		

	// Booking table
	booking_key,	
	case_ref,		
	bk_dob,			
	bk_created,		
	rec_kind,		
	bk_l_name,		
	bk_f_name,		
	bk_m_name,		
	bk_suffix,		
	bk_l_name_dbl,
	bk_f_name_dbl,
	bk_m_name_dbl,
	bk_cli_int1,	
	bk_cli_int2,	
	bk_cli_code1,	
	bk_cli_code2,	
	bk_cli_code3,	
	bk_cli_code4,	
	bk_cli_code5,	
	bk_cli_code6,	
	bk_cli_char1,	
	bk_cli_char2,	
	bk_cli_char3,	
	bk_cli_char4,
	bk_create_user,
	bk_note_count,
	bk_note_text,
	Agency,
	Station,
	Site,
	Division,

	// Folder
	fl_masterkey,
	fl_cli_int1,	
	fl_cli_short1,
	fl_cli_short2,
	fl_cli_code1,
	fl_cli_code2,
	fl_cli_code3,
	fl_cli_code4,
	fl_cli_code5,
	fl_cli_code6,
	fl_cli_char1,
	fl_cli_char2,
	fl_cli_char3,
	fl_cli_char4,
	fl_create_user,
	fl_note_count,
	fl_note_text,

	// Person
	pr_create_user,
	pr_l_name,	
	pr_f_name,	
	pr_m_name,	
	pr_suffix,	
	pr_l_name_dbl,
	pr_f_name_dbl,
	pr_m_name_dbl,
	pr_dob,		
	pr_SSN,
	pr_DL_State,
	pr_DL_Number,

	// Alias
	al_count,
	al_l_name,		
	al_f_name,		
	al_m_name,		
	al_suffix,	
	al_dob,
	moniker,
			
	// Charge
	chg_count,
	counts,		
	code,			
	statute,		
	chg_class,			
	degree,		
	category,		
	description,
	ncic,
	chargeStr,

	// ForceField flag
	isForceField,

	// Appearance
	ap_count,
	ap_sex,			
	ap_age,			
	ap_race,			
	skin_tone,	
	ap_height,		
	ap_weight,		
	ap_build,			
	ap_hair_color,	
	ap_hair_style,	
	ap_hair_length,	
	ap_eye_color,	
	ap_cor_vision,
	ap_facial_hair,
	ap_facial_hair_desc,
	ap_Complexion,	
	ap_cli_code1,	
	ap_cli_code2,	
	ap_cli_code3,	
	ap_cli_code4,	
	ap_cli_code5,	
	ap_cli_code6,	
	ap_id,	
	ap_note_count,
	ap_note_text,
	ap_Beard,
	ap_Mustache,
	ap_SideBurns,
	SkinColor,
	ap_user_field1,
	ap_user_field2,
	ap_user_field3,
	ap_user_field4,
	ap_user_field5,
	ap_user_field6,

	// SMT
	smt_count,
	body_side,
	body_part,
	smt_desc,
	text_desc,
	smt_note_text,

	// Image
	img_count,
	img_type,			
	img_created,	
	table_ref,	
	img_class,	
	img_embedded,
	img_status,
	img_filename,
	img_id,
	img_smt_ncic,
	img_smt_desc,	
	JPEG,		
	
	FIELD_LAST   // for use in array sizing
} fieldname_et;

typedef struct
{
	int whichrecord;	// which record structure contains the data:
										// 0:none - no data for this target
										// 1:Type 1
										// 2:Type 2
										// 3:filled by us

	int field;        // what field contains the data (1-999)

	int subfield;     // what subfield (1-999)
	
	int itemno;			// which item in subfield (1-999)

	char transto;		// what do we translate the string to for the target
							// 'C':char, 'I':int, 'D':datetime, 'd': date
	
	char fieldname[30];	// target field name
	
	char *pdata;	// a pointer into the record structure where the actual data is
} nistrecord_st;

extern nistrecord_st nistTable[FIELD_LAST];
