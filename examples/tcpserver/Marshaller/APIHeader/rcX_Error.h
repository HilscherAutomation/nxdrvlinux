#ifndef __RCX_ERROR_H
#define __RCX_ERROR_H

/* /////////////////////////////////////////////////////////////////////////////////// */
/*  RCX Task error codes */
/* /////////////////////////////////////////////////////////////////////////////////// */
/*  */
/*  MessageId: TLR_E_RCX_QUE_UNKNOWN */
/*  */
/*  MessageText: */
/*  */
/*   Queue unknown. */
/*  */
#define TLR_E_RCX_QUE_UNKNOWN            ((TLR_RESULT)0xC02B0001L)

/*  */
/*  MessageId: TLR_E_RCX_QUE_IDX_UNKNOWN */
/*  */
/*  MessageText: */
/*  */
/*   Queue table index does not exist. */
/*  */
#define TLR_E_RCX_QUE_IDX_UNKNOWN        ((TLR_RESULT)0xC02B0002L)

/*  */
/*  MessageId: TLR_E_RCX_TSK_UNKNOWN */
/*  */
/*  MessageText: */
/*  */
/*   Task unknown. */
/*  */
#define TLR_E_RCX_TSK_UNKNOWN            ((TLR_RESULT)0xC02B0003L)

/*  */
/*  MessageId: TLR_E_RCX_TSK_IDX_UNKNOWN */
/*  */
/*  MessageText: */
/*  */
/*   Task table index does not exist. */
/*  */
#define TLR_E_RCX_TSK_IDX_UNKNOWN        ((TLR_RESULT)0xC02B0004L)

/*  */
/*  MessageId: TLR_E_RCX_TSK_HANDLE_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Task handle invalid. */
/*  */
#define TLR_E_RCX_TSK_HANDLE_INVALID     ((TLR_RESULT)0xC02B0005L)

/*  */
/*  MessageId: TLR_E_RCX_TSK_INFO_IDX_UNKNOWN */
/*  */
/*  MessageText: */
/*  */
/*   Task info field index unknown. */
/*  */
#define TLR_E_RCX_TSK_INFO_IDX_UNKNOWN   ((TLR_RESULT)0xC02B0006L)

/*  */
/*  MessageId: TLR_I_RCX_FILE_RETRANSMIT */
/*  */
/*  MessageText: */
/*  */
/*   The last data block was invalid, please retransmit. */
/*  */
#define TLR_I_RCX_FILE_RETRANSMIT        ((TLR_RESULT)0x402B0001L)

/*  */
/*  MessageId: TLR_E_RCX_FILE_XFR_TYPE_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Requested transfer type invalid. */
/*  */
#define TLR_E_RCX_FILE_XFR_TYPE_INVALID  ((TLR_RESULT)0xC02B0007L)

/*  */
/*  MessageId: TLR_E_RCX_FILE_REQUEST_INCORRECT */
/*  */
/*  MessageText: */
/*  */
/*   Request is incorrectly formatted i.e. wrong parameters. */
/*  */
#define TLR_E_RCX_FILE_REQUEST_INCORRECT ((TLR_RESULT)0xC02B0008L)

/*  */
/*  MessageId: TLR_E_RCX_UNKNOWN_PORT_INDEX */
/*  */
/*  MessageText: */
/*  */
/*   Unknown port index. */
/*  */
#define TLR_E_RCX_UNKNOWN_PORT_INDEX     ((TLR_RESULT)0xC02B0009L)

/*  */
/*  MessageId: TLR_E_RCX_ROUTER_TABLE_FULL */
/*  */
/*  MessageText: */
/*  */
/*   Router Table is full. */
/*  */
#define TLR_E_RCX_ROUTER_TABLE_FULL      ((TLR_RESULT)0xC02B000AL)

/*  */
/*  MessageId: TLR_E_RCX_NO_SUCH_ROUTER_IN_TABLE */
/*  */
/*  MessageText: */
/*  */
/*   No such router in table. */
/*  */
#define TLR_E_RCX_NO_SUCH_ROUTER_IN_TABLE ((TLR_RESULT)0xC02B000BL)

/*  */
/*  MessageId: TLR_E_RCX_INSTANCE_NOT_NULL */
/*  */
/*  MessageText: */
/*  */
/*   Mid_Sys Instance is not 0. */
/*  */
#define TLR_E_RCX_INSTANCE_NOT_NULL      ((TLR_RESULT)0xC02B000CL)

/*  */
/*  MessageId: TLR_E_RCX_COMMAND_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Invalid command. */
/*  */
#define TLR_E_RCX_COMMAND_INVALID        ((TLR_RESULT)0xC02B000DL)

/*  */
/*  MessageId: TLR_E_RCX_TSK_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Invalid task handle. */
/*  */
#define TLR_E_RCX_TSK_INVALID            ((TLR_RESULT)0xC02B000EL)

/*  */
/*  MessageId: TLR_E_RCX_TSK_NOT_A_USER_TASK */
/*  */
/*  MessageText: */
/*  */
/*   Access denied. Not a user task (See Config-File). */
/*  */
#define TLR_E_RCX_TSK_NOT_A_USER_TASK    ((TLR_RESULT)0xC02B000FL)

/*  */
/*  MessageId: TLR_E_RCX_LOG_QUE_NOT_SETTABLE */
/*  */
/*  MessageText: */
/*  */
/*   Logical queue handle not settable. */
/*  */
#define TLR_E_RCX_LOG_QUE_NOT_SETTABLE   ((TLR_RESULT)0xC02B0010L)

/*  */
/*  MessageId: TLR_E_RCX_LOG_QUE_NOT_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Logical queue handle invalid. */
/*  */
#define TLR_E_RCX_LOG_QUE_NOT_INVALID    ((TLR_RESULT)0xC02B0011L)

/*  */
/*  MessageId: TLR_E_RCX_LOG_QUE_NOT_SET */
/*  */
/*  MessageText: */
/*  */
/*   Logical queue handle has not been set. */
/*  */
#define TLR_E_RCX_LOG_QUE_NOT_SET        ((TLR_RESULT)0xC02B0012L)

/*  */
/*  MessageId: TLR_E_RCX_LOG_QUE_ALREADY_USED */
/*  */
/*  MessageText: */
/*  */
/*   Logical queue handle is already in use. */
/*  */
#define TLR_E_RCX_LOG_QUE_ALREADY_USED   ((TLR_RESULT)0xC02B0013L)

/*  */
/*  MessageId: TLR_E_RCX_TSK_NO_DEFAULT_QUEUE */
/*  */
/*  MessageText: */
/*  */
/*   Task has no default process queue. */
/*  */
#define TLR_E_RCX_TSK_NO_DEFAULT_QUEUE   ((TLR_RESULT)0xC02B0014L)

/*  */
/*  MessageId: TLR_E_RCX_MODULE_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Firmware Module is invalid. CRC-32 check failed. */
/*  */
#define TLR_E_RCX_MODULE_INVALID         ((TLR_RESULT)0xC02B0015L)

/*  */
/*  MessageId: TLR_E_RCX_MODULE_NOT_FOUND */
/*  */
/*  MessageText: */
/*  */
/*   Firmware Module has not been found. Maybe it has not been downloaded before. */
/*  */
#define TLR_E_RCX_MODULE_NOT_FOUND       ((TLR_RESULT)0xC02B0016L)

/*  */
/*  MessageId: TLR_E_RCX_MODULE_RELOC_ERROR */
/*  */
/*  MessageText: */
/*  */
/*   Firmware Module has an invalid reloc table. */
/*  */
#define TLR_E_RCX_MODULE_RELOC_ERROR     ((TLR_RESULT)0xC02B0017L)

/*  */
/*  MessageId: TLR_E_RCX_MODULE_NO_INIT_TBL */
/*  */
/*  MessageText: */
/*  */
/*   Firmware Module has no init table. */
/*  */
#define TLR_E_RCX_MODULE_NO_INIT_TBL     ((TLR_RESULT)0xC02B0018L)

/*  */
/*  MessageId: TLR_E_RCX_MODULE_NO_ENTRY_POINT */
/*  */
/*  MessageText: */
/*  */
/*   Firmware Module has no code entry point. */
/*  */
#define TLR_E_RCX_MODULE_NO_ENTRY_POINT  ((TLR_RESULT)0xC02B0019L)

