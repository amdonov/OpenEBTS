// IWNISTCodes.cpp: implementation of the CNistRecord class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "IWNISTCodes.h"

nistrecord_st nistTable[FIELD_LAST] = 
{	
	//							IWS Column					Comments
	//							----------					-----------------------
	// Table: "booking"
	1,	8,	1,	1,	'C',		"Jail",				NULL,	// ORI
	0,	0,	0,	0,	'C',		"OfficerID",		NULL,
	0,	0,	0,	0,	'C',		"OfficerName",		NULL,
	3,	0,	0,	0,	'D',		"TimeStamp",		NULL,	// current time
	0,	0,	0,	0,	'C',		"Beat",				NULL,
	0,	0,	0,	0,	'C',		"Notes",			NULL,	 
	1,	9,  1,	1,	'C',		"BookingNumber",	NULL,   // Transaction Control Number
	2,	45,	1,	1,	'd',		"ArrestDate",		NULL,	// DOA
	0,	0,	0,	0,	'C',		"ArrestTime",		NULL,	 
	0,	0,	0,	0,	'C',		"AgencyID",			NULL,	//
	2,	450, 1,	1, 	'C',		"Sealed",			NULL, // Sealed record
	2,	300, 1,	1,	'C',		"MasterKey",		NULL,
	2,	43,	1,	1,	'C',		"RecordType",		"01", //$$$  This follows the AZ spreadsheet, unknown if different in value from B.Txt

	// Table: "Address"
	2,	41,	1,	1,	'C',		"ADRNUM",			NULL,	// RESIDENTIAL only
	2,	528,1,	1,	'C',		"ADRDIR",			NULL,	// data comes in on ADRNUM and we parse out the rest
	2,	529,1,	1,	'C',		"ADRSTNAME",		NULL,
	2,	530,1,	1,	'C',		"ADRSTTYPE",		NULL,
	2,	531,1,	1,	'C',		"ADRSUITE",			NULL,
//	2,	534,1,	1,	'd',		"ADREFFDATE",		NULL,
//	2,	535,1,	1,	'd',		"ADRTERDATE",		NULL,
	2,	536,1,	1,	'C',		"ADRPO",				NULL,	// ignore
	2,	538,1,	1,	'C',		"ADRCT",				NULL,
	2,	539,1,	1,	'C',		"ADRST",				NULL,
	2,	532,1,	1,	'C',		"ADRZIP",			NULL,
	2,	533,1,	1,	'C',		"ADRZIP4",			NULL,	 
	2,	537,1,	1,	'C',		"ADRBN",				NULL,   // business name
	2,	540,1,	1,	'C',		"ADRC1",				NULL,
	2,	541,1,	1,	'C',		"ADRGD",				NULL,
	2,	542,1,	1,	'C',		"AddressType",			NULL,  
	2,	543,1,	1,	'C',		"ADRSH",				NULL,
	2,	544,1,	1,	'I',		"ADRDT",				NULL,
	2,	545,1,	1,	'C',		"ADRDU",				NULL,
	2,	546,1,	1,	'C',		"ADRLF",				NULL,
	2,	547,1,	1,	'C',		"ADRAN",				NULL,
	2,	548,1,	1,	'C',		"ADRDI",				NULL,
	2,	549,1,	1,	'C',		"ADRCN",				NULL,

	// Arrest, Work, Relative Addresses
	0,	0,	0,	0,	'C',		"ArrestAddress",		NULL,
	0,	0,	0,	0,	'C',		"ARADRDIR",			NULL,	// data comes in on ArrestAddress and we parse out the rest
	0,	0,	0,	0,	'C',		"ARADRSTNAME",		NULL,
	0,	0,	0,	0,	'C',		"ARADRSTTYPE",		NULL,
	0,	0,	0,	0,	'C',		"ARADRSUITE",			NULL,
	0,	0,	0,	0,	'C',		"ARADRPO",				NULL,	// ignore
	0,	0,	0,	0,	'C',		"ARADRCT",				NULL,
	0,	0,	0,	0,	'C',		"ARADRST",				NULL,
	0,	0,	0,	0,	'C',		"ARADRZIP",			NULL,
	0,	0,	0,	0,	'C',		"ARADRZIP4",			NULL,	 
	0,	0,	0,	0,	'C',		"ARADRBN",			NULL,	 

	0,	0,	0,	0,	'C',		"WorkAddress",			NULL,
	0,	0,	0,	0,	'C',		"EMPADRDIR",			NULL,	// data comes in on WorkAddress and we parse out the rest
	0,	0,	0,	0,	'C',		"EMPADRSTNAME",		NULL,
	0,	0,	0,	0,	'C',		"EMPADRSTTYPE",		NULL,
	0,	0,	0,	0,	'C',		"EMPADRSUITE",			NULL,
	0,	0,	0,	0,	'C',		"EMPADRPO",				NULL,	// ignore
	0,	0,	0,	0,	'C',		"EMPADRCT",				NULL,
	0,	0,	0,	0,	'C',		"EMPADRST",				NULL,
	0,	0,	0,	0,	'C',		"EMPADRZIP",			NULL,
	0,	0,	0,	0,	'C',		"EMPADRZIP4",			NULL,	 
	0,	0,	0,	0,	'C',		"EMPADRBN",			NULL,	 

	0,	0,	0,	0,	'C',		"RelativeAddress",		NULL,

  // Table: "Identification"
	0,	0,	0,	0,	'C',		"FirstName",		NULL,	 
	2,	18,	1,	1,	'C',		"LastName",			NULL,	// NAM - last,	first,	mid - comma delimited 30chars max
	0,	0,	0,	0,	'C',		"Middle",			NULL,
	0,	0,	0,	0,	'C',		"LastName_dbl",	NULL,	// double last name, as used in Mexico   - not used
	0,	0,	0,	0,	'C',		"FirstName_dbl",	NULL,	// as used in Canada, ie Jean Luc - not used
	0,	0,	0,	0,	'C',		"Middle_dbl",		NULL, // not used
	0,	0,	0,	0,	'C',		"Suffix",			NULL,
	0,	0,	0,	0,	'C',		"Title",				NULL,
	2,	22,	1,	1,	'd',		"DOB",				NULL, 
	0,	0,	0,	0,	'C',		"POB_City",			NULL,
	0,	0,	0,	0,	'C',		"POB_State",		NULL,	   
	0,	0,	0,	0,	'C',		"POB_County",		NULL,	
	2,	20,	1,	1,	'C',		"POB_Country",		NULL,	// POB this is a two letter code - lookup in NCIC POB table
	2,	16,	1,	1,	'C',		"SSN1",				NULL, // SOC
	2,	16,	1,	2,	'C',		"SSN2",				NULL,
	2,	16,	1,	3,	'C',		"SSN3",				NULL,
	3,	 0,	0,	0,	'I',		"Age",				NULL, // figured from DOB
	2,	27,	1,	1,	'I',		"Height",			NULL,	// HGT (inches)
	2,	610,1,	1,	'C',		"DL1_State",		NULL,
	0,	0,	0,	0,	'C',		"DL1_Class",		NULL,
	2,	608,1,	1,	'C',		"DL1_Number",		NULL,
	0,	0,	0,	0,	'C',		"DL1_Expiration",	NULL,
	0,	0,	0,	0,	'C',		"DL2_State",		NULL,
	0,	0,	0,	0,	'C',		"DL2_Class",		NULL,
	0,	0,	0,	0,	'C',		"DL2_Number",		NULL,
	0,	0,	0,	0,	'C',		"DL2_Expiration",	NULL,
	2,	25,	1,	1,	'C',		"Race",				NULL, // RAC is a one char code
	2,	31,	1,	1,	'C',		"EyeColor",			NULL, // EYE three letter code
	0,	0,	0,	0,	'I',		"Glasses",			NULL,
	2,	14,	1,	1,	'C',		"FID",				NULL, // FBI
	0,	0,	0,	0,	'C',		"State",			NULL,
	2,	15,	1,	1,	'C',		"StateID",			NULL, // SID
	2,	29,	1,	1,	'I',		"Weight",			NULL, // WGT
	2,	32,	1,	1,	'C',		"HairColor",		NULL, // HAI - 3 letter code
	2,	24,	1,	1,	'C',		"Sex",				NULL, // SEX - 1 letter code
	0,	0,	0,	0,	'C',		"HairLength",		NULL,
	0,	0,	0,	0,	'C',		"HairType",			NULL,
	0,	0,	0,	0,	'C',		"FacialHair",		NULL,
	2,	21,	1,	1,	'I',		"Alien",			NULL,	// gotten from CTZ
	0,	0,	0,	0,	'I',		"NameRefused",		NULL,
	2,	19,	1,	1,	'C',		"Alias",			NULL,	// only take the first one
	
 // Table: "SMT"
	0,	0,	0,	0,	'I',		"SMT_ID",			NULL,
	2,	26,	1,	1,	'C',		"NCIC",				NULL,	
	0,	0,	0,	0,	'C',		"Notes",			NULL,		// index is "smtNotes"
	0,	0,	0,	0,	'I',		"ImageNum",			NULL,

 // Table: "Images"
	0,	0,	0,	0,	'I',		"Thumbnail",		NULL,		// these are handled specially
	0,	0,	0,	0,	'I',		"Image",				NULL,
	0,	0,	0,	0,	'I',		"ImageNumber",		NULL,

 // Table: "UserFields"
	2,	513,1,	1,	'C',		"UserField1",		NULL,
	2,	514,1,	1,	'C',		"UserField2",		NULL,
	2,	515,1,	1,	'C',		"UserField3",		NULL,
	2,	516,1,	1,	'C',		"UserField4",		NULL,
	2,	517,1,	1,	'C',		"UserField5",		NULL,
	2,	518,1,	1,	'C',		"UserField6",		NULL,
	2,	519,1,	1,	'C',		"UserField7",		NULL,
	2,	520,1,	1,	'C',		"UserField8",		NULL,
	2,	521,1,	1,	'C',		"UserField9",		NULL,
	2,	522,1,	1,	'C',		"UserField10",		NULL,
	2,	523,1,	1,	'C',		"UserField11",		NULL,
	2,	524,1,	1,	'C',		"UserField12",		NULL,

 // Table: "Charges"
	2,	47, 1, 2,	'C',		"Charge",			NULL,  // ASL Arrest Segment Literal holds the charge 
	2,	286,1, 1,	'C',		"Classification",	NULL,

//
//----------------------------------------------------------------------------------
// XImage Extensions:

	// Table: User
	1,	8,  1,	1,  'C',			"station1",		NULL,	// ORI
	10, 4,  1,	1,	'C',		"station10",	NULL,	//	ORI for Type 10
	
	// Table: Booking	
	2,	200,  1,	1,	'C',	"booking_key",	NULL,	// Transaction Control No. - booking_key
	2,	9,    1,	1,	'C',	"case_ref",		NULL,	// Agency Case #
	2,	22,   1,	1,	'd',	"bk_dob",		NULL,	// Date of Birth (ccyymmdd) - bk_dob
	2,	38,   1,	1,	'd',	"bk_created",	NULL,	// Date Printed (ccyymmdd) - bk_created
	2,	219,  1,	1,	'C',	"rec_kind",		NULL,	// Print Type
	2,	201,  1,	1,	'C',	"bk_l_name",	NULL,	// Last Name  - also put in 2.18, "last, first middle" 30c max. - bk_l_name
	2,	202,  1,	1,	'C',	"bk_f_name",	NULL,	// First Name - bk_f_name 
	2,	203,  1,	1,	'C',	"bk_m_name",	NULL,	// Middle Name - bk_m_name
	2,	204,  1,	1,	'C',	"bk_suffix",	NULL,	// Suffix - bk_suffix 
	0,	000,  1,	1,	'C',	"bk_l_name_dbl",NULL,	// Last Name Double
	0,	000,  1,	1,	'C',	"bk_f_name_dbl",NULL,	// First Name Double
	0,	000,  1,	1,	'C',	"bk_m_name_dbl",NULL,	// Middle Name Double
	2,	205,  1,	1,	'I',	"bk_cli_int1 ",	NULL,	// User-Defined
	2,	206,  1,	1,	'I',	"bk_cli_int2 ",	NULL,	// User-Defined
	2,	207,  1,	1,	'C',	"bk_cli_code1",	NULL,	// User-Defined
	2,	208,  1,	1,	'C',	"bk_cli_code2",	NULL,	// User-Defined
	2,	209,  1,	1,	'C',	"bk_cli_code3",	NULL,	// User-Defined
	2,	210,  1,	1,	'C',	"bk_cli_code4",	NULL,	// User-Defined
	2,	211,  1,	1,	'C',	"bk_cli_code5",	NULL,	// User-Defined
	2,	212,  1,	1,	'C',	"bk_cli_code6",	NULL,	// User-Defined
	2,	213,  1,	1,	'C',	"bk_cli_char1",	NULL,	// User-Defined
	2,	214,  1,	1,	'C',	"bk_cli_char2",	NULL,	// User-Defined
	2,	215,  1,	1,	'C',	"bk_cli_char3",	NULL,	// User-Defined
	2,	216,  1,	1,	'C',	"bk_cli_char4",	NULL,	// User-Defined
	2,	314,  1,	1,	'C',	"bk_create_user", NULL, // login id of creator
	2, 	217,  1,	1,	'I',	"bk_note_count",	NULL, // number of note records
	2,  218,  1,	1,	'C',	"bk_note_text",	NULL,	// text of note
	2,  440,  1,	1,	'C',	"Agency",	NULL,	// Agency name
	2,  441,  1,	1,	'C',	"Station",	NULL,	// Station name
	2,  442,  1,	1,	'C',	"Site",	NULL,	// Site name
	2,  443,  1,	1,	'C',	"Division",	NULL,	// Division name
	
	// Table: Folder
	2,	300,  1,	1,	'C',	"fl_masterkey",	NULL,	
	2,	301,  1,	1,	'C',	"fl_cli_int1",		NULL,	
	2,	302,  1,	1,	'C',	"fl_cli_short1",	NULL,
	2,	303,  1,	1,	'C',	"fl_cli_short2",	NULL,
	2,	304,  1,	1,	'C',	"fl_cli_code1",	NULL,	
	2,	305,  1,	1,	'C',	"fl_cli_code2",	NULL,	
	2,	306,  1,	1,	'C',	"fl_cli_code3",	NULL,	
	2,	307,  1,	1,	'C',	"fl_cli_code4",	NULL,	
	2,	308,  1,	1,	'C',	"fl_cli_code5",	NULL,	
	2,	309,  1,	1,	'C',	"fl_cli_code6",	NULL,	
	2,	310,  1,	1,	'C',	"fl_cli_char1",	NULL,	
	2,	311,  1,	1,	'C',	"fl_cli_char2",	NULL,	
	2,	312,  1,	1,	'C',	"fl_cli_char3",	NULL,	
	2,	313,  1,	1,	'C',	"fl_cli_char4",	NULL,	
	2,	315,  1,	1,	'C',	"fl_create_user",	NULL,
	2, 	320,	1,	1,	'I',	"fl_note_count",	NULL, // number of note records
	2,	321,	1,	1,	'C',	"fl_note_text",	NULL,	// text of note

	// Table: Person
	0,	330, 1,		1,	'C',	"pr_create_user",	NULL,	// 	
	2,	331, 1,		1,	'C',	"pr_l_name",		NULL,	// pr_l_name
	2,	332, 1,		1,	'C',	"pr_f_name",		NULL,	// pr_f_name
	2,	333, 1,		1,	'C',	"pr_m_name",		NULL,	// pr_m_name
	2,	334, 1,		1,	'C',	"pr_suffix",		NULL,	//
	0,	000, 1,		1,	'C',	"pr_l_name_dbl",	NULL,	// Last Name Double
	0,	000, 1,		1,	'C',	"pr_f_name_dbl",	NULL,	// First Name Double
	0,	000, 1,		1,	'C',	"pr_m_name_dbl",	NULL,	// Middle Name Double
	2,	335, 1,		1,	'd',	"pr_dob",			NULL,	
	2,	816, 1,		1,	'C',	"pr_SSN",			NULL,	// Person SSN
	2,	600, 1,		1,	'C',	"pr_DL_State",		NULL,	// Person DL State
	2,	602, 1,		1,	'C',	"pr_DL_Number",		NULL,	// Person DL Number

	// Table: Alias
	// This table can have multiple records, 	each needs to be sent to the next subfield in sequence
	2,	220,  1,	1,	'I',	"al_count",	NULL,	// number of Aliases      
	2,	221,  1,	1,	'C',	"al_l_name",	NULL,	// Alias Last Name - al_l_name 
	2,	222,  1,	1,	'C',	"al_f_name",	NULL,	// Alias First Name - al_f_name 
	2,	223,  1,	1,	'C',	"al_m_name",	NULL,	// Alias Middle Name - al_m_name     
	2,	224,  1,	1,	'C',	"al_suffix",	NULL,	// Alias Suffix - al_suffix 
	2,	225,  1,	1,	'd',	"al_dob",		NULL,	// Alias DOB - al_dob
	2,	152,  1,	1,	'd',	"moniker",		NULL,	// Alias Moniker
	
	// Table: Book_Charge	
	// This table can have multiple records, 	each needs to be sent to the next subfield in sequence
	2,	230,  1,	1,	'I',	"chg_count",	NULL,	// number of Charges         
	2,	231,  1,	1,	'I',	"counts",		NULL,	// counts of this charge
	
	// Table: Book_Charge	
	2,	232,  1,	1,	'C',	"code",			NULL,	// Charge code           
	2,	233,  1,	1,	'C',	"statute",		NULL,	// Charge statute        
	2,	234,  1,	1,	'C',	"chg_class",	NULL,	// Charge classification 
	2,	235,  1,	1,	'C',	"degree",		NULL,	// Charge degree         
	2,	236,  1,	1,	'C',	"category",		NULL,	// Charge category       
	2,	237,  1,	1,	'C',	"description",	NULL,	// Charge description    
	2,	238,  1,	1,	'C',	"ncic",			NULL,	// Charge NCIC code - usually a 4 digit number
	2,	239,  1,	1,	'C',	"chargeStr",	NULL,	// Charge String assembled by ForceField Export

	2,	400,  1,	1,	'C',	"IsForceField",	NULL,	// Set to "FieldField" if FF generated	

	// Table: Appearance	
	// This table can have multiple records, 	each needs to be sent to the next subfield in sequence
	2,	250,  1,	1,	'C',	"ap_count",	NULL,	// Appearance count
	2,	251,  1,	1,	'I',	"ap_sex",		NULL,	// Sex  - also put in 2.24 - sex
	2,	252,  1,	1,	'I',	"age",		NULL,	// Age - age 
	2,	253,  1,	1,	'L',	"ap_race",		NULL,	// Race - also put in 2.25 - race
	2,	254,  1,	1,	'L',	"skin_tone",	NULL,	// Skin Tone - skin_tone 
	2,	255,  1,	1,	'I',	"ap_height",	NULL,	// Height - also put in 2.27 - height
	2,	256,  1,	1,	'I',	"ap_weight",	NULL,	// Weight - also put in 2.29 - weight
	2,	257,  1,	1,	'L',	"ap_build",NULL,	// Build - build 
	2,	258,  1,	1,	'L',	"ap_hair_color",	NULL,	// Hair Color - also put in 2.32 - hair_color
	2,	259,  1,	1,	'L',	"ap_hair_style",	NULL,	// Hair Style - hair_style
	2,	260,  1,	1,	'L',	"ap_hair_length",	NULL,	// Hair Length - hair_length 
	2,	261,  1,	1,	'L',	"ap_eye_color",	NULL,	// Eye Color  - also put in 2.31 - eye_color
	2,	262,  1,	1,	'L',	"ap_cor_vision",	NULL,	// Corrected Vision - cor_vision 
	2,	263,  1,	1,	'I',	"ap_FacialHair",	NULL,	// Facial Hair 
	2,	264,  1,	1,	'L',	"ap_FacialHairLength",	NULL,	// Facial Hair Description - facial_hair_desc
	2,	274,  1,	1,	'C',	"ap_Complexion",		NULL,
	2,	265,  1,	1,	'C',	"ap_cli_code1",	NULL,	// User-Defined                  
	2,	266,  1,	1,	'C',	"ap_cli_code2",	NULL,	// User-Defined                  
	2,	267,  1,	1,	'C',	"ap_cli_code3",	NULL,	// User-Defined                  
	2,	268,  1,	1,	'C',	"ap_cli_code4",	NULL,	// User-Defined                  
	2,	269,  1,	1,	'C',	"ap_cli_code5",	NULL,	// User-Defined                  
	2,	270,  1,	1,	'C',	"ap_cli_code6",	NULL,	// User-Defined                  
	2,	271,  1,	1,	'I',	"ap_id",				NULL,	// Image Table ref   
	2,	272,  1,	1,	'I',	"ap_note_count",	NULL,	// number of note records
	2,	273,  1,	1,	'C',	"ap_note_text",	NULL,	// text of note
	2,	275,  1,	1,	'C',	"ap_Beard",		NULL,
	2,	276,  1,	1,	'C',	"ap_Mustache",		NULL,
	2,	277,  1,	1,	'C',	"ap_SideBurns",	NULL,
	0,	0,	0,	0,	'C',		"SkinColor",		NULL,  // NEW CCS2
	2,	500,  1,	1,	'C',	"ap_user_field1",	NULL,	// Appearance user field
	2,	501,  1,	1,	'C',	"ap_user_field2",	NULL,	// Appearance user field
	2,	502,  1,	1,	'C',	"ap_user_field3",	NULL,	// Appearance user field
	2,	503,  1,	1,	'C',	"ap_user_field4",	NULL,	// Appearance user field
	2,	504,  1,	1,	'C',	"ap_user_field5",	NULL,	// Appearance user field
	2,	505,  1,	1,	'C',	"ap_user_field6",	NULL,	// Appearance user field

	// Table: SMT	
	// This table can have multiple records, each needs to be sent to the next subfield in sequence
	2,	280,  1,	1,	'I',	"smt_count",	NULL,	// number of SMT records
	2,	281,  1,	1,	'C',	"body_side",	NULL,	// Body Side
	2,	282,  1,	1,	'C',	"body_part",	NULL,	// Body Part
	2,	283,  1,	1,	'C',	"smt_desc",		NULL,	// SMT Description
	2,	284,  1,	1,	'C',	"text_desc",	NULL,	// Text Description
	2,	285,  1,	1,	'C',	"smt_note_text",NULL,	// SMT notes (CCS2)
	
	// Table: Image	
	// This table can have multiple records	
	// each image requires a separate Type 10 record
	2,	290, 1,	1,	'I',	"img_count",	NULL,	// number of images in NIST file
 	10, 003, 1, 1,	'C',	"img_type",		NULL,	// Image Type - FACE, SCAR, MARK, TATOO, OTHER NOTE: This has to be filled in after the fact!
	10, 005, 1, 1,	'd',	"img_created",	NULL,	// Photo Date                    
	10, 200, 1, 1,	'I',	"table_ref",	NULL,	// Image Table Ref               
	10, 201, 1, 1,	'C',	"img_class",	NULL,	// Image class - face, profile, full body
	10,	210, 1, 1,	'C',	"img_embedded",	NULL,	// Y if embedded, N if not embedded
	10,	220, 1, 1,	'C',	"img_status",	NULL,	// 
	10,	230, 1, 1,	'C',	"img_filename",	NULL,	// Image filename if not embedded
	10,	240, 1, 1,	'C',	"img_id",		NULL,	// Image id (unique identifier)
	10,	40, 1,	1,	'C',	"img_smt_ncic",		NULL,	// SMT NCIC Description
	10,	42, 1,	1,	'C',	"img_smt_desc",		NULL,	// SMT descriptors
	0,	999, 1, 1,	'C',	"JPEG",			NULL,	// Image Data                    
};

