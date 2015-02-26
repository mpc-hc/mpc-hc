/*************************************************************************
  vStrip by [maven] (maven@maven.de)
    udf.c:  routines for udf-parsing (because windows just doesn't cut it),
                    refs: udf102.pdf, udf200.pdf
*************************************************************************/

#include "stdafx.h"
#include "udf.h"

static bool aspi_GetSectorInfo(const HANDLE hDrive, DWORD* sec_size, DWORD* max_sec)
{
    LARGE_INTEGER size = {0, 0};
    GetFileSizeEx(hDrive, &size);

    *sec_size = 2048;
    *max_sec = (DWORD)(size.QuadPart / *sec_size);

    return true;
}

static bool aspi_ReadSectors(const HANDLE hDrive, int lba, int nSectors, DWORD sec_size, BYTE* sector)
{
    DWORD nbr = 0;
    LARGE_INTEGER offset;
    offset.QuadPart = lba*sec_size;
    SetFilePointerEx(hDrive, offset, &offset, FILE_BEGIN);
    return lba*sec_size == offset.QuadPart && ReadFile(hDrive, sector, nSectors*sec_size, &nbr, nullptr);
}

static bool udf_GetLBA(const tp_udf_FileEntry fe, const DWORD sec_size, DWORD *start, DWORD *end)
{
    if (fe->LengthofAllocationDescriptors == 0) {
        return false;
    }
    switch (fe->ICBTag.Flags & udf_icbf_Mask) {
        case udf_icbf_ShortAd: {
            tp_udf_short_ad ad = (tp_udf_short_ad)(fe->ExtendedAttributes + fe->LengthofExtendedAttributes);

            *start = ad->Location;
            *end = *start + ((ad->Length & udf_LengthMask) - 1) / sec_size;
            return true;
        }
        break;
        case udf_icbf_LongAd: {
            tp_udf_long_ad ad = (tp_udf_long_ad)(fe->ExtendedAttributes + fe->LengthofExtendedAttributes);

            *start = ad->Location.Location; // ignore partition number
            *end = *start + ((ad->Length & udf_LengthMask) - 1) / sec_size;
            return true;
        }
        break;
        case udf_icbf_ExtAd: {
            tp_udf_ext_ad ad = (tp_udf_ext_ad)(fe->ExtendedAttributes + fe->LengthofExtendedAttributes);

            *start = ad->Location.Location; // ignore partition number
            *end = *start + ((ad->Length & udf_LengthMask) - 1) / sec_size;
            return true;
        }
        break;
    }
    return false;
}