/*  */
/*  MessageId: TLR_E_RCX_ACCESS_DENIED_IN_LOCKED_STATE */
/*  */
/*  MessageText: */
/*  */
/*   Access denied due to current operating conditions. */
/*  */
#define TLR_E_RCX_ACCESS_DENIED_IN_LOCKED_STATE ((TLR_RESULT)0xC02B001AL)

/*  */
/*  MessageId: TLR_E_RCX_INVALID_FIRMWARE_SIZE */
/*  */
/*  MessageText: */
/*  */
/*   Firmware does not fit into flash. */
/*  */
#define TLR_E_RCX_INVALID_FIRMWARE_SIZE  ((TLR_RESULT)0xC02B001BL)

/*  */
/*  MessageId: TLR_E_RCX_MODULE_RELOCATION_DISTANCE_TOO_LONG */
/*  */
/*  MessageText: */
/*  */
/*   The relocation distance is too long. */
/*  */
#define TLR_E_RCX_MODULE_RELOCATION_DISTANCE_TOO_LONG ((TLR_RESULT)0xC02B001CL)

/*  */
/*  MessageId: TLR_E_RCX_SEC_FAILED */
/*  */
/*  MessageText: */
/*  */
/*   Access to the security flash failed. */
/*  */
#define TLR_E_RCX_SEC_FAILED             ((TLR_RESULT)0xC02B001DL)

/*  */
/*  MessageId: TLR_E_RCX_SEC_DISABLED */
/*  */
/*  MessageText: */
/*  */
/*   Security flash is disabled at firmware. */
/*  */
#define TLR_E_RCX_SEC_DISABLED           ((TLR_RESULT)0xC02B001EL)

/*  */
/*  MessageId: TLR_E_RCX_INVALID_EXTENSION */
/*  */
/*  MessageText: */
/*  */
/*   Invalid Extension field. */
/*  */
#define TLR_E_RCX_INVALID_EXTENSION      ((TLR_RESULT)0xC02B001FL)

/*  */
/*  MessageId: TLR_E_RCX_BLOCK_SIZE_OUT_OF_RANGE */
/*  */
/*  MessageText: */
/*  */
/*   Block size out of range. */
/*  */
#define TLR_E_RCX_BLOCK_SIZE_OUT_OF_RANGE ((TLR_RESULT)0xC02B0020L)

/*  */
/*  MessageId: TLR_E_RCX_INVALID_CHANNEL */
/*  */
/*  MessageText: */
/*  */
/*   Invalid Channel. */
/*  */
#define TLR_E_RCX_INVALID_CHANNEL        ((TLR_RESULT)0xC02B0021L)

/*  */
/*  MessageId: TLR_E_RCX_INVLAID_FILE_LENGTH */
/*  */
/*  MessageText: */
/*  */
/*   Invalid File Length. */
/*  */
#define TLR_E_RCX_INVLAID_FILE_LENGTH    ((TLR_RESULT)0xC02B0022L)

/*  */
/*  MessageId: TLR_E_RCX_INVALID_CHARACTER */
/*  */
/*  MessageText: */
/*  */
/*   Invalid Character. */
/*  */
#define TLR_E_RCX_INVALID_CHARACTER      ((TLR_RESULT)0xC02B0023L)

/*  */
/*  MessageId: TLR_E_RCX_PACKET_OUT_OF_SEQUENCE */
/*  */
/*  MessageText: */
/*  */
/*   Packet out of sequence. */
/*  */
#define TLR_E_RCX_PACKET_OUT_OF_SEQUENCE ((TLR_RESULT)0xC02B0024L)