/* DJD: Nice table, but I don't see it used in the code anywhere, so for now it's commented out

const IWNISTFIELD g_NIST_Fields[] = 
{
//       |       |       |      |                      
// Record| Field |  Sub  | NIST |
//	Type |  Num  | Field | Code |  Description
//	---- | ----- | ----- | ---- |  ------------------------------------
//
	{ 1,	  1,	 0,	   "LEN",  "Logical Record Length" },
	{ 1,	  2,	 0,	   "VER",  "Version Number" },
	{ 1,	  3,	 0,	   "CNT",  "File Content" },
	{ 1,	  4,	 0,	   "TOT",  "Type of Transaction" },
	{ 1,	  5,	 0,	   "DAT",  "Date" },
	{ 1,	  6,	 0,	   "PRY",  "Priority" },
	{ 1,	  7,	 0,	   "DAI",  "Destination Agency" },
	{ 1,	  8,	 0,	   "ORI",  "Originating Agency" },
	{ 1,	  9,	 0,	   "TCN",  "Transaction Control Number" },
	{ 1,	 10,	 0,	   "TCR",  "Transaction Control Reference" },
	{ 1,	 11,	 0,	   "NSR",  "Native Scanning Resolution" },
	{ 1,	 12,	 0,	   "NTR",  "Native Transmitting Resolution" },

	{ 2,	  1,	 0,	   "LEN",  "Logical Record Length" },
	{ 2,	  2,	 0,	   "IDC",  "Image Designation Character" },
	{ 2,	  3,	 0,	   "FFN",  "FBI File Number" },
	{ 2,	  5,	 0,	   "RET",  "Retention Code" },
	{ 2,	  6,	 0,	   "ATN",  "Attention Indicator" },
	{ 2,	  7,	 0,	   "SCO",  "Send Copy To Set" },
	{ 2,	  8,	 0,	   "NCH",  "No Charge Indicator" },
	{ 2,	  9,	 0,	   "OCA",  "Originating Agency Case Number" },
	{ 2,	 10,	 0,	   "CIN",  "Case Identifier Number" },

	{ 2,	 11,	 0,	   "CIX",  "Case Identifier Number Extention" },
	{ 2,	 12,	 0,	   "LCN",  "FBI Latent Case Number" },
	{ 2,	 13,	 0,	   "LCX",  "FBI Latent Case Extension" },
	{ 2,	 14,	 0,	   "FBI",  "FBI Number" },
	{ 2,	 15,	 0,	   "SID",  "State Identification Number" },
	{ 2,	 16,	 0,	   "SOC",  "Social Security Account Number" },
	{ 2,	 17,	 0,	   "MNU",  "Miscellaneous ID Numbers" },
	{ 2,	 18,	 0,	   "NAM",  "Name" },
	{ 2,	 19,	 0,	   "AKA",  "Aliases" },
	{ 2,	 20,	 0,	   "POB",  "Place of Birth" },
	{ 2,	 21,	 0,	   "CTZ",  "Country of Citizenship" },
	{ 2,	 22,	 0,	   "DOB",  "Date of Birth" },
	{ 2,	 23,	 0,	   "AGR",  "Age Range" },
	{ 2,	 24,	 0,	   "SEX",  "Gender" },
	{ 2,	 25,	 0,	   "RAC",  "Race" },
	{ 2,	 26,	 0,	   "SMT",  "Scars, 0, Marks and Tattoos" },
	{ 2,	 27,	 0,	   "HGT",  "Height" },

// Record| Field |  Sub  | NIST |
//	Type |  Num  | Field | Code |  Description
//	---- | ----- | ----- | ---- |  ------------------------------------

	{ 2,	 28,	 0,	   "HTR",  "Height Range" },
	{ 2,	 29,	 0,	   "WGT",  "Weight" },
	{ 2,	 30,	 0,	   "WTR",  "Weight Range" },
	{ 2,	 31,	 0,	   "EYE",  "Eye Color" },
	{ 2,	 32,	 0,	   "HAI",  "Hair Color" },
	{ 2,	 33,	 0,	   "FPC",  "NCIC Fingerprint Classification" },
	{ 2,	 34,	 0,	   "PAT",  "Pattern Level Class" },
	{ 2,	 35,	 0,	   "PPA",  "Palm Prints Available" },
	{ 2,	 36,	 0,	   "PHT",  "Photo Available" },
	{ 2,	 37,	 0,	   "RFP",  "Reason Fingerprinted" },
	{ 2,	 38,	 0,	   "DPR",  "Date Fingerprinted" },
	{ 2,	 39,	 0,	   "EAD",  "Employer and Address" },
	{ 2,	 40,	 0,	   "OCP",  "Occupation" },
	{ 2,	 41,	 0,	   "RES",  "Residence of Person Fingerprint" },
	{ 2,	 42,	 0,	   "MIL",  "Military Code" },
	{ 2,	 43,	 0,	   "TSR",  "Type of Search Requested" },
	{ 2,	 44,	 0,	   "GEO",  "Geographic Area of Search" },
	{ 2,	 45,	 0,	   "DOA",  "Date of Arrest" },
	{ 2,	 46,	 0,	   "DOS",  "Date of Arrest Suffix" },
	{ 2,	 47,	 0,	   "AOL",  "Arrest Offense Literal" },
	{ 2,	 47,	 1,	   "DOO",  "Date of Offense" },
	{ 2,	 47,	 2,	   "DOR",  "Date of Offense Range" },
	{ 2,	 48,	 0,	   "CSR",  "Civil Search Request Indicator" },
	{ 2,	 51,	 0,	   "CSL",  "Arrest Offense Literal" },
	{ 2,	 51,	 1,	   "CDD",  "Court Disposition Date" },
	{ 2,	 51,	 2,	   "COL",  "Court Offense Literal" },
	{ 2,	 51,	 3,	   "CPL",  "Other Court Sentence Provision" },
	{ 2,	 53,	 0,	   "OFC",  "Offense Category" },
	{ 2,	 54,	 0,	   "SSD",  "Supervision Start Date" },
	{ 2,	 55,	 0,	   "SLE",  "Custody/ Supervision Status Literal" },
	{ 2,	 56,	 0,	   "ICO",  "Identification Comments" },
	{ 2,	 57,	 0,	   "FNR",  "Finger Numbers Requested" },
	{ 2,	 58,	 0,	   "ITD",  "Image Type Desired" },
	{ 2,	 59,	 0,	   "SRF",  "Search Results Findings" },
	{ 2,	 60,	 0,	   "MSG",  "Status / Error Message" },
	{ 2,	 61,	 0,	   "CST",  "Case Title" },
	{ 2,	 62,	 0,	   "IMT",  "Image Type" },
	{ 2,	 63,	 0,	   "PTD",  "Person Type Designator" },
	{ 2,	 64,	 0,	   "CAN",  "Candidate List" },
	{ 2,	 67,	 0,	   "IMA",  "Image Processing Set" },

// Record| Field |  Sub  | NIST |
//	Type |  Num  | Field | Code |  Description
//	---- | ----- | ----- | ---- |  ------------------------------------

	{ 2,	 67,	 1,	   "IMAK", "IMA Make" },
	{ 2,	 67,	 2,	   "IMOD", "IMA Model" },
	{ 2,	 67,	 3,	   "IMSN", "IMA Serial Number" },
	{ 2,	 68,	 0,	   "IPP",  "Image Preprocessing" }, 
	{ 2,	 70,	 0,	   "RAP",  "Request for (FBI) Electronic Rapsheet" },
	{ 2,	 71,	 0,	   "ACN",  "Action To Be Taken" },
	{ 2,	 72,	 0,	   "FIU",  "Fingerprint Image(s) Updated" },
	{ 2,	 73,	 0,	   "CRI",  "Controlling Agency Identifier" },
	{ 2,	100,	 0,	   "REV",  "Software Revision" },
	{ 2,	101,	 0,	   "TBL",  "Tables Version" },
	{ 2,	102,	 0,	   "ACTY", "Activity Type" },
	{ 2,	103,	 0,	   "SCN",  "State Control Number" },
	{ 2,	104,	 0,	   "VR",	"Verification Set" },
	{ 2,	104,	 1,	   "VRF",  "Verified" },
	{ 2,	104,	 2,	   "VRA",  "VR Agency" },
	{ 2,	104,	 3,	   "VRD",  "VR Date"  },
	{ 2,	104,	 4,	   "VRO",  "VR Officer"  },
	{ 2,	105,	 0,	   "LSID", "Live Scan ID" },
	{ 2,	106,	 0,	   "TOT2", "Transaction Type II" },
	{ 2,	107,	 0,	   "???",  "(Flags)" },
	{ 2,	110,	 0,	   "BORI", "Booking ORI Set" },
	{ 2,	110,	 1,	   "BCOD", "BORI Code" },
	{ 2,	110,	 2,	   "BLIT", "BORI Literal" },
	{ 2,	111,	 0,	   "BKF",  "Booking For Set" },
	{ 2,	111,	 1,	   "BKFC", "BKF ORI Code" },
	{ 2,	111,	 2,	   "BKFL", "BKF ORI Literal" },
	{ 2,	112,	 0,	   "OCA",  "Your Number Set" },
	{ 2,	113,	 0,	   "DOA",  "Date of Arrest" },
	{ 2,	115,	 0,	   "CHG",  "Charge Set" },
	{ 2,	115,	 1,	   "CJIS", "CJIS Code" },
	{ 2,	115,	 2,	   "BKTC", "Booking Type Code" },
	{ 2,	115,	 3,	   "BKTL", "Booking Type Literal" },
	{ 2,	115,	 4,	   "NOC",  "Number of Counts" },
	{ 2,	115,	 5,	   "STAT", "Statutory Code" },
	{ 2,	115,	 6,	   "CID",  "Code Type" },
	{ 2,	115,	 7,	   "TOC",  "Type of Charge" },
	{ 2,	115,	 8,	   "IDF",  "Statute Identifier" },
	{ 2,	115,	 9,	   "DEG",  "Degree" },
	{ 2,	115,	10,	   "LIT",  "Literal" },
	{ 2,	115,	11,	   "QUAS", "QUA Statute" },

// Record| Field |  Sub  | NIST |
//	Type |  Num  | Field | Code |  Description
//	---- | ----- | ----- | ---- |  ------------------------------------

	{ 2,	115,	12,	   "QUAC", "QUA Code Type" },
	{ 2,	115,	13,	   "NUM",  "Numbers" },
	{ 2,	115,	14,	   "DOO",  "Date of Offense" },
	{ 2,	115,	15,	   "DOO",  "Date of Offense" },
	{ 2,	116,	 0,	   "JL",   "Jail Time Set" },
	{ 2,	116,	 1,	   "JLL",  "Length of Time" },
	{ 2,	116,	 2,	   "JLTC", "Sentence Time Code" },
	{ 2,	116,	 3,	   "JLTL", "Sentence Time Literal" },
	{ 2,	116,	 4,	   "JLJC", "Sentence Type Code" },
	{ 2,	116,	 5,	   "JLJL", "Sentence Type Literal" },
	{ 2,	117,	 0,	   "FP",   "Fingerprint Set" },
	{ 2,	117,	 1,	   "FPD",  "Fingerprint Date" },
	{ 2,	117,	 2,	   "FPO",  "Fingerprint Official" },
	{ 2,	118,	 0,	   "ICO",  "Basis for Caution" },
	{ 2,	119,	 0,	   "JUV",  "Juvenile Disposition Set" },
	{ 2,	119,	 1,	   "JDD",  "Juvenile Disposition Date"  },
	{ 2,	119,	 2,	   "JDL",  "Juvenile Disposition Literal"  },
	{ 2,	119,	 3,	   "JDC",  "Juvenile Disposition Code"  },
	{ 2,	120,	 0,	   "PHA",  "Photo Available" },
	{ 2,	121,	 0,	   "RAP",  "CA Rapback" },
	{ 2,	122,	 0,	   "SCO",  "Send Copy To Set" },
	{ 2,	122,	 1,	   "SCT",  "SCO ORI" },
	{ 2,	122,	 2,	   "SCL",  "SCO Literal"  },
	{ 2,	122,	 3,	   "RTE",  "SCO Route"  },
	{ 2,	123,	 0,	   "ATN",  "CA Attention" },
	{ 2,	124,	 0,	   "IPA",  "Image Pattern/Annotation Set" },
	{ 2,	124,	 1,	   "FING", "Finger Number" },
	{ 2,	124,	 2,	   "ANC",  "Annotation Code" },
	{ 2,	150,	 0,	   "NAM",  "Name Set" },
	{ 2,	150,	 1,	   "LAST", "Last Name" },
	{ 2,	150,	 2,	   "FIRST","First Name" },
	{ 2,	150,	 3,	   "MID",  "Middle Name" },
	{ 2,	150,	 4,	   "SUF",  "Suffix Name" },
	{ 2,	151,	 0,	   "AKA",  "Alias Name Set" },
	{ 2,	151,	 1,	   "AKL",  "AKA Last" },
	{ 2,	151,	 2,	   "AKF",  "AKA First" },
	{ 2,	151,	 3,	   "AKM",  "AKA Middle" },
	{ 2,	151,	 4,	   "AKS",  "AKA Suffix" },
	{ 2,	152,	 0,	   "MON",  "Moniker" },
	{ 2,	153,	 0,	   "DOB",  "Date Of Birth" },

// Record| Field |  Sub  | NIST |
//	Type |  Num  | Field | Code |  Description
//	---- | ----- | ----- | ---- |  ------------------------------------

	{ 2,	154,	 0,	   "SEX",  "Gender" },
	{ 2,	155,	 0,	   "RAC",  "Race" },
	{ 2,	156,	 0,	   "EYE",  "Eye Color" },
	{ 2,	158,	 0,	   "POB",  "Place of Birth Set" },
	{ 2,	158,	 1,	   "PB",   "State" },
	{ 2,	158,	 2,	   "CTY",  "PB City" },
	{ 2,	159,	 0,	   "CTZ",  "Citizenship Set" },
	{ 2,	159,	 1,	   "CTZC", "CTZ Code CTZD CTZ Documented" },
	{ 2,	160,	 0,	   "SMT",  "Scars, Marks, and Tattoos Set" },
	{ 2,	160,	 1,	   "SMTC", "SMT Code" },
	{ 2,	160,	 2,	   "SMTD", "SMT Descriptor" },
	{ 2,	161,	 0,	   "MDS",  "Miscellaneous Descriptors" },
	{ 2,	162,	 0,	   "ADR",  "Address Set" },
	{ 2,	162,	 1,	   "ADRH", "House Number" },
	{ 2,	162,	 2,	   "ADRD", "Direction" },
	{ 2,	162,	 3,	   "ADRN", "Street Name" },
	{ 2,	162,	 4,	   "ADRA", "Apartment Number" },
	{ 2,	162,	 5,	   "ADRC", "ADR City" },
	{ 2,	162,	 6,	   "ADRS", "ADR State" },
	{ 2,	162,	 7,	   "ADRZ", "ADR Zip" },
	{ 2,	162,	 8,	   "ADRX", "ADR X-Zip" },
	{ 2,	162,	 9,	   "ADRY", "Years at Address" },
	{ 2,	162,	10,	   "ADRM", "Months at Addres" },
	{ 2,	163,	 0,	   "EAD",  "Employer/Address Set" },
	{ 2,	163,	 1,	   "EMP",  "Employer" },
	{ 2,	163,	 2,	   "EMPA", "EMP Street Number" },
	{ 2,	163,	 3,	   "EMPD", "Direction" },
	{ 2,	163,	 4,	   "EMPN", "EMP Street" },
	{ 2,	163,	 5,	   "EMPO", "Office Number" },
	{ 2,	163,	 6,	   "EMPC", "EMP City" },
	{ 2,	163,	 7,	   "EMPS", "EMP State" },
	{ 2,	163,	 8,	   "EMPZ", "EMP Zip" },
	{ 2,	163,	 9,	   "EMPX", "EMP X-Zip" },
	{ 2,	163,	10,	   "EMPP", "EMP Phone" },
	{ 2,	164,	 0,	   "OCC",  "Occupation" },
	{ 2,	165,	 0,	   "HGT",  "Height" },
	{ 2,	166,	 0,	   "WGT",  "Weight" },
	{ 2,	170,	 0,	   "SID",  "State Identification Number" },
	{ 2,	171,	 0,	   "SOC",  "Social Security Number" },
	{ 2,	172,	 0,	   "CDL",  "CA Driver's License" },

// Record| Field |  Sub  | NIST |
//	Type |  Num  | Field | Code |  Description
//	---- | ----- | ----- | ---- |  ------------------------------------

	{ 2,	173,	 0,	   "MNU",  "Miscellaneous ID Numbers" },
	{ 2,	174,	 0,	   "FBI",  "FBI Number" },
	{ 2,	200,	 0,	   "RTYP", "Reg Type" },
	{ 2,	201,	 0,	   "ARR",  "Arresting Agency" },
	{ 2,	203,	 0,	   "TYN",  "Type of Narcotic" },
	{ 2,	204,	 0,	   "DOC",  "Date of Conviction" },
	{ 2,	205,	 0,	   "DOR",  "Date of Release" },
	{ 2,	206,	 0,	   "SPR",  "Supervision Set" },
	{ 2,	206,	 1,	   "SPRA", "SPR Agency" },
	{ 2,	206,	 2,	   "SPRL", "SPR Location" },
	{ 2,	206,	 3,	   "SPRO", "SPR Officer" },
	{ 2,	206,	 4,	   "SPRP", "SPR Phone" },
	{ 2,	206,	 5,	   "SPRM", "SPR Terminal MNE" },
	{ 2,	207,	 0,	   "GRD",  "Grid Code Set" },
	{ 2,	207,	 1,	   "GRDO", "Grid Coordinate" },
	{ 2,	207,	 2,	   "GRDP", "Grid Page" },
	{ 2,	208,	 0,	   "VEH",  "Vehicle Set" },
	{ 2,	208,	 1,	   "VLIC", "VEH License" },
	{ 2,	208,	 2,	   "VSTA", "VEH State" },
	{ 2,	208,	 3,	   "VMAK", "VEH Make" },
	{ 2,	208,	 4,	   "VMOD", "VEH Model" },
	{ 2,	208,	 5,	   "VYR",  "VEH Year" },
	{ 2,	208,	 6,	   "VCOL", "VEH Color" },
	{ 2,	208,	 7,	   "VVIN", "VEH VIN Number" },
	{ 2,	250,	 0,	   "OBKN", "Original BKN" },
	{ 2,	251,	 0,	   "ODOA", "Original DOA" },
	{ 2,	252,	 0,	   "ONAM", "Original (Last) Name" },
	{ 2,	253,	 0,	   "OBORI","Original BORI" },
	{ 2,	254,	 0,	   "OSCN", "Original OSCN" },
	{ 2,	300,	 0,	   "CTYP", "Custody Type" },
	{ 2,	300,	 1,	   "CTYC", "CTYP Code" },
	{ 2,	300,	 2,	   "CTYL", "CTYP Literal" },
	{ 2,	301,	 0,	   "CMT",  "Commitment Status Set" },
	{ 2,	301,	 1,	   "CNT1", "CMT Hookup" },
	{ 2,	301,	 2,	   "CMTJ", "CMT CJIS Code" },
	{ 2,	301,	 3,	   "CMTS", "CMT Statute" },
	{ 2,	301,	 4,	   "CMTC", "CMT Code Identifier" },
	{ 2,	301,	 5,	   "CMTL", "CMT Literal" },
	{ 2,	301,	 6,	   "RCVC", "RCVD Code" },
	{ 2,	301,	 7,	   "RCVL", "RCVD Literal" },

// Record| Field |  Sub  | NIST |
//	Type |  Num  | Field | Code |  Description
//	---- | ----- | ----- | ---- |  ------------------------------------

	{ 2,	301,	 8,	   "CMTN", "CMT Number" },
	{ 2,	302,	 0,	   "CVT",  "Convicted Offense Set" },
	{ 2,	302,	 1,	   "CNT2", "CVT Hookup" },
	{ 2,	302,	 2,	   "CJIS", "CJIS Code" },
	{ 2,	302,	 3,	   "NOC",  "Number of Counts" },
	{ 2,	302,	 4,	   "STAT", "Statutory Code" },
	{ 2,	302,	 5,	   "CID",  "Code Type" },
	{ 2,	302,	 6,	   "TOC",  "Type of Charge" },
	{ 2,	302,	 7,	   "IDF",  "Statute Identifier" },
	{ 2,	302,	 8,	   "DEG",  "Degree" },
	{ 2,	302,	 9,	   "LIT",  "Literal" },
	{ 2,	302,	10,	   "QUAS", "QUA Statute" },
	{ 2,	302,	11,	   "QUAC", "QUA Code Type" },
	{ 2,	302,	12,	   "SNL",  "SEN Length" },
	{ 2,	302,	13,	   "SNTC", "SEN Time Code" },
	{ 2,	302,	14,	   "SNTL", "SEN Time Literal" },
	{ 2,	302,	15,	   "SNYC", "SEN Type Code" },
	{ 2,	302,	16,	   "SNYL", "SEN Type Literal" },
	{ 2,	302,	17,	   "SNC",  "SEN CC/CS Court Case" },
	{ 2,	303,	 0,	   "EXP",  "Expiration Date" },
	{ 2,	304,	 0,	   "COND", "Conditions" },
	{ 2,	305,	 0,	   "FIRE", "Firearms Prohibition" },
	{ 2,	307,	 0,	   "SSD",  "Supervision Start Date" },

	{ 4,	  1,	 0,	   "LEN",	"Logical Record Length" },
	{ 4,	  2,	 0,	   "IDC",	"Image Designation Character" },
	{ 4,	  3,	 0,	   "IMP",	"Impression Type" },
	{ 4,	  1,	 3,	   "IMP",	"Live-scan slap" },
	{ 4,	  2,	 3,	   "IMP",	"Live-scan roll" },
	{ 4,	  3,	 3,	   "IMP",	"Non-live-scan slap" },
	{ 4,	  4,	 3,	   "IMP",	"Non-live-scan roll" },
	{ 4,	  4,	 0,	   "FGP",	"Finger Position" },
	{ 4,	  4,	 1,	   "RT" ,	"Right thumb" },
	{ 4,	  4,	 2,	   "RI" ,  "Right index finger" },
	{ 4,	  4,	 3,	   "RM" ,  "Right middle finger" },
	{ 4,	  4,	 4,	   "RR" ,  "Right ring finger" },
	{ 4,	  4,	 5,	   "RL" ,  "Right little finger" },
	{ 4,	  4,	 6,	   "LT" ,  "Left thumb" },
	{ 4,	  4,	 7,	   "LI" ,  "Left index finger" },
	{ 4,	  4,	 8,	   "LM" ,  "Left middle finger" },

// Record| Field |  Sub  | NIST |
//	Type |  Num  | Field | Code |  Description
//	---- | ----- | ----- | ---- |  ------------------------------------

	{ 4,	  4,	 9,	   "LR" ,  "Left ring finger" },
	{ 4,	  4,	10,	   "LL" ,  "Left little finger" },
	{ 4,	  4,	11,	   "RTS",	"Right thumb slap" },
	{ 4,	  4,	12,	   "LTS",	"Left thumb slap" },
	{ 4,	  4,	13,	   "R4S",	"Right four slap" },
	{ 4,	  4,	14,	   "L4S",	"Left four slap" },
	{ 4,	  5,	 0,	   "ISR",	"Image Scanning Resolution" },
	{ 4,	  6,	 0,	   "HLL",	"Horz. Line Length" },
	{ 4,	  7,	 0,	   "VLL",	"Vert. Line Length" },
	{ 4,	  8,	 0,	   "GCA",	"Compression Alogrithm" },
	{ 4,	999,	 0,	   "DAT",	"Image Data" },

	{ 10,	  1,	 0,	   "LEN",	"Logical Record Length" },
	{ 10,	  2,	 0,	   "IDC",	"Image Designation Character" },
	{ 10,	  3,	 0,	   "IMT",	"Image Type" },
	{ 10,	  4,	 0,	   "SRC",	"Source Agency" },
	{ 10,	  5,	 0,	   "PHD",	"Photo Date" },
	{ 10,	  6,	 0,	   "HLL",	"Image Width" },
	{ 10,	  7,	 0,	   "VLL",	"Image Height" },
	{ 10,	  8,	 0,	   "SLC",	"Scale Units" },
	{ 10,	  9,	 0,	   "HPS",	"Horz. Pixel Scale" },
	{ 10,	 10,	 0,	   "VPS",	"Vert. Pixel Scale" },
	{ 10,	 11,	 0,	   "CGA",	"Compression Algorithm" },
	{ 10,	 12,	 0,	   "CSP",	"Color Space" },
	{ 10,	 20,	 0,	   "POS",	"Subject Pose" },
	{ 10,	 21,	 0,	   "POA",	"Pose Offset Angle" },
	{ 10,	 22,	 0,	   "PXS",	"Photo Description" },
	{ 10,	 40,	 0,	   "SMT",	"NCIC Designation Code" },
	{ 10,	 41,	 0,	   "SMS",	"SMT Size" },
	{ 10,	 42,	 0,	   "SMD",	"SMT Descriptors" },
	{ 10,	 43,	 0,	   "COL",	"Colors Present" },
	{ 10,	200,	 0,	   "UDF",	"User-Defined Field" },
	{ 10,	999,	 0,	   "DAT",	"Image Data" }
 };	
	
const int g_NIST_Fields_Count = sizeof(g_NIST_Fields)/sizeof(g_NIST_Fields[0]);
*/
