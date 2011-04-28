/*
 * lib/krb5/error_tables/krb5_err.c:
 * This file is automatically generated; please do not edit it.
 */
#if defined(_WIN32)
# include "win-mac.h"
#endif

#if !defined(_WIN32)
extern void initialize_krb5_error_table (void);
#endif

/* Lclint doesn't handle null annotations on arrays
   properly, so we need this typedef in each
   generated .c file.  */
/*@-redef@*/
typedef /*@null@*/ const char *ncptr;
/*@=redef@*/

static ncptr const text[] = {
	"No error",
	"Client's entry in database has expired",
	"Server's entry in database has expired",
	"Requested protocol version not supported",
	"Client's key is encrypted in an old master key",
	"Server's key is encrypted in an old master key",
	"Client not found in Kerberos database",
	"Server not found in Kerberos database",
	"Principal has multiple entries in Kerberos database",
	"Client or server has a null key",
	"Ticket is ineligible for postdating",
	"Requested effective lifetime is negative or too short",
	"KDC policy rejects request",
	"KDC can't fulfill requested option",
	"KDC has no support for encryption type",
	"KDC has no support for checksum type",
	"KDC has no support for padata type",
	"KDC has no support for transited type",
	"Clients credentials have been revoked",
	"Credentials for server have been revoked",
	"TGT has been revoked",
	"Client not yet valid - try again later",
	"Server not yet valid - try again later",
	"Password has expired",
	"Preauthentication failed",
	"Additional pre-authentication required",
	"Requested server and ticket don't match",
	"KRB5 error code 27",
	"KRB5 error code 28",
	"KRB5 error code 29",
	"KRB5 error code 30",
	"Decrypt integrity check failed",
	"Ticket expired",
	"Ticket not yet valid",
	"Request is a replay",
	"The ticket isn't for us",
	"Ticket/authenticator don't match",
	"Clock skew too great",
	"Incorrect net address",
	"Protocol version mismatch",
	"Invalid message type",
	"Message stream modified",
	"Message out of order",
	"Illegal cross-realm ticket",
	"Key version is not available",
	"Service key not available",
	"Mutual authentication failed",
	"Incorrect message direction",
	"Alternative authentication method required",
	"Incorrect sequence number in message",
	"Inappropriate type of checksum in message",
	"Policy rejects transited path",
	"Response too big for UDP, retry with TCP",
	"KRB5 error code 53",
	"KRB5 error code 54",
	"KRB5 error code 55",
	"KRB5 error code 56",
	"KRB5 error code 57",
	"KRB5 error code 58",
	"KRB5 error code 59",
	"Generic error (see e-text)",
	"Field is too long for this implementation",
	"KRB5 error code 62",
	"KRB5 error code 63",
	"KRB5 error code 64",
	"KRB5 error code 65",
	"KRB5 error code 66",
	"KRB5 error code 67",
	"KRB5 error code 68",
	"KRB5 error code 69",
	"KRB5 error code 70",
	"KRB5 error code 71",
	"KRB5 error code 72",
	"KRB5 error code 73",
	"KRB5 error code 74",
	"KRB5 error code 75",
	"KRB5 error code 76",
	"KRB5 error code 77",
	"KRB5 error code 78",
	"KRB5 error code 79",
	"KRB5 error code 80",
	"KRB5 error code 81",
	"KRB5 error code 82",
	"KRB5 error code 83",
	"KRB5 error code 84",
	"KRB5 error code 85",
	"KRB5 error code 86",
	"KRB5 error code 87",
	"KRB5 error code 88",
	"KRB5 error code 89",
	"KRB5 error code 90",
	"KRB5 error code 91",
	"KRB5 error code 92",
	"KRB5 error code 93",
	"KRB5 error code 94",
	"KRB5 error code 95",
	"KRB5 error code 96",
	"KRB5 error code 97",
	"KRB5 error code 98",
	"KRB5 error code 99",
	"KRB5 error code 100",
	"KRB5 error code 101",
	"KRB5 error code 102",
	"KRB5 error code 103",
	"KRB5 error code 104",
	"KRB5 error code 105",
	"KRB5 error code 106",
	"KRB5 error code 107",
	"KRB5 error code 108",
	"KRB5 error code 109",
	"KRB5 error code 110",
	"KRB5 error code 111",
	"KRB5 error code 112",
	"KRB5 error code 113",
	"KRB5 error code 114",
	"KRB5 error code 115",
	"KRB5 error code 116",
	"KRB5 error code 117",
	"KRB5 error code 118",
	"KRB5 error code 119",
	"KRB5 error code 120",
	"KRB5 error code 121",
	"KRB5 error code 122",
	"KRB5 error code 123",
	"KRB5 error code 124",
	"KRB5 error code 125",
	"KRB5 error code 126",
	"KRB5 error code 127",
	"$Id: krb5_err.et,v 5.72.2.1 2003/06/06 21:00:59 tlyu Exp $",
	"Invalid flag for file lock mode",
	"Cannot read password",
	"Password mismatch",
	"Password read interrupted",
	"Illegal character in component name",
	"Malformed representation of principal",
	"Can't open/find Kerberos configuration file",
	"Improper format of Kerberos configuration file",
	"Insufficient space to return complete information",
	"Invalid message type specified for encoding",
	"Credential cache name malformed",
	"Unknown credential cache type",
	"Matching credential not found",
	"End of credential cache reached",
	"Request did not supply a ticket",
	"Wrong principal in request",
	"Ticket has invalid flag set",
	"Requested principal and ticket don't match",
	"KDC reply did not match expectations",
	"Clock skew too great in KDC reply",
	"Client/server realm mismatch in initial ticket request",
	"Program lacks support for encryption type",
	"Program lacks support for key type",
	"Requested encryption type not used in message",
	"Program lacks support for checksum type",
	"Cannot find KDC for requested realm",
	"Kerberos service unknown",
	"Cannot contact any KDC for requested realm",
	"No local name found for principal name",
	"Mutual authentication failed",
	"Replay cache type is already registered",
	"No more memory to allocate (in replay cache code)",
	"Replay cache type is unknown",
	"Generic unknown RC error",
	"Message is a replay",
	"Replay I/O operation failed XXX",
	"Replay cache type does not support non-volatile storage",
	"Replay cache name parse/format error",
	"End-of-file on replay cache I/O",
	"No more memory to allocate (in replay cache I/O code)",
	"Permission denied in replay cache code",
	"I/O error in replay cache i/o code",
	"Generic unknown RC/IO error",
	"Insufficient system space to store replay information",
	"Can't open/find realm translation file",
	"Improper format of realm translation file",
	"Can't open/find lname translation database",
	"No translation available for requested principal",
	"Improper format of translation database entry",
	"Cryptosystem internal error",
	"Key table name malformed",
	"Unknown Key table type",
	"Key table entry not found",
	"End of key table reached",
	"Cannot write to specified key table",
	"Error writing to key table",
	"Cannot find ticket for requested realm",
	"DES key has bad parity",
	"DES key is a weak key",
	"Bad encryption type",
	"Key size is incompatible with encryption type",
	"Message size is incompatible with encryption type",
	"Credentials cache type is already registered.",
	"Key table type is already registered.",
	"Credentials cache I/O operation failed XXX",
	"Credentials cache permissions incorrect",
	"No credentials cache found",
	"Internal credentials cache error",
	"Error writing to credentials cache",
	"No more memory to allocate (in credentials cache code)",
	"Bad format in credentials cache",
	"No credentials found with supported encryption types",
	"Invalid KDC option combination (library internal error)",
	"Request missing second ticket",
	"No credentials supplied to library routine",
	"Bad sendauth version was sent",
	"Bad application version was sent (via sendauth)",
	"Bad response (during sendauth exchange)",
	"Server rejected authentication (during sendauth exchange)",
	"Unsupported preauthentication type",
	"Required preauthentication key not supplied",
	"Generic preauthentication failure",
	"Unsupported replay cache format version number",
	"Unsupported credentials cache format version number",
	"Unsupported key table format version number",
	"Program lacks support for address type",
	"Message replay detection requires rcache parameter",
	"Hostname cannot be canonicalized",
	"Cannot determine realm for host",
	"Conversion to service principal undefined for name type",
	"Initial Ticket response appears to be Version 4 error",
	"Cannot resolve network address for KDC in requested realm",
	"Requesting ticket can't get forwardable tickets",
	"Bad principal name while trying to forward credentials",
	"Looping detected inside krb5_get_in_tkt",
	"Configuration file does not specify default realm",
	"Bad SAM flags in obtain_sam_padata",
	"Invalid encryption type in SAM challenge",
	"Missing checksum in SAM challenge",
	"Bad checksum in SAM challenge",
	"Keytab name too long",
	"Key version number for principal in key table is incorrect",
	"This application has expired",
	"This Krb5 library has expired",
	"New password cannot be zero length",
	"Password change failed",
	"Bad format in keytab",
	"Encryption type not permitted",
	"No supported encryption types (config file error?)",
	"Program called an obsolete, deleted function",
	"unknown getaddrinfo failure",
	"no data available for host/domain name",
	"host/domain name not found",
	"service name unknown",
	"Cannot determine realm for numeric host address",
	"Invalid key generation parameters from KDC",
	"service not available",
    0
};

#include <com_err.h>

const struct error_table et_krb5_error_table = { text, -1765328384L, 246 };

#if !defined(_WIN32)
void initialize_krb5_error_table (void)
    /*@modifies internalState@*/
{
    (void) add_error_table (&et_krb5_error_table);
}
#endif
