#pragma pack(push,1)

typedef
struct _omf_record_header
{
	BYTE type;			// Type of record
	WORD length;		// Length or record (without header)
} omf_record_header;

// Type F0h. Library header.
typedef
struct _omf_lib_header
{
	omf_record_header header;		// Header
	DWORD dictionary_offset;		// Dictionary offset
	WORD dictionary_size;			// Dictionary size in 512-byte Blocks
	BYTE flags;						// Flags: 0x01 = case sensitive (applies to both regular and extended dictionaries)
} omf_lib_header;

#pragma pack(pop)
