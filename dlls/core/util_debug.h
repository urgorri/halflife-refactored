#pragma once

#ifdef DEBUG
extern edict_t *DBG_EntOfVars( const entvars_t *pev );
inline edict_t *ENT( const entvars_t *pev )
{
	return DBG_EntOfVars( pev );
}
#else
inline edict_t *ENT( const entvars_t *pev )
{
	return pev->pContainingEntity;
}
#endif

#ifdef DEBUG
void DBG_AssertFunction( BOOL fExpr, const char *szExpr, const char *szFile, int szLine, const char *szMessage );
#define ASSERT( f ) DBG_AssertFunction( f, #f, __FILE__, __LINE__, NULL )
#define ASSERTSZ( f, sz ) DBG_AssertFunction( f, #f, __FILE__, __LINE__, sz )
#else // !DEBUG
#define ASSERT( f )
#define ASSERTSZ( f, sz )
#endif // !DEBUG

extern void UTIL_LogPrintf( char *fmt, ... );
