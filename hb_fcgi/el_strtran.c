#ifndef USING_HB_EL

/*
	* StrTran() function
	*
	* Copyright 1999 Antonio Linares <alinares@fivetech.com>
	* Copyright 2011 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
	* rewritten to fix incompatibilities with Clipper and fatal performance
	* of original code
	* Copyright 2020 Eric Lendvai USA
	* Added support for case insensitive replaces via an extra parameter.
	* Especially useful for html tags. Is similar to VFP9 behavior.
	*
	* This program is free software; you can redistribute it and/or modify
	* it under the terms of the GNU General Public License as published by
	* the Free Software Foundation; either version 2, or (at your option)
	* any later version.
	*
	* This program is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	* GNU General Public License for more details.
	*
	* You should have received a copy of the GNU General Public License
	* along with this program; see the file LICENSE.txt.  If not, write to
	* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	* Boston, MA 02110-1301 USA (or visit https://www.gnu.org/licenses/).
	*
	* As a special exception, the Harbour Project gives permission for
	* additional uses of the text contained in its release of Harbour.
	*
	* The exception is that, if you link the Harbour libraries with other
	* files to produce an executable, this does not by itself cause the
	* resulting executable to be covered by the GNU General Public License.
	* Your use of that executable is in no way restricted on account of
	* linking the Harbour library code into it.
	*
	* This exception does not however invalidate any other reasons why
	* the executable file might be covered by the GNU General Public License.
	*
	* This exception applies only to the code released by the Harbour
	* Project under the name Harbour.  If you copy code from other
	* Harbour Project or Free Software Foundation releases into a copy of
	* Harbour, as the General Public License permits, the exception does
	* not apply to the code that you add in this way.  To avoid misleading
	* anyone as to the status of such modified files, you must delete
	* this exception notice from them.
	*
	* If you write modifications of your own for Harbour, it is your choice
	* whether to permit this exception to apply to your modifications.
	* If you do not wish that, delete this exception notice.
	*
	*/

#include <ctype.h>  // Needed for toupper()

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"

/* FIXME: Check for string overflow, Clipper can crash if the resulting
				string is too large. Example:
				StrTran( "...", ".", Replicate( "A", 32000 ) ) [vszakats] */

HB_FUNC( EL_STRTRAN )
{
	PHB_ITEM pText = hb_param( 1, HB_IT_STRING );
	PHB_ITEM pSeek = hb_param( 2, HB_IT_STRING );

	if( pText && pSeek )
	{
        HB_ISIZ  nStart, nCount, nFlag;
        int iCaseSensitiveTest;

		nStart = hb_parnsdef( 4, 1 );
		nCount = hb_parnsdef( 5, -1 );
        nFlag  = hb_parnsdef( 6, 0 );

        if( nStart <= 0 ) nStart = 1; //in VFP9 ,missing parameters values are not allowed, instead user must use -1 as a value.

        // nFlag Only implemented the nFlag values of -1,0 and 1 to match the syntax from VFP9. Value 2 and 3 never worked properly in VFP9
        // 1 = Search is case-insensitive and replacement is performed with exact Replacement text. 
        // Any Other values = Search is case-sensitive and replacement is performed with exact Replacement text. 

        iCaseSensitiveTest    = ( nFlag == 1 ? 0 : 1);

		if( nStart && nCount )
		{
			HB_SIZE nText = hb_itemGetCLen( pText );
			HB_SIZE nSeek = hb_itemGetCLen( pSeek );

			if( nSeek && nSeek <= nText && nStart > 0 )
			{
				PHB_ITEM pReplace = hb_param( 3, HB_IT_STRING );
				HB_SIZE nReplace = hb_itemGetCLen( pReplace );
				const char * szReplace = hb_itemGetCPtr( pReplace );
				const char * szText = hb_itemGetCPtr( pText );
				const char * szSeek = hb_itemGetCPtr( pSeek );
				HB_SIZE nFound = 0;
				HB_SIZE nReplaced = 0;
				HB_SIZE nT = 0;
				HB_SIZE nS = 0;

				while( nT < nText && nText - nT >= nSeek - nS )
				{
					if( ( iCaseSensitiveTest ? szText[ nT ] == szSeek[ nS ] : toupper(szText[ nT ]) == toupper(szSeek[ nS ]) ) )
					{
						++nT;
						if( ++nS == nSeek )
						{
							if( ++nFound >= (HB_SIZE) nStart )
							{
								nReplaced++;
								if( --nCount == 0 )
									nT = nText;
							}
							nS = 0;
						}
					}
					else if( nS )
					{
						nT -= nS - 1;
						nS = 0;
					}
					else
						++nT;
				}

				if( nReplaced )
				{
					HB_SIZE nLength = nText;

					if( nSeek > nReplace )
						nLength -= ( nSeek - nReplace ) * nReplaced;
					else
						nLength += ( nReplace - nSeek ) * nReplaced;

					if( nLength )
					{
						char * szResult = ( char * ) hb_xgrab( nLength + 1 );
						char * szPtr = szResult;

						nFound -= nReplaced;
						nT = nS = 0;
						do
						{
							if( nReplaced && ( iCaseSensitiveTest ? szText[ nT ] == szSeek[ nS ] : toupper( szText[ nT ] ) == toupper( szSeek[ nS ] ) ) )
							{
								++nT;
								if( ++nS == nSeek )
								{
									const char * szCopy;

									if( nFound )
									{
										nFound--;
										szCopy = szSeek;
									}
									else
									{
										nReplaced--;
										szCopy = szReplace;
										nS = nReplace;
									}
									while( nS )
									{
										*szPtr++ = *szCopy++;
										--nS;
									}
								}
							}
							else
							{
								if( nS )
								{
									nT -= nS;
									nS = 0;
								}
								*szPtr++ = szText[ nT++ ];
							}
						}
						while( nT < nText );

						hb_retclen_buffer( szResult, nLength );
					}
					else
						hb_retc_null();
				}
				else
					hb_itemReturn( pText );
			}
			else
				hb_itemReturn( pText );
		}
		else
			hb_retc_null();
	}
	else
	{
		/* NOTE: Undocumented but existing Clipper Run-time error [vszakats] */
#ifdef HB_CLP_STRICT
		hb_errRT_BASE_SubstR( EG_ARG, 1126, NULL, HB_ERR_FUNCNAME, 0 );
#else
		hb_errRT_BASE_SubstR( EG_ARG, 1126, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
#endif
	}
}

#endif //USING_HB_EL