/*  */
/*  MessageId: TLR_E_RCX_NOT_POSSIBLE_IN_CURRENT_STATE */
/*  */
/*  MessageText: */
/*  */
/*   Not possible in current state. */
/*  */
#define TLR_E_RCX_NOT_POSSIBLE_IN_CURRENT_STATE ((TLR_RESULT)0xC02B0025L)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_INVALID_ZONE */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom Zone Parameter is invalid. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_INVALID_ZONE ((TLR_RESULT)0xC02B0026L)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_NOT_ALLOWED */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom access is not allowed in current state. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_NOT_ALLOWED ((TLR_RESULT)0xC02B0027L)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_NOT_AVAILABLE */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom is not available. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_NOT_AVAILABLE ((TLR_RESULT)0xC02B0028L)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_INVALID_CHECKSUM */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom has an invalid checksum. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_INVALID_CHECKSUM ((TLR_RESULT)0xC02B0029L)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_ZONE_NOT_WRITABLE */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom Zone is not writeable. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_ZONE_NOT_WRITABLE ((TLR_RESULT)0xC02B002AL)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_READ_FAILED */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom Read Failed. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_READ_FAILED ((TLR_RESULT)0xC02B002BL)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_WRITE_FAILED */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom Write Failed. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_WRITE_FAILED ((TLR_RESULT)0xC02B002CL)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_ZONE_ACCESS_DENIED */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom Zone Access Denied. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_ZONE_ACCESS_DENIED ((TLR_RESULT)0xC02B002DL)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_EMULATED */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom Emulated. No write possible. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_EMULATED ((TLR_RESULT)0xC02B002EL)

/*  */
/*  MessageId: TLR_E_RCX_FILE_NAME_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   File name is invalid. */
/*  */
#define TLR_E_RCX_FILE_NAME_INVALID      ((TLR_RESULT)0xC02B002FL)

/*  */
/*  MessageId: TLR_E_RCX_FILE_SEQUENCE_ERROR */
/*  */
/*  MessageText: */
/*  */
/*   File Sequence Error. */
/*  */
#define TLR_E_RCX_FILE_SEQUENCE_ERROR    ((TLR_RESULT)0xC02B0030L)

/*  */
/*  MessageId: TLR_E_RCX_FILE_SEQUENCE_END_ERROR */
/*  */
/*  MessageText: */
/*  */
/*   File Sequence End Error. */
/*  */
#define TLR_E_RCX_FILE_SEQUENCE_END_ERROR ((TLR_RESULT)0xC02B0031L)

/*  */
/*  MessageId: TLR_E_RCX_FILE_SEQUENCE_BEGIN_ERROR */
/*  */
/*  MessageText: */
/*  */
/*   File Sequence Begin Error. */
/*  */
#define TLR_E_RCX_FILE_SEQUENCE_BEGIN_ERROR ((TLR_RESULT)0xC02B0032L)

/*  */
/*  MessageId: TLR_E_RCX_UNEXPECTED_BLOCK_SIZE */
/*  */
/*  MessageText: */
/*  */
/*   Unexpected File Transfer Block Size. */
/*  */
#define TLR_E_RCX_UNEXPECTED_BLOCK_SIZE  ((TLR_RESULT)0xC02B0033L)

/*  */
/*  MessageId: TLR_E_HIL_FILE_HEADER_CRC_ERROR */
/*  */
/*  MessageText: */
/*  */
/*   Hilscher File Header has invalid CRC error. */
/*  */
#define TLR_E_HIL_FILE_HEADER_CRC_ERROR  ((TLR_RESULT)0xC02B0034L)

/*  */
/*  MessageId: TLR_E_HIL_FILE_HEADER_MODULE_SIZE_DIFFERS */
/*  */
/*  MessageText: */
/*  */
/*   Hilscher File Header specifies a different module size than the actual module header itself. */
/*  */
#define TLR_E_HIL_FILE_HEADER_MODULE_SIZE_DIFFERS ((TLR_RESULT)0xC02B0035L)

/*  */
/*  MessageId: TLR_E_HIL_FILE_HEADER_MD5_CHECKSUM_ERROR */
/*  */
/*  MessageText: */
/*  */
/*   Hilscher File Header contains a wrong MD-5 checksum for file data. */
/*  */
#define TLR_E_HIL_FILE_HEADER_MD5_CHECKSUM_ERROR ((TLR_RESULT)0xC02B0036L)

/*  */
/*  MessageId: TLR_E_RCX_PACKET_WOULD_BE_TOO_LONG_FOR_MTU */
/*  */
/*  MessageText: */
/*  */
/*   The packet would be too long for transfer. */
/*  */
#define TLR_E_RCX_PACKET_WOULD_BE_TOO_LONG_FOR_MTU ((TLR_RESULT)0xC02B0037L)

/*  */
/*  MessageId: TLR_E_INVALID_BLOCK */
/*  */
/*  MessageText: */
/*  */
/*   Invalid block id */
/*  */
#define TLR_E_INVALID_BLOCK              ((TLR_RESULT)0xC02B0038L)