tp_udf_file udf_get_root(const HANDLE hDrive, const WORD partition_number)
{
    BYTE sector[fio_SECTOR_SIZE];
    tp_udf_tag tag = (tp_udf_tag)sector;
    DWORD sec_size, max_sec, i;
    DWORD MVDS_lba, MVDS_lba_end, MVDS_back_lba, MVDS_back_lba_end;
    DWORD FileDescriptorSequence_lba = 0;
    DWORD partition_lba = 0;
    tp_udf_AnchorVolumeDescriptorPointer avd;
    bool res, part_valid, vol_valid;

    if (!aspi_GetSectorInfo(hDrive, &sec_size, &max_sec)) {
        return nullptr;
    }

    if (sec_size != fio_SECTOR_SIZE || max_sec < 256) {
        return nullptr;
    }

    // read AnchorVolumeDescriptorPointer at 256 (or MaxSec) (Tag == 2)
    res = aspi_ReadSectors(hDrive, 256, 1, sec_size, sector);
    if (!res || tag->TagIdentifier != udf_TAG_AnchorVolumeDescriptor) {
        res = aspi_ReadSectors(hDrive, max_sec, 1, sec_size, sector);
        if (!res || tag->TagIdentifier != udf_TAG_AnchorVolumeDescriptor) {
            return nullptr;
        }
    }

    // check Static Structures

    // get MainVolumeDescriptorSequence Location & Length
    avd = (tp_udf_AnchorVolumeDescriptorPointer)sector;
    MVDS_lba = avd->MainVolumeDescriptorSequenceExtent.Location;
    MVDS_lba_end = MVDS_lba + (avd->MainVolumeDescriptorSequenceExtent.Length - 1) / sec_size;
    MVDS_back_lba = avd->ReserveVolumeDescriptorSequenceExtent.Location;
    MVDS_back_lba_end = MVDS_back_lba + (avd->ReserveVolumeDescriptorSequenceExtent.Length - 1) / sec_size;

    // read MVDS_Location..MVDS_Location + (MVDS_Length - 1) / SectorSize sectors

    part_valid = vol_valid = false;
    i = 1;
    do {
        // try twice (if we need to) for ReserveAnchor
        DWORD j = MVDS_lba;
        do {
            res = aspi_ReadSectors(hDrive, j++, 1, sec_size, sector);
            if (res) {
                if (tag->TagIdentifier == udf_TAG_PartitionDescriptor && !part_valid) {
                    // get stuff out of partition
                    tp_udf_PartitionDescriptor par = (tp_udf_PartitionDescriptor )sector;

                    part_valid = par->PartitionNumber == partition_number;
                    if (part_valid) {
                        // extract par->PartitionStartingLocation, par->PartitionLength
                        partition_lba = par->PartitionStartingLocation;
                    }
                } else if (tag->TagIdentifier == udf_TAG_LogicalVolumeDescriptor && !vol_valid) {
                    // get stuff out of volume
                    tp_udf_LogicalVolumeDescriptor vol = (tp_udf_LogicalVolumeDescriptor)sector;

                    // check_volume sector size
                    vol_valid = (vol->LogicalBlockSize == sec_size) && (partition_number == vol->FileSetDescriptorSequence.Location.PartitionNumber);
                    if (vol_valid) {
                        // extract vol->FileSetDescriptorSequence
                        FileDescriptorSequence_lba = vol->FileSetDescriptorSequence.Location.Location;
                        // DWORD FileDescriptorSequence_lba_end = FileDescriptorSequence_lba + ((vol->FileSetDescriptorSequence.Length & udf_LengthMask) - 1) / sec_size;
                    }
                }
            } else {
                tag->TagIdentifier = 0;
            }
        } while (j <= MVDS_lba_end && tag->TagIdentifier != udf_TAG_TerminatingDescriptor && ((!part_valid) || (!vol_valid)));

        if ((!part_valid) || (!vol_valid)) {
            // try backup
            MVDS_lba = MVDS_back_lba;
            MVDS_lba_end = MVDS_back_lba_end;
        }
    } while (i-- && ((!part_valid) || (!vol_valid)));

    if (part_valid && vol_valid) {
        // read FileSetDescriptor, get RootDir Location & Length, RootDir Length != 0
        res = aspi_ReadSectors(hDrive, FileDescriptorSequence_lba + partition_lba, 1, sec_size, sector);
        if (res && tag->TagIdentifier == udf_TAG_FileSetDescriptor) {
            tp_udf_FileSetDescriptor fsd = (tp_udf_FileSetDescriptor)sector;

            if (partition_number == fsd->RootDirectoryICB.Location.PartitionNumber) {
                DWORD parent_icb = fsd->RootDirectoryICB.Location.Location;
                res = aspi_ReadSectors(hDrive, partition_lba + parent_icb, 1, sec_size, sector);
                if (res && tag->TagIdentifier == udf_TAG_FileEntry) {
                    tp_udf_FileEntry fe = (tp_udf_FileEntry)sector;

                    if (fe->ICBTag.FileType == udf_FT_Directory) {
                        tp_udf_file root = (tp_udf_file)malloc(sizeof *root);

                        root->partition_lba = partition_lba;
                        udf_GetLBA(fe, sec_size, &root->dir_lba, &root->dir_end_lba);
                        root->dir_left = (DWORD)fe->InformationLength; // don't want directories of more than 4gb
                        root->sector = nullptr;
                        root->fid = nullptr;
                        root->sec_size = sec_size;
                        strcpy_s(root->name, "/");
                        root->is_dir = true;
                        root->is_parent = false;
                        return root;
                    }
                }
            }
        }
    }

    return nullptr;
}

static void udf_GetName(const BYTE *data, const DWORD len, char *target)
{
    DWORD p = 1, i = 0;

    if (len == 0 || !(data[0] & 0x18)) {
        target[0] = '\0';
    }

    if (data[0] & 0x10) {
        // ignore MSB of unicode16
        p++;

        while (p < len) {
            target[i++] = data[p += 2];
        }
    } else {
        while (p < len) {
            target[i++] = data[p++];
        }
    }

    target[i]='\0';
}

tp_udf_file udf_get_sub(const HANDLE hDrive, tp_udf_file f)
{
    if (f->is_dir && !f->is_parent && f->fid) {
        BYTE sector[fio_SECTOR_SIZE];
        tp_udf_tag tag = (tp_udf_tag)sector;
        bool res;

        res = aspi_ReadSectors(hDrive, f->partition_lba + f->fid->ICB.Location.Location, 1, f->sec_size, sector);
        if (res && tag->TagIdentifier == udf_TAG_FileEntry) {
            tp_udf_FileEntry fe = (tp_udf_FileEntry)sector;

            if (fe->ICBTag.FileType == udf_FT_Directory) {
                tp_udf_file newf = (tp_udf_file)malloc(sizeof *newf);

                if (newf == nullptr) {
                    return nullptr;
                }

                ZeroMemory(newf, sizeof(*newf));
                strcpy_s(newf->name, f->name); // maybe just ""?
                newf->sec_size = f->sec_size;
                newf->partition_lba = f->partition_lba;
                udf_GetLBA(fe, f->sec_size, &newf->dir_lba, &newf->dir_end_lba);
                newf->dir_left = (DWORD)fe->InformationLength; // don't want directories of more than 4gb
                newf->sector = nullptr;
                newf->fid = nullptr;
                newf->is_dir = true;
                newf->is_parent = false;
                return newf;
            }
        }
    }
    return nullptr;
}

tp_udf_file udf_get_next(const HANDLE hDrive, tp_udf_file f)
{
    if (f->dir_left <= 0) {
        f->fid = nullptr;
        return nullptr;
    }

    if (f->fid) {
        // advance to next FileIdentifierDescriptor
        DWORD ofs = 4 * ((sizeof *(f->fid) + f->fid->LengthofImplementationUse + f->fid->LengthofFileIdentifier + 3) / 4);

        f->fid = (tp_udf_FileIdentifierDescriptor)((BYTE *)f->fid + ofs);
    }

    if (f->fid == nullptr) {
        bool res = true;

        DWORD size = f->sec_size * (f->dir_end_lba - f->dir_lba + 1);

        if (!f->sector) {
            f->sector = (BYTE*)malloc(size);
        }
        res = aspi_ReadSectors(hDrive, f->partition_lba + f->dir_lba, (WORD)(f->dir_end_lba - f->dir_lba + 1), f->sec_size, f->sector);
        if (res) {
            f->fid = (tp_udf_FileIdentifierDescriptor)f->sector;
        } else {
            f->fid = nullptr;
        }
    }

    if (f->fid && f->fid->DescriptorTag.TagIdentifier == udf_TAG_FileIdentifierDescriptor) {
        DWORD ofs = 4 * ((sizeof *f->fid + f->fid->LengthofImplementationUse + f->fid->LengthofFileIdentifier + 3) / 4);

        f->dir_left -= ofs;
        f->is_dir = (f->fid->FileCharacteristics & udf_FID_Directory) != 0;
        f->is_parent = (f->fid->FileCharacteristics & udf_FID_Parent) != 0;
        udf_GetName(f->fid->ImplementationUse + f->fid->LengthofImplementationUse, f->fid->LengthofFileIdentifier, f->name);
        return f;
    }
    return nullptr;
}

void udf_free(tp_udf_file f)
{
    if (f) {
        if (f->sector) {
            free(f->sector);
        }
        free(f);
    }
}

#pragma warning(push)
#pragma warning(disable:4995 4996)

#define udf_PATH_DELIMITERS "/\\"

static tp_udf_file udf_ff_traverse(const HANDLE hDrive, tp_udf_file f, char *token)
{
    while (udf_get_next(hDrive, f)) {
        if (_stricmp(token, f->name) == 0) {
            char *next_tok = strtok(nullptr, udf_PATH_DELIMITERS);

            if (!next_tok) {
                return f;    // found
            } else if (f->is_dir) {
                tp_udf_file f2 = udf_get_sub(hDrive, f);

                if (f2) {
                    tp_udf_file f3 = udf_ff_traverse(hDrive, f2, next_tok);

                    if (!f3) {
                        udf_free(f2);
                    }
                    return f3;
                }
            }
        }
    }
    return nullptr;
}

tp_udf_file udf_find_file(const HANDLE hDrive, const WORD partition, const char *name)
{
    tp_udf_file f = udf_get_root(hDrive, partition), f2 = nullptr;

    if (f) {
        char tokenline[udf_MAX_PATHLEN];
        char *token;

        strcpy_s(tokenline, name);
        token = strtok(tokenline, udf_PATH_DELIMITERS);
        if (token) {
            f2 = udf_ff_traverse(hDrive, f, token);
        }
        udf_free(f);
    }
    return f2;
}

#pragma warning(pop)

bool udf_get_lba(const HANDLE hDrive, const tp_udf_file f, DWORD *start_lba, DWORD *end_lba)
{
    if (f->fid) {
        BYTE sector[2048];
        tp_udf_FileEntry fe = (tp_udf_FileEntry)sector;
        bool res;

        res = aspi_ReadSectors(hDrive, f->partition_lba + f->fid->ICB.Location.Location, 1, f->sec_size, sector);
        if (res && fe->DescriptorTag.TagIdentifier == udf_TAG_FileEntry) {
            return udf_GetLBA(fe, f->sec_size, start_lba, end_lba);
        }
    }
    return false;
}
