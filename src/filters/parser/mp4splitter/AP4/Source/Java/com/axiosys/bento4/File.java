package com.axiosys.bento4;

import java.io.IOException;

public class File {
    private AtomList atoms;
    private Movie    movie;
    
    public File(String filename) throws IOException, InvalidFormatException {
        atoms = new AtomList(filename);
        ContainerAtom moov = (ContainerAtom)atoms.getChild(Atom.TYPE_MOOV, 0);
        if (moov == null) {
            movie = null;
        } else {
            movie = new Movie(moov);
        }
    }
    
    public Movie getMovie() {
        return movie;
    }
}