/*  */
/*  MessageId: TLR_E_INVALID_STRUCT_NUMBER */
/*  */
/*  MessageText: */
/*  */
/*   Invalid structure number */
/*  */
#define TLR_E_INVALID_STRUCT_NUMBER      ((TLR_RESULT)0xC02B0039L)

/*  */
/*  MessageId: TLR_E_HIL_FILE_HEADER_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Invalid file header */
/*  */
#define TLR_E_HIL_FILE_HEADER_INVALID    ((TLR_RESULT)0xC02B003AL)

/*  */
/*  MessageId: TLR_E_LICENSE_CHIPTYPE_UNSUPPORTED */
/*  */
/*  MessageText: */
/*  */
/*   Target device not supported for license update */
/*  */
#define TLR_E_LICENSE_CHIPTYPE_UNSUPPORTED ((TLR_RESULT)0xC02B003BL)

/*  */
/*  MessageId: TLR_E_LICENSE_CHIPTYPE_MISMATCH */
/*  */
/*  MessageText: */
/*  */
/*   License incompatible for target device */
/*  */
#define TLR_E_LICENSE_CHIPTYPE_MISMATCH  ((TLR_RESULT)0xC02B003CL)

/*  */
/*  MessageId: TLR_E_LICENSE_HW_MISMATCH */
/*  */
/*  MessageText: */
/*  */
/*   License generated for different device */
/*  */
#define TLR_E_LICENSE_HW_MISMATCH        ((TLR_RESULT)0xC02B003DL)

/*  */
/*  MessageId: TLR_E_MODULE_CONTAINS_NO_MODULE_DESCRIPTOR */
/*  */
/*  MessageText: */
/*  */
/*   Missing module descriptor in module. */
/*  */
#define TLR_E_MODULE_CONTAINS_NO_MODULE_DESCRIPTOR ((TLR_RESULT)0xC02B003EL)

/*  */
/*  MessageId: TLR_E_MODULE_CONTAINS_UNKNOWN_VERSION */
/*  */
/*  MessageText: */
/*  */
/*   Unknown version in module descriptor. */
/*  */
#define TLR_E_MODULE_CONTAINS_UNKNOWN_VERSION ((TLR_RESULT)0xC02B003FL)

/*  */
/*  MessageId: TLR_E_MODULE_HAS_NO_INIT_FUNCTION */
/*  */
/*  MessageText: */
/*  */
/*   Module has no init function. */
/*  */
#define TLR_E_MODULE_HAS_NO_INIT_FUNCTION ((TLR_RESULT)0xC02B0040L)

/*  */
/*  MessageId: TLR_E_MODULE_OFFSET_RANGE_ERROR */
/*  */
/*  MessageText: */
/*  */
/*   Module part exceeded offset range. */
/*  */
#define TLR_E_MODULE_OFFSET_RANGE_ERROR  ((TLR_RESULT)0xC02B0041L)

/*  */
/*  MessageId: TLR_E_MODULE_INVALID_ELF_HEADER */
/*  */
/*  MessageText: */
/*  */
/*   Invalid ELF header in module. */
/*  */
#define TLR_E_MODULE_INVALID_ELF_HEADER  ((TLR_RESULT)0xC02B0042L)

/*  */
/*  MessageId: TLR_E_MODULE_INVALID_ELF_SECTION_REFERENCE */
/*  */
/*  MessageText: */
/*  */
/*   Invalid ELF section reference in module. */
/*  */
#define TLR_E_MODULE_INVALID_ELF_SECTION_REFERENCE ((TLR_RESULT)0xC02B0043L)

/*  */
/*  MessageId: TLR_E_MODULE_INVALID_ELF_SYMBOL_REFERENCE */
/*  */
/*  MessageText: */
/*  */
/*   Invalid ELF symbol reference in module. */
/*  */
#define TLR_E_MODULE_INVALID_ELF_SYMBOL_REFERENCE ((TLR_RESULT)0xC02B0044L)

