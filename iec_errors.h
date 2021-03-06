#ifndef IEC_ERRS_H
#define IEC_ERRS_H

#define ERR_OK								00
#define ERR_FILES_SCRATCHED					01
#define ERR_PARTITION_OK					02
#define ERR_READ_ERROR						20
#define ERR_READ_ERROR_DATA_BLK_NOT_FOUND	22
#define ERR_READ_ERROR_CRC_ERROR			23
#define ERR_WRITE_ERROR						25
#define ERR_WRITE_PROTECT_ON				26
#define ERR_DISK_ID_MISMATCH				29
#define ERR_SYNTAX_ERROR_GEN				30
#define ERR_SYNTAX_ERROR_CMD				31
#define ERR_SYNTAX_ERROR_CMDLENGTH			32
#define ERR_SYNTAX_ERROR_NAME				33
#define ERR_SYNTAX_ERROR_NONAME				34
#define ERR_SYNTAX_ERROR_CMD15				39
#define ERR_RECORD_NOT_PRESENT				50
#define ERR_OVERFLOW_IN_RECORD				51
#define ERR_FILE_TOO_LARGE					52
#define ERR_WRITE_FILE_OPEN					60
#define ERR_FILE_NOT_OPEN					61
#define ERR_FILE_NOT_FOUND					62
#define ERR_FILE_EXISTS						63
#define ERR_FILE_TYPE_MISMATCH				64
#define ERR_NO_BLOCK						65
#define ERR_ILLEGAL_T_S						66
#define ERR_ILLEGAL_SYSTEM_T_S				67
#define ERR_FRESULT_CODE					69
#define ERR_NO_CHANNEL          			70
#define ERR_DIRECTORY_ERROR					71
#define ERR_DISK_FULL						72
#define ERR_DOS								73
#define ERR_DRIVE_NOT_READY					74
// custom
#define ERR_BAD_COMMAND						75
#define ERR_UNIMPLEMENTED					76
#define ERR_PARTITION_ERROR					77

#endif // !IEC_ERRS_H