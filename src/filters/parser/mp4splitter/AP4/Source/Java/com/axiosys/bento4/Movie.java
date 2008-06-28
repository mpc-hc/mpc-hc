package com.axiosys.bento4;

import java.util.ArrayList;

public class Movie {
    private ContainerAtom moov;
    private ArrayList     tracks = new ArrayList();
    
    public Movie(ContainerAtom moov) {
        this.moov = moov;
        
        for (int i=0; i<moov.getChildrenCount(); i++) {
            Atom child = moov.getChild(i);
            if (child.getType() == Atom.TYPE_TRAK) {
                TrakAtom trak = (TrakAtom)child;
                tracks.add(new Track(trak));
            }
        }
    }
    
    public Atom findAtom(String path) {
        return AtomUtils.findAtom(moov, path);
    }
    
    public Track[] getTracks() {
        Track[] result = new Track[tracks.size()];
        tracks.toArray(result);
        return result;
    }
    
    public int[] getTrackIds() {
        int[] result = new int[tracks.size()];
        for (int i=0; i<result.length; i++) {
            Track track = (Track)tracks.get(i);
            result[i] = track.getId();
        }
        
        return result;
    }
    
    public Track getTrackById(int id) {
        for (int i=0; i<tracks.size(); i++) {
            Track track = (Track)tracks.get(i);
            if (track.getId() == id) return track;
        }
        
        return null;
    }

    public int getTrackIndex(int id) {
        for (int i=0; i<tracks.size(); i++) {
            Track track = (Track)tracks.get(i);
            if (track.getId() == id) return i;
        }
        
        return -1;
    }
}
