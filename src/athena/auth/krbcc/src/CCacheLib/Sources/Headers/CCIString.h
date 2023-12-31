/*
 * CCICache.h
 *
 * $Header: /cvs/pismere/pismere/athena/auth/krbcc/src/CCacheLib/Sources/Headers/CCIString.h,v 1.4 2000/06/16 21:44:08 dalmeida Exp $
 */
 
#pragma once

#include "CCache.h"

#include "CCIUniqueInProcess.h"
#include "CCIMagic.h"
#include "CCILeaks.h"
#include "CCIInternal.h"
#include "CCIInternalize.h"

#include <vector>
#include <string>

class CCEString {
	public:
		static cc_int32 Release (
			cc_string_t	inString);
			
	private:
		// Disallowed
		CCEString ();
		CCEString (const CCEString&);
		CCEString& operator = (const CCEString&);
};

class CCIString:
	public CCIUniqueInProcess <CCIString>,
	public CCIMagic <CCIString>,
	public CCILeaks <CCIString>,
	public CCIInternal <CCIString, cc_string_d> {

	public:
		CCIString (
			const std::string&		inString);
			
		~CCIString ();
			
		enum {
			class_ID = FOUR_CHAR_CODE ('CCSt'),
			invalidObject = ccErrInvalidString
		};
		
	private:
		std::string			mString;
		
		void		Validate ();

		static const	cc_string_f	sFunctionTable;

		friend class StInternalize <CCIString, cc_string_d>;
		friend class CCIInternal <CCIString, cc_string_d>;

		// Disallowed
		CCIString ();
		CCIString (const CCIString&);
		CCIString& operator = (const CCIString&);
};

typedef StInternalize <CCIString, cc_string_d>	StString;

