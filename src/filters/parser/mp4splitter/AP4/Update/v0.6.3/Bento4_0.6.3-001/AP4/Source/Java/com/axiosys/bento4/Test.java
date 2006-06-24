package com.axiosys.bento4;

import java.io.IOException;

public class Test {

    /**
     * @param args
     * @throws IOException 
     * @throws InvalidFormatException 
     */
    public static void main(String[] args) throws IOException, InvalidFormatException {
        AtomList atoms = new AtomList(args[0]);
        
        if (args.length > 1) {
            Atom atom = AtomUtils.findAtom(atoms, args[1]);
            if (atom != null) {
                System.out.println(atom);
                byte[] payload = atom.getPayload();
                System.out.println(new String(payload));
            }
        } else {
            System.out.println(atoms);
        }
    }
}