/*  */
/*  MessageId: TLR_E_MODULE_CONTAINS_AN_UNDEFINED_SYMBOL */
/*  */
/*  MessageText: */
/*  */
/*   Module contains an undefined symbol. */
/*  */
#define TLR_E_MODULE_CONTAINS_AN_UNDEFINED_SYMBOL ((TLR_RESULT)0xC02B0045L)

/*  */
/*  MessageId: TLR_E_MODULE_CONTAINS_INVALID_CODE_SYMBOL */
/*  */
/*  MessageText: */
/*  */
/*   Module contains invalid symbol to code area. */
/*  */
#define TLR_E_MODULE_CONTAINS_INVALID_CODE_SYMBOL ((TLR_RESULT)0xC02B0046L)

/*  */
/*  MessageId: TLR_E_MODULE_CONTAINS_UNSUPPORTED_SYMBOL_BINDING */
/*  */
/*  MessageText: */
/*  */
/*   Module contains an supported symbol binding. */
/*  */
#define TLR_E_MODULE_CONTAINS_UNSUPPORTED_SYMBOL_BINDING ((TLR_RESULT)0xC02B0047L)

/*  */
/*  MessageId: TLR_E_MODULE_CONTAINS_UNSUPPORTED_SYMBOL_TYPE */
/*  */
/*  MessageText: */
/*  */
/*   Module contains an supported symbol type. */
/*  */
#define TLR_E_MODULE_CONTAINS_UNSUPPORTED_SYMBOL_TYPE ((TLR_RESULT)0xC02B0048L)

/*  */
/*  MessageId: TLR_E_MODULE_INVALID_SECTION_OFFSET_ENCOUNTERED */
/*  */
/*  MessageText: */
/*  */
/*   Invalid section offset encountered. */
/*  */
#define TLR_E_MODULE_INVALID_SECTION_OFFSET_ENCOUNTERED ((TLR_RESULT)0xC02B0049L)

/*  */
/*  MessageId: TLR_E_MODULE_UNSUPPORTED_RELOC_TYPE */
/*  */
/*  MessageText: */
/*  */
/*   Unsupported reloc type. */
/*  */
#define TLR_E_MODULE_UNSUPPORTED_RELOC_TYPE ((TLR_RESULT)0xC02B004AL)

/*  */
/*  MessageId: TLR_E_MODULE_RELOC_DISTANCE_TOO_LONG */
/*  */
/*  MessageText: */
/*  */
/*   Reloc distance too long. */
/*  */
#define TLR_E_MODULE_RELOC_DISTANCE_TOO_LONG ((TLR_RESULT)0xC02B004BL)

/*  */
/*  MessageId: TLR_E_MODULE_RELOC_ERROR */
/*  */
/*  MessageText: */
/*  */
/*   Reloc error. */
/*  */
#define TLR_E_MODULE_RELOC_ERROR         ((TLR_RESULT)0xC02B004CL)

/*  */
/*  MessageId: TLR_E_MODULE_SHT_RELA_NOT_SUPPORTED */
/*  */
/*  MessageText: */
/*  */
/*   Rela relocs not supported. */
/*  */
#define TLR_E_MODULE_SHT_RELA_NOT_SUPPORTED ((TLR_RESULT)0xC02B004DL)

/*  */
/*  MessageId: TLR_E_MODULE_SPECIAL_SYM_PARSE_ERROR */
/*  */
/*  MessageText: */
/*  */
/*   Special syms could not be parsed. */
/*  */
#define TLR_E_MODULE_SPECIAL_SYM_PARSE_ERROR ((TLR_RESULT)0xC02B004EL)

/*  */
/*  MessageId: TLR_E_MODULE_MISSING_SPECIAL_SYMS */
/*  */
/*  MessageText: */
/*  */
/*   Missing special symbols in ELF symtab. */
/*  */
#define TLR_E_MODULE_MISSING_SPECIAL_SYMS ((TLR_RESULT)0xC02B004FL)

/*  */
/*  MessageId: TLR_E_MODULE_RCX_JUMP_TABLE_IS_SHORTER_THAN_EXPECTED */
/*  */
/*  MessageText: */
/*  */
/*   rcX Jump table is shorter than expected. */
/*  */
#define TLR_E_MODULE_RCX_JUMP_TABLE_IS_SHORTER_THAN_EXPECTED ((TLR_RESULT)0xC02B0050L)

