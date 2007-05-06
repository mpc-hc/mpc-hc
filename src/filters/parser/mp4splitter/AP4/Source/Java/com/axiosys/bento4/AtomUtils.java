package com.axiosys.bento4;

import java.util.Iterator;
import java.util.List;

public class AtomUtils {
    public static Atom findAtom(AtomParent parent, String path) {
        Atom atom = null;
        while (path != null) {
            int separator = path.indexOf('/');
            String atomName;
            int index = 0;
            if (separator > 0) {
                atomName = path.substring(0, separator);
                path = path.substring(separator+1);
            } else {
                atomName = path;
                path = null;
            }
            
            if (atomName.length() != 4) {
                // we need at least 3 more chars
                if (atomName.length() < 7) return null;
                
                // parse the name trailer
                if (atomName.charAt(4) != '[' || atomName.charAt(atomName.length()-1) != ']') {
                    return null;
                }
                String indexString = atomName.substring(5, atomName.length()-1);
                index = Integer.parseInt(indexString);
            }
            
            int type = Atom.nameToType(atomName);
            atom = parent.getChild(type, index);
            if (path == null) return atom;
            if (atom instanceof AtomParent) {
                parent = (AtomParent)atom;
            } else {
                return null;
            }
        }
        
        return atom;
    }

    public static Atom findChild(List atoms, int type, int index) {
        for (Iterator i = atoms.iterator(); i.hasNext();) {
            Atom atom = (Atom)i.next();
            if (atom.getType() == type) {
                if (index-- == 0) return atom;
            }
        }
        
        return null;
    }
}
