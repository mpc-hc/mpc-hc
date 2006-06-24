package com.axiosys.bento4;

public interface AtomParent {
    int  getChildrenCount();
    Atom getChild(int index);
    Atom getChild(int type, int index);
}