/*  */
/*  MessageId: TLR_E_MODULE_LIBC_JUMP_TABLE_IS_SHORTER_THAN_EXPECTED */
/*  */
/*  MessageText: */
/*  */
/*   libc Jump table is shorter than expected. */
/*  */
#define TLR_E_MODULE_LIBC_JUMP_TABLE_IS_SHORTER_THAN_EXPECTED ((TLR_RESULT)0xC02B0051L)

/*  */
/*  MessageId: TLR_E_MODULE_TASK_GROUP_RANGE_DOES_NOT_MATCH_STATIC_TASK_TABLE */
/*  */
/*  MessageText: */
/*  */
/*   Task Group Range does not match static task table. */
/*  */
#define TLR_E_MODULE_TASK_GROUP_RANGE_DOES_NOT_MATCH_STATIC_TASK_TABLE ((TLR_RESULT)0xC02B0052L)

/*  */
/*  MessageId: TLR_E_MODULE_INTERRUPT_GROUP_RANGE_DOES_NOT_MATCH_INTERRUPT_TABLE */
/*  */
/*  MessageText: */
/*  */
/*   Interrupt Group Range does not match interrupt table. */
/*  */
#define TLR_E_MODULE_INTERRUPT_GROUP_RANGE_DOES_NOT_MATCH_INTERRUPT_TABLE ((TLR_RESULT)0xC02B0053L)

/*  */
/*  MessageId: TLR_E_MODULE_INTERRUPT_GROUP_TASK_RANGE_DOES_NOT_MATCH_INTERRUPT_TABLE */
/*  */
/*  MessageText: */
/*  */
/*   Interrupt Group Task-Range does not match interrupt table. */
/*  */
#define TLR_E_MODULE_INTERRUPT_GROUP_TASK_RANGE_DOES_NOT_MATCH_INTERRUPT_TABLE ((TLR_RESULT)0xC02B0054L)

/*  */
/*  MessageId: TLR_E_MODULE_LED_TAG_TOO_SHORT */
/*  */
/*  MessageText: */
/*  */
/*   LED-Tag is too short. */
/*  */
#define TLR_E_MODULE_LED_TAG_TOO_SHORT   ((TLR_RESULT)0xC02B0055L)

/*  */
/*  MessageId: TLR_E_MODULE_LED_TAG_CONTAINS_INVALID_PARAMETERS */
/*  */
/*  MessageText: */
/*  */
/*   LED-Tag contains invalid parameters. */
/*  */
#define TLR_E_MODULE_LED_TAG_CONTAINS_INVALID_PARAMETERS ((TLR_RESULT)0xC02B0056L)

/*  */
/*  MessageId: TLR_E_MODULE_CONTAINS_UNSUPPORTED_COMMON_SYMBOL */
/*  */
/*  MessageText: */
/*  */
/*   Module contains unsupported *COM* symbol. */
/*  */
#define TLR_E_MODULE_CONTAINS_UNSUPPORTED_COMMON_SYMBOL ((TLR_RESULT)0xC02B0057L)

/*  */
/*  MessageId: TLR_E_RCX_DEVICE_CLASS_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Device class in file header does not match target. */
/*  */
#define TLR_E_RCX_DEVICE_CLASS_INVALID   ((TLR_RESULT)0xC02B0058L)

/*  */
/*  MessageId: TLR_E_RCX_MFG_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Manufacturer in file header does not match target. */
/*  */
#define TLR_E_RCX_MFG_INVALID            ((TLR_RESULT)0xC02B0059L)

/*  */
/*  MessageId: TLR_E_RCX_HW_COMPATIBILITY_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Hardware compatibility index in file header does not match target. */
/*  */
#define TLR_E_RCX_HW_COMPATIBILITY_INVALID ((TLR_RESULT)0xC02B005AL)

/*  */
/*  MessageId: TLR_E_RCX_HW_OPTIONS_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   Hardware options in file header does not match target. */
/*  */
#define TLR_E_RCX_HW_OPTIONS_INVALID     ((TLR_RESULT)0xC02B005BL)

/*  */
/*  MessageId: TLR_E_RCX_SECURITY_EEPROM_ZONE_NOT_READABLE */
/*  */
/*  MessageText: */
/*  */
/*   Security Eeprom Zone is not readable. */
/*  */
#define TLR_E_RCX_SECURITY_EEPROM_ZONE_NOT_READABLE ((TLR_RESULT)0xC02B4D52L)

