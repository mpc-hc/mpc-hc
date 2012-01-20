/*****************************************************************
|
|    AP4 - enda Atom
|
 ****************************************************************/

#ifndef _AP4_ENDA_ATOM_H_
#define _AP4_ENDA_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Atom.h"
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|       AP4_EndaAtom
+---------------------------------------------------------------------*/
class AP4_EndaAtom : public AP4_Atom
{
public:
	AP4_EndaAtom(AP4_Size         size,
                 AP4_ByteStream&  stream);

    AP4_Result WriteFields(AP4_ByteStream& stream) { return AP4_FAILURE; }

    AP4_Atom::Type ReadEndian() const { return m_LittleEndian; }

private:
	AP4_UI16 m_LittleEndian;
};

#endif // _AP4_ENDA_ATOM_H_
