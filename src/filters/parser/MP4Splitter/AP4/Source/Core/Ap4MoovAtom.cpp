/*****************************************************************
|
|    AP4 - moov Atoms 
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4MoovAtom.h"
#include "Ap4TrakAtom.h"
#include "Ap4AtomFactory.h"

// ==> Start patch MPC
#include "AP4DcomAtom.h"
#include "AP4CmvdAtom.h"
#include "../../../../../../zlib/zlib.h"
// <== End patch MPC

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_MoovAtom)

/*----------------------------------------------------------------------
|   AP4_TrakAtomCollector
+---------------------------------------------------------------------*/
class AP4_TrakAtomCollector : public AP4_List<AP4_Atom>::Item::Operator
{
public:
    AP4_TrakAtomCollector(AP4_List<AP4_TrakAtom>* track_atoms) :
        m_TrakAtoms(track_atoms) {}

    AP4_Result Action(AP4_Atom* atom) const {
        if (atom->GetType() == AP4_ATOM_TYPE_TRAK) {
            AP4_TrakAtom* trak = AP4_DYNAMIC_CAST(AP4_TrakAtom, atom);
            if (trak) {
                m_TrakAtoms->Add(trak);
            }
        }
        return AP4_SUCCESS;
    }

private:
    AP4_List<AP4_TrakAtom>* m_TrakAtoms;
};

/*----------------------------------------------------------------------
|   AP4_MoovAtom::AP4_MoovAtom
+---------------------------------------------------------------------*/
AP4_MoovAtom::AP4_MoovAtom() :
    AP4_ContainerAtom(AP4_ATOM_TYPE_MOOV),
    m_TimeScale(0)
{
}

/*----------------------------------------------------------------------
|   AP4_MoovAtom::AP4_MoovAtom
+---------------------------------------------------------------------*/
AP4_MoovAtom::AP4_MoovAtom(AP4_UI32         size,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_MOOV, size, false, stream, atom_factory),
    m_TimeScale(0)
{
	// ==> Start patch MPC
	if(AP4_ContainerAtom* cmov = dynamic_cast<AP4_ContainerAtom*>(GetChild(AP4_ATOM_TYPE_CMOV)))
	{
		AP4_DcomAtom* dcom = dynamic_cast<AP4_DcomAtom*>(cmov->GetChild(AP4_ATOM_TYPE_DCOM));
		AP4_CmvdAtom* cmvd = dynamic_cast<AP4_CmvdAtom*>(cmov->GetChild(AP4_ATOM_TYPE_CMVD));
		if(dcom && dcom->GetCompressorSubType() == AP4_ATOM_TYPE('z','l','i','b') && cmvd)
		{
			const AP4_DataBuffer& data = cmvd->GetDataBuffer();

			z_stream d_stream;
			d_stream.zalloc = (alloc_func)0;
			d_stream.zfree = (free_func)0;
			d_stream.opaque = (voidpf)0;

			int res;

			if(Z_OK == (res = inflateInit(&d_stream)))
			{
				d_stream.next_in = (Bytef*)data.GetData();
				d_stream.avail_in = data.GetDataSize();

				unsigned char* dst = NULL;
				int n = 0;
				
				do
				{
					dst = (unsigned char*)realloc(dst, ++n*1000);
					d_stream.next_out = &dst[(n-1)*1000];
					d_stream.avail_out = 1000;

					if(Z_OK != (res = inflate(&d_stream, Z_NO_FLUSH)) && Z_STREAM_END != res)
					{
						free(dst);
						dst = NULL;
						break;
					}
				}
				while(0 == d_stream.avail_out && 0 != d_stream.avail_in && Z_STREAM_END != res);

				inflateEnd(&d_stream);

				if(dst)
				{
					AP4_ByteStream* s = new AP4_MemoryByteStream(dst, d_stream.total_out);
					ReadChildren(atom_factory, *s, d_stream.total_out);
					s->Release();
					free(dst);
				}

				if(AP4_MoovAtom* moov = dynamic_cast<AP4_MoovAtom*>(GetChild(AP4_ATOM_TYPE_MOOV)))
				{
					AP4_List<AP4_Atom> Children;

					for(AP4_List<AP4_Atom>::Item* item = moov->GetChildren().FirstItem(); 
						item; 
						item = item->GetNext())
					{
						Children.Add(item->GetData());
					}

					for(AP4_List<AP4_Atom>::Item* item = Children.FirstItem(); 
						item; 
						item = item->GetNext())
					{
						AP4_Atom* atom = item->GetData();
						atom->Detach();
						atom->SetParent(this);
						m_Children.Add(atom);
					}

					moov->Detach();
					delete moov;
				}
			}
		}
	}
	// <== End patch MPC

	// collect all trak atoms
    m_Children.Apply(AP4_TrakAtomCollector(&m_TrakAtoms));    
}

/*----------------------------------------------------------------------
|   AP4_MoovAtom::AdjustChunkOffsets
+---------------------------------------------------------------------*/
AP4_Result 
AP4_MoovAtom::AdjustChunkOffsets(AP4_SI64 offset)
{
    for (AP4_List<AP4_TrakAtom>::Item* item = m_TrakAtoms.FirstItem();
         item;
         item = item->GetNext()) {
        AP4_TrakAtom* trak = item->GetData();
        trak->AdjustChunkOffsets(offset);
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MoovAtom::OnChildAdded
+---------------------------------------------------------------------*/
void
AP4_MoovAtom::OnChildAdded(AP4_Atom* atom)
{
    // keep the atom in the list of trak atoms
    if (atom->GetType() == AP4_ATOM_TYPE_TRAK) {
        AP4_TrakAtom* trak = AP4_DYNAMIC_CAST(AP4_TrakAtom, atom);
        if (trak) {
            m_TrakAtoms.Add(trak);
        }
    }

    // call the base class implementation
    AP4_ContainerAtom::OnChildAdded(atom);
}

/*----------------------------------------------------------------------
|   AP4_MoovAtom::OnChildRemoved
+---------------------------------------------------------------------*/
void
AP4_MoovAtom::OnChildRemoved(AP4_Atom* atom)
{
    // remove the atom from the list of trak atoms
    if (atom->GetType() == AP4_ATOM_TYPE_TRAK) {
        AP4_TrakAtom* trak = AP4_DYNAMIC_CAST(AP4_TrakAtom, atom);
        if (trak) {
            m_TrakAtoms.Remove(trak);
        }
    }

    // call the base class implementation
    AP4_ContainerAtom::OnChildRemoved(atom);
}