/*  */
/*  MessageId: TLR_E_RCX_FILE_TRANSFER_IN_USE */
/*  */
/*  MessageText: */
/*  */
/*   File Transfer in use. */
/*  */
#define TLR_E_RCX_FILE_TRANSFER_IN_USE   ((TLR_RESULT)0xC02B524CL)

/*  */
/*  MessageId: TLR_E_RCX_FILE_TRANSFER_PACKET_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   File Transfer Packet invalid. */
/*  */
#define TLR_E_RCX_FILE_TRANSFER_PACKET_INVALID ((TLR_RESULT)0xC02B4444L)

/*  */
/*  MessageId: TLR_E_RCX_FILE_TRANSFER_NOT_ACTIVE */
/*  */
/*  MessageText: */
/*  */
/*   File Transfer is not active. */
/*  */
#define TLR_E_RCX_FILE_TRANSFER_NOT_ACTIVE ((TLR_RESULT)0xC02B5342L)

/*  */
/*  MessageId: TLR_E_RCX_FILE_TRANSFER_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   File Transfer has invalid type code. */
/*  */
#define TLR_E_RCX_FILE_TRANSFER_INVALID  ((TLR_RESULT)0xC02B5257L)

/*  */
/*  MessageId: TLR_E_RCX_FILE_CRC_REPEATEDLY_WRONG */
/*  */
/*  MessageText: */
/*  */
/*   File Transfer was tried repeatedly with a wrong CRC. */
/*  */
#define TLR_E_RCX_FILE_CRC_REPEATEDLY_WRONG ((TLR_RESULT)0xC02B4352L)

/*  */
/*  MessageId: TLR_E_RCX_FILE_TRANSFER_TYPE_NOT_AVAILABLE */
/*  */
/*  MessageText: */
/*  */
/*   Transfer Type is not available. */
/*  */
#define TLR_E_RCX_FILE_TRANSFER_TYPE_NOT_AVAILABLE ((TLR_RESULT)0xC02B4353L)

/*  */
/*  MessageId: TLR_E_RCX_PATH_INVALID */
/*  */
/*  MessageText: */
/*  */
/*   File Path submitted in File Transfer was invalid. */
/*  */
#define TLR_E_RCX_PATH_INVALID           ((TLR_RESULT)0xC02B5555L)

/*  */
/*  MessageId: TLR_E_RCX_DRIVER_CFG_TABLE_INIT_FUNCTION_MISSING */
/*  */
/*  MessageText: */
/*  */
/*   Driver Configuration Table Init Function missing. */
/*  */
#define TLR_E_RCX_DRIVER_CFG_TABLE_INIT_FUNCTION_MISSING ((TLR_RESULT)0xC02BFFFFL)

/*  */
/*  MessageId: TLR_E_RCX_CONFIGURATION_LOCKED */
/*  */
/*  MessageText: */
/*  */
/*   Configuration has been locked. */
/*  */
#define TLR_E_RCX_CONFIGURATION_LOCKED   ((TLR_RESULT)0xC02B4B54L)

/*  */
/*  MessageId: TLR_E_RCX_NOT_ENOUGH_SPACE_FOR_FILE */
/*  */
/*  MessageText: */
/*  */
/*   Not enough space on volume for file. */
/*  */
#define TLR_E_RCX_NOT_ENOUGH_SPACE_FOR_FILE ((TLR_RESULT)0xC02B4242L)

/*  */
/*  MessageId: TLR_E_RCX_FORMAT_ERASE_FAILED */
/*  */
/*  MessageText: */
/*  */
/*   Error formatting / erasing volume. */
/*  */
#define TLR_E_RCX_FORMAT_ERASE_FAILED    ((TLR_RESULT)0xC02B4243L)

/*  */
/*  MessageId: TLR_E_RCX_FORMAT_VERIFY_FAILED */
/*  */
/*  MessageText: */
/*  */
/*   Error erasing sector. */
/*  */
#define TLR_E_RCX_FORMAT_VERIFY_FAILED   ((TLR_RESULT)0xC02B4244L)




#endif  /* __RCX_ERROR_H */

