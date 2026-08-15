#!/usr/bin/env python3

# Copyright (c) 2023,2026 Yuichi Nakamura
# Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

import sys
import os
import time
from struct import pack, unpack

# X68k FD boot sector
FDBOOTSECT = b'`<\x90X68IPL30\x00\x04\x01\x01\x00\x02\xc0\x00\xd0\x04\xfe\x02\x00\x08\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00           FAT12   O\xfa\xff\xc0M\xfa\x01\xb8K\xfa\x00\xe0I\xfa\x00\xeaC\xfa\x01 N\x94p\x8eNO~p\xe1H\x8e@&:\x01\x02"N$:\x01\x002\x07N\x95f("N2:\x00\xfa IE\xfa\x01xp\n\x00\x10\x00 \xb1\nV\xc8\xff\xf8g8\xd2\xfc\x00 Q\xc9\xff\xe6E\xfa\x00\xe0`\x10E\xfa\x00\xfa`\nE\xfa\x01\x10`\x04E\xfa\x01(a\x00\x00\x94"JL\x99\x00\x06p#NON\x942\x07pONOp\xfeNOt\x004)\x00\x1a\xe1Z\xd4z\x00\xa4\x84\xfa\x00\x9c\x84z\x00\x94\xe2\nd\x04\x08\xc2\x00\x18HBR\x02"N&:\x00~2\x07N\x954|h\x00"N\x0cYHUf\xa6T\x89\xb5\xd9f\xa6/\x19 Y\xd1\xd9/\x08/\x112|g\xc0v@\xd6\x88N\x95"\x1f$\x1f"_J\x80f\x00\xff|\xd5\xc2S\x81e\x04B\x1a`\xf8N\xd1pFNO\x08\x00\x00\x1ef\x02p\x00Nup!NONur\x0fp"NOr\x19t\x0cp#NOa\x08r\x19t\rp#NOv,r p NOQ\xcb\xff\xf8Nu\x00\x00\x04\x00\x03\x00\x00\x06\x00\x08\x00\x1f\x00\t\x1a\x00\x00"\x00\rHuman.sys \x82\xaa \x8c\xa9\x82\xc2\x82\xa9\x82\xe8\x82\xdc\x82\xb9\x82\xf1\x00\x00%\x00\r\x83f\x83B\x83X\x83N\x82\xaa\x81@\x93\xc7\x82\xdf\x82\xdc\x82\xb9\x82\xf1\x00\x00\x00#\x00\rHuman.sys \x82\xaa \x89\xf3\x82\xea\x82\xc4\x82\xa2\x82\xdc\x82\xb7\x00\x00 \x00\rHuman.sys \x82\xcc \x83A\x83h\x83\x8c\x83X\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x00human   sys\x00\x00\x00\x00\x00'

# BPB parameters
BPB = {
    '':   ( 1024, 1, 1, 2, 192, 1232, 0xfe, 2,  8, 2 ),       # 2HD (1232KB)
    '/5': (  512, 1, 1, 2, 224, 2400, 0xf9, 7, 15, 2 ),       # 2HC (1200KB)
    '/8': (  512, 2, 1, 2, 112, 1280, 0xfb, 2,  8, 2 ),       # 2DD  (640KB)
    '/9': (  512, 2, 1, 2, 112, 1440, 0xf9, 3,  9, 2 ),       # 2DD  (720KB)
    '/4': (  512, 1, 1, 2, 224, 2880, 0xf0, 9, 18, 2 ),       # 2HQ (1440KB)
}

# SASI hard disks use 256-byte physical sectors.
HDF = {
    # image sectors, disk table value, partition sectors, logical sectors,
    # sectors/FAT.  These values match genuine X68000 SASI images.
    '/h10': (40788, 0xaf50, 0x9f2e, 0x27cb, 0x14),
    '/h20': (81048, 0x15660, 0x13c68, 0x4f1a, 0x28),
    '/h40': (162096, 0x2acc0, 0x278f8, 0x9e3e, 0x50),
}
HDF_PART_START = 33

# X68000 SASI HDD IPL MENU and Hudson soft 2.00 partition boot sector.
# The BPB in HDBOOTSECT is replaced with the selected disk's values.
HDDIPL = bytes.fromhex(
    '600000ca0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001a1b5b363b3332485836383030302048415244204449534b2049504c204d454e551b5b32353b323248834a815b835c838b834c815b82c5914991f082b582c4838a835e815b8393834c815b82f0899f82b582c482ad82be82b382a2001b5b32363b323848914991f082b582bd82e082cc82f08ea993ae8b4e93ae82c682b582c4936f985e82b582dc82b700004ffaff324285203c0000008e4e4f1e00e1477404263c0000010043fa02c6610002324a006600016a43fa02b847faff0a0c915836384b6600016c740e42434244428643e900104a116716524626c91029000808000000660852434a006602524451caffe04a4367000142720a70044e4f0800000466124a44670e5344671c43fafef4610001c4602843fafeea610001ba43faff3e610001b2601447fafe9c205b242800084a28000866f4600000be7a02424345fafe84225210290008670ab005670672026100017661000116588a5243b64665e2602a61000106610000f2b03c001d673ab03c0035670cb03c003c671ab03c003e66e4610000e85243b64665024243610000b266f260cc610000d453436a04360653436100009e66f260b847fafe18e54320733000242800084a05673a43fa01b8720e43e900104a29fff86712201108000018660a4211b480670412bc000251c9ffe22f027404263c0000010043fa0180610000e8241fc4bc00ffffff263c0000040043fafdbcd3fc00000400610000d04a0066080c11006066224ed145fa00da43fa00cc610000ae224a610000a860fe45fa00e360ea45fa00fa60e445fa011160de41fafd802003e5402070000010280008b00567024a004e7542804e4fe048b03c004e6602701d4e75615a43fafd563003e54043f1000072247409d44370234e4f225172286146240941fa00e29488e88a84fc000ad4bc003000307220b47c00306702320261264842320261207229611c72206118740742411219611051cafff872036002720b70224e4f4e7570204e4f4e7570214e4f4e7570456002704648e7784032074e4f4cdf021e4e751a1b5b31363b333348002020836e815b8368836683428358834e82aa93c782df82dc82b982f10020208ac7979d8375838d8362834e82aa89f382ea82c482a282dc82b70020208b4e93ae89c2945c82c897cc88e682aa82a082e882dc82b982f1008268826f826b8375838d8362834e82cc93e0976582aa88d98fed82c582b70000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000')
HDBOOTSECT = bytes.fromhex(
    '6020487564736f6e20736f667420322e303004000102000102009e3ef850000000214ffaffdc43fa01346100012a203c0000008e4e4f1200e1414282143affdf4280103affd1c4c04280303affcad480e582d4baffca263c0000040043fa019a610000c84a00660000d043fa018c3c3c001f244947fa01757e0a101a803c0020b01b660651cffff46014d3fc0000002051ceffe043fa00c8610000bc60fe42803029001ae158554042871e3aff68c0c7e580d4804280303aff60eb80d0bc000003ffe088e488e580d48048e7600043fa0128263c0000010061504cdf00064a00665643fa01140c594855665654890c990000680066562f192619d6992f032f19227c000067c0d6bc00000040611c221f241f225f4a00662041f900006800d1c253816504421860f84ed148e7784070464e4f4cdf021e4e7543fa004861186000ff5c43fa005d610e6000ff5243fa007561046000ff4870214e4f4e751a001b5b31363b33354848756d616e2e7379732082aa208ca982c282a982e882dc82b982f1001b5b31363b333848836683428358834e82aa814093c782df82dc82b982f1001b5b31363b33364848756d616e2e7379732082aa2089f382ea82c482a282dc82b7001b5b31363b33334848756d616e2e7379732082cc2083418368838c835882aa88d98fed82c582b70068756d616e202020737973000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000')

# For error handling
class DiskFull(Exception):
    def __str__(self):
        return 'Disk image full'
class RootDirFull(Exception):
    def __str__(self):
        return 'Root directory full'
class FileNotFound(Exception):
    def __init__(self, name):
        self.name = name
    def __str__(self):
        return f'No such file or directory: \'{self.name:s}\''


class DiskFat:
    def __init__(self, disk):
        """Create new FAT data"""
        self.disk = disk
        self.table = [0] * self.disk.maxcls
        self.table[0] = self.disk.fatend
        self.table[1] = self.disk.fatend

        if self.disk.fhin:          # read FAT data in the XDF file
            self.disk.fhin.seek(self.disk.sect2byte(self.disk.snfat))
            self.decode(self.disk.fhin.read(self.disk.sect2byte0(self.disk.fatsize)))

    def flush(self):
        """Write FAT data into new XDF file"""
        self.disk.fhout.seek(self.disk.sect2byte(self.disk.snfat))
        self.disk.fhout.write(self.encode())

    def decode(self, data):
        """Decode binary FAT data into the table entries"""
        if self.disk.fatbits == 16:
            for n in range(self.disk.maxcls):
                self.table[n] = unpack(self.disk.byteorder + 'H',
                                       data[n * 2:n * 2 + 2])[0]
            return
        n = 0
        i = 0
        while n < self.disk.maxcls:
            self.table[n] = data[i] + ((data[i + 1] & 0xf) << 8)
            if n >= self.disk.maxcls - 1:
                break
            self.table[n + 1] = ((data[i + 1] & 0xf0) >> 4) + (data[i + 2] << 4)
            n += 2
            i += 3

    def encode(self):
        """Encode FAT data into binary"""
        if self.disk.fatbits == 16:
            data = bytearray(b'\0' * self.disk.sect2byte0(self.disk.fatsize))
            for n, c in enumerate(self.table):
                data[n * 2:n * 2 + 2] = pack(self.disk.byteorder + 'H', c)
            data[0] = self.disk.mediabyte
            return data
        data = bytearray(b'\0' * self.disk.sect2byte0(self.disk.fatsize))
        n = 0
        f = True
        for c in self.table:
            if f:
                d = c
            else:
                d = d | (c << 12)
                dd = pack('<L', d)
                data[n:n + 3] = dd[:3]
                n += 3
            f = not f
        if not f:
            dd = pack('<L', d)
            data[n:n + 3] = dd[:3]
        data[0] = self.disk.mediabyte
        return data

    def getchain(self, cls):
        """Get cluster chain starting with cluster 'cls'"""
        if cls == 0:
            return []
        clist = [cls]
        while self.table[cls] < self.disk.fatendmin:
            cls = self.table[cls]
            clist.append(cls)
        return clist

    def allocate(self, size=0):
        """Allocate clusters for 'size' bytes data"""
        ncls = (size // self.disk.clsbytes) + 1
        clist = []
        lastcls = -1
        try:
            for i in range(ncls):
                cls = self.table.index(0, lastcls + 1)
                clist.append(cls)
                lastcls = cls
        except ValueError as e:
            raise DiskFull
        for i in range(len(clist) - 1):
            self.table[clist[i]] = clist[i + 1]
        self.table[clist[-1]] = self.disk.fatend
        return clist[0]

    def extend(self, cls):
        """Extend existing cluster chain to add one cluster"""
        clist = self.getchain(cls)
        try:
            c = self.table.index(0)
        except ValueError as e:
            raise DiskFull
        self.table[clist[-1]] = c
        self.table[c] = self.disk.fatend

    def __repr__(self):
        x = 0
        r = ''
        for n in self.table:
            if (x & 0xf) == 0:
                r += f'\n{x:03x} :'
            r += f' {n:03x}'
            if (x & 0xf) == 7:
                r += ' '
            x += 1
        r += '\n'
        return r


class Dirent:
    def __init__(self, disk, data=None, name='', attr=0, cls=0, dir=None):
        """Create new directory entry"""
        self.disk = disk
        if data:        # read existing entry
            self.decode(data)
        else:           # create new entry
            self.name = name
            self.attr = attr
            self.cls = cls
            self.size = 0
            self.time = time.time()
            self.dir = dir

    def decode(self, data):
        """Decode one directory entry"""
        self.name = self.decodename(data[0:8], data[8:11], data[12:22])
        self.attr = data[11]
        (dtime, ddate, self.cls, self.size) = unpack('<3HL', data[22:32])
        self.time = time.mktime((
                        ((ddate >> 9) & 0x7f) + 1980,
                        ((ddate >> 5) & 0xf),
                        (ddate & 0x1f),
                        ((dtime >> 11) & 0x1f),
                        ((dtime >> 5) & 0x3f),
                        ((dtime & 0x1f) << 1),
                        0, 0, 0))

    def encode(self):
        """Encode one directory entry"""
        (name1, ext, name2) = self.encodename(self.name)
        data = pack('<8s3sB10s', name1, ext, self.attr, name2)
        tm = time.localtime(self.time)
        dtime = (tm[3] << 11) | (tm[4] << 5) | (tm[5] >> 1)
        ddate = ((tm[0] - 1980) << 9) | (tm[1] << 5) | tm[2]
        data += pack('<3HL', dtime, ddate, self.cls, self.size)
        return data

    def decodename(self, name1, ext, name2):
        """Decode filename in the directory entry"""
        name = name1
        if name2[0]:
            name = (name1 + name2.replace(b'\0', b' ')).rstrip(b' ')
        else:
            name = name1.rstrip(b' ')
        if name[0] == 0x05:
            name[0] = 0xe5
        if ext != b'   ':
            name += b'.' + ext.rstrip(b' ')         # file extension
        return name.decode('ShiftJIS')

    def encodename(self, name):
        """Encode filename for the directory entry"""
        bname = name.encode('ShiftJIS')
        if bname[0] == 0xe5:
            bname[0] = 0x05
        ext = b''
        if name != '.' and name != '..':
            epos = bname.rfind(b'.')
            if epos > 0 and (len(bname) - epos) <= 4:   # file extension
                ext = bname[epos + 1:]
                bname = bname[:epos]
        ext = (ext + b'   ')[:3]
        bname = bname + b' ' * 18
        name1 = bname[0:8]
        name2 = (bname[8:18].rstrip(b' ') + b'\0' * 10)[:10]
        return (name1, ext, name2)

    def isdir(self):
        return self.attr & 0x10
    
    def isvol(self):
        return self.attr & 0x08

    def readdir(self):
        if not self.isdir():
            return None
        return DiskDir(self.disk, self.cls)

    def readfile(self, dirname):
        """Read file contents from XDF image and create the file"""
        if self.name == '.' or self.name == '..':
            return
        if self.isvol() or self.isdir():
            return
        with open(dirname + self.name, 'wb') as f:
            size = self.size
            clsbytes = self.disk.clsbytes
            for c in self.disk.fat.getchain(self.cls):
                s = min(size, clsbytes)
                if s == 0:
                    break
                self.disk.fhin.seek(self.disk.clsoffset(c))
                f.write(self.disk.fhin.read(s))
                size -= s
        if 'utime' in os.__dict__:
            os.utime(dirname + self.name, times=(self.time, self.time))

    def __repr__(self):
        if self.isdir():
            s = '  <dir>   '
        elif self.isvol():
            s = '  <vol>   '
        else:
            s = f'{self.size:-8d}  '
        tm = time.localtime(self.time)
        s += f'{tm[0]:04d}-{tm[1]:02d}-{tm[2]:02d} {tm[3]:2d}:{tm[4]:02d}:{tm[5]:02d}'
        s += '  ' + self.name
        return s


class DiskDir:
    def __init__(self, disk, cls=0):
        """Create new directory data starting with cluster 'cls'"""
        self.disk = disk
        self.entlist = []
        self.cls = cls

        if self.disk.fhin:          # read diectory in the XDF file
            if self.cls == 0:           # root directory
                self.disk.fhin.seek(self.disk.sect2byte(self.disk.snrootdir))
                self.decode(self.disk.fhin.read(self.disk.nrootdirent * 32))
            else:                       # sub directory
                dirblk = b''
                for c in self.disk.fat.getchain(self.cls):
                    self.disk.fhin.seek(self.disk.clsoffset(c))
                    dirblk += self.disk.fhin.read(self.disk.clsbytes)
                self.decode(dirblk)
        else:                       # create initial directory data
            if self.cls == 0:           # root directory
                self.entmax = self.disk.nrootdirent
            else:                       # sub directory
                self.entmax = self.disk.clsbytes // 32

    def flush(self):
        """Write directory into new XDF file"""
        data = self.encode()
        if self.cls == 0:               # root directory
            self.disk.fhout.seek(self.disk.sect2byte(self.disk.snrootdir))
            self.disk.fhout.write(data)
        else:                           # sub directory
            i = 0
            for c in self.disk.fat.getchain(self.cls):
                self.disk.fhout.seek(self.disk.clsoffset(c))
                self.disk.fhout.write(data[i:i + self.disk.clsbytes])
                i += self.disk.clsbytes

    def decode(self, data):
        """Decode directory data into entry lists"""
        self.entlist = []
        i = 0
        while i < len(data):
            if data[i] == 0:
                break
            if data[i] != 0xe5:
                dirent = Dirent(self.disk, data[i : i+32])
                self.entlist.append(dirent)
            i += 32

    def encode(self):
        """Encode directory entry list into the data"""
        data = b''
        for d in self.entlist:
            data += d.encode()
        return data

    def adddirent(self, dirent):
        """Add one directory entry into the list and extend the directory data if needed"""
        self.entlist.append(dirent)
        if len(self.entlist) > self.entmax:
            if self.cls == 0:
                raise RootDirFull   # root directory cannot be extended
            self.disk.fat.extend(self.cls)
            self.entmax += self.disk.clsbytes // 32

    def mkdir(self, dirname):
        """Make new sub directory"""
        for f in self.entlist:
            if f.name == dirname:
                return f.dir        # already exists

        # create new empty directory entries
        newdir = DiskDir(self.disk, self.disk.fat.allocate())
        newdir.adddirent(Dirent(self.disk, name='.', attr=0x10, cls=newdir.cls))
        newdir.adddirent(Dirent(self.disk, name='..', attr=0x10, cls=self.cls))

        # add directory data as sub directory
        newent = Dirent(self.disk, name=dirname, attr=0x10, cls=newdir.cls)
        newent.dir = newdir
        self.adddirent(newent)
        self.disk.subdirs.append(newdir)
        return newdir

    def writefile(self, dirname, fname):
        """Write file contents into XDF image from the existing file"""
        try:
            st = os.stat(dirname + fname)
        except:
            raise FileNotFound(dirname + fname)
        size = st[6]
        newent = Dirent(self.disk, name=fname, attr=0x20, cls=self.disk.fat.allocate(size))
        newent.size = size
        newent.time = st[8]

        with open(dirname + fname, 'rb') as f:
            clsbytes = self.disk.clsbytes
            for c in self.disk.fat.getchain(newent.cls):
                s = min(size, clsbytes)
                if s == 0:
                    break
                self.disk.fhout.seek(self.disk.clsoffset(c))
                self.disk.fhout.write(f.read(s))
                size -= s
        self.adddirent(newent)

    def __iter__(self):
        return iter(self.entlist)

    def __repr__(self):
        r = ''
        for n in self.entlist:
            r += str(n) + '\n'
        return r


class XdfDisk:
    def __init__(self, fhin=None, fhout=None, format=None):
        """Create new XDF disk image"""
        self.fhin = fhin                # file handle for input (list/extract)
        self.fhout = fhout              # file handle for output (create)

        bpb = BPB['']
        self.base = 0
        self.ishdf = False
        self.byteorder = '<'
        if self.fhin:
            fhin.seek(0x400)
            if fhin.read(4) == b'X68K':
                self.ishdf = True
                self.byteorder = '>'
                fhin.seek(0x410 + 8)
                self.base = unpack('>L', fhin.read(4))[0] * 256
            # Human68k HDD boot sectors place the BPB at 0x12; floppy BPBs
            # use the DOS-compatible 0x0b location.
            fhin.seek(self.base + (0x12 if self.ishdf else 0xb))
            b = fhin.read(0x11)
            if self.ishdf:
                # Human68k's HDD BPB has 68000-order words except for the
                # FAT start and FAT size fields inherited from the DOS BPB.
                bpb = (unpack('>H', b[0:2])[0], b[2], b[5], b[3],
                       unpack('>H', b[6:8])[0],
                       unpack('>H', b[8:10])[0], b[10],
                       unpack('<H', b[11:13])[0],
                       unpack('>H', b[13:15])[0],
                       unpack('>H', b[15:17])[0])
            elif b[10] >= 0xf0:
                bpb = unpack('<HBHB2HB3H', b)
        else:
            if format in BPB:
                bpb = BPB[format]
            elif format in HDF:
                self.ishdf = True
                self.byteorder = '>'
                self.base = HDF_PART_START * 256
                nsect = HDF[format][3]
                fatsize = HDF[format][4]
                bpb = (1024, 1, 1, 2, 512, nsect, 0xf8, fatsize, 0, 0)

        self.sectbytes = bpb[0]         # sector size in byte (1024)
        self.clssect = bpb[1]           # sectors per cluster (1)
        self.snfat = bpb[2]             # sector # of the FAT (1)
        self.nfat= bpb[3]               # # of FATs (2)
        self.nrootdirent = bpb[4]       # # of root directory entries (192)
        self.nsect = bpb[5]             # # of sectors (1232)
        self.mediabyte = bpb[6]         # media byte (0xfe)
        self.fatsize = bpb[7]           # sectors per FAT (2)

        # cluster size in byte
        self.clsbytes = self.sect2byte0(self.clssect)
        # sector # of the root directory
        self.snrootdir = self.snfat + self.nfat * self.fatsize
        # sector # of the data area
        self.sndata = self.snrootdir + (self.nrootdirent * 32) // self.sectbytes
        # max cluster number + 1
        self.maxcls = ((self.nsect - self.sndata) // self.clssect) + 2
        self.fatbits = 16 if self.maxcls >= 4087 else 12
        self.fatend = 0xffff if self.fatbits == 16 else 0xfff
        self.fatendmin = 0xfff8 if self.fatbits == 16 else 0xff8

        if fhout:
            if self.ishdf:
                hdf = HDF[format]
                fhout.seek(hdf[0] * 256 - 1)
                fhout.write(b'\0')
                fhout.seek(0)
                fhout.write(HDDIPL)
                fhout.write(b'X68K' + pack('>3L', hdf[0], hdf[0], hdf[1]))
                fhout.write(b'Human68k' + pack('>2L', HDF_PART_START,
                                               hdf[2]))
                fhout.seek(self.base)
                fhout.write(HDBOOTSECT)
            else:
                for i in range(self.nsect):
                    fhout.write(bytes(self.sectbytes))
                fhout.seek(0)
                fhout.write(FDBOOTSECT)
            fhout.seek(self.base + (0x12 if self.ishdf else 0xb))
            if self.ishdf:
                fhout.write(pack('>H', bpb[0]) +
                            bytes((bpb[1], bpb[3], 0, bpb[2])) +
                            pack('>2H', bpb[4], bpb[5]) + bytes((bpb[6],)) +
                            pack('<H', bpb[7]) + pack('>H', bpb[8]))
            else:
                fhout.write(pack('<HBHB2HB3H', *bpb))
            if format == '/4':
                fhout.seek(0)
                fhout.write(b'\xeb\xfe\x90')

        self.fat = DiskFat(self)
        self.rootdir = DiskDir(self)
        self.subdirs = []

    def sect2byte(self, sect):
        return self.base + self.sect2byte0(sect)

    def sect2byte0(self, sect):
        return self.sectbytes * sect

    def clsoffset(self, cls):
        return self.sect2byte(self.sndata) + self.clsbytes * (cls - 2)

    def listxdf(self, dir, name):
        subdirs = []
        print(name + ':')
        for ent in dir:
            print(ent)
            if ent.name == '.' or ent.name == '..':
                continue
            if ent.isdir():
                subdirs.append(ent)
        for ent in subdirs:
            print()
            self.listxdf(ent.readdir(), name + '/' + ent.name)

    def extractxdf(self, dir, dirname, extfiles):
        for ent in dir:
            if ent.name == '.' or ent.name == '..':
                continue
            if ent.isdir():
                extfilessub = extfiles
                if not extfiles or ((dirname + ent.name) in extfiles) \
                                or ((dirname + ent.name + '/') in extfiles):
                    extfilessub = []        # extract whole sub directory
                else:
                    for f in extfiles:
                        if f.startswith(dirname + ent.name + '/'):
                            break           # extract specified files in sub directory
                    else:
                        continue
                try:
                    os.mkdir(dirname + ent.name)
                except FileExistsError as e:
                    pass
                self.extractxdf(ent.readdir(), dirname + ent.name + '/', extfilessub)
            else:
                if not extfiles or ((dirname + ent.name) in extfiles):
                    print(dirname + ent.name)
                    ent.readfile(dirname)

    def createxdf(self, dir, dirname, files):
        if files == [] or files == ['']:
            files = os.listdir(dirname)
        for f in files:
            if f == '.' or f == '..':
                continue
            if '/' in f:            # add specified files in sub directory
                i = f.find('/')
                df = f[:i]
                ff = f[i+1:]
                d = dir.mkdir(df)
                self.createxdf(d, dirname + df + '/', [ff])
            else:                   # add one file or whole sub directory
                try:
                    st = os.stat(dirname + f)
                except:
                    raise FileNotFound(dirname + f)
                if st[0] & 0o40000:     # directory
                    d = dir.mkdir(f)
                    self.createxdf(d, dirname + f + '/', [])
                else:
                    print(dirname + f)
                    dir.writefile(dirname, f)

    def flush(self):
        """Write FAT and directory data into new XDF file"""
        self.fat.flush()
        self.rootdir.flush()
        for d in self.subdirs:
            d.flush()


def usage():
    print(f'usage: {sys.argv[0]:s} t <xdf file>                         : List XDF contents')
    print(f'       {sys.argv[0]:s} x <xdf file> [<files>...]            : Extract XDF file')
    print(f'       {sys.argv[0]:s} c [<format>] <xdf file> [<files>...] : Create XDF file')
    print( 'format: (none)=2HD(1232KB) /5=2HC(1200KB) /8=2DD(640KB) /9=2DD(720KB) /4=2HQ(1440KB)')
    print( '        /h10=SASI HDD(10MB) /h20=SASI HDD(20MB) /h40=SASI HDD(40MB)')
    sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        usage()

    try:
        if sys.argv[1] == 't' or sys.argv[1] == 'list':
            with open(sys.argv[2], 'rb') as f:
                d = XdfDisk(fhin=f)
                d.listxdf(d.rootdir, '.')
        elif sys.argv[1] == 'x' or sys.argv[1] == 'extract':
            extfiles = sys.argv[3:]
            with open(sys.argv[2], 'rb') as f:
                d = XdfDisk(fhin=f)
                d.extractxdf(d.rootdir, '', extfiles)
        elif sys.argv[1] == 'c' or sys.argv[1] == 'create':
            if sys.argv[2] in BPB or sys.argv[2] in HDF:
                if len(sys.argv) < 3:
                    usage()
                format = sys.argv[2]
                fname = sys.argv[3]
                extfiles = sys.argv[4:]
            else:
                format = ''
                fname = sys.argv[2]
                extfiles = sys.argv[3:]
            with open(fname, 'wb') as f:
                d = XdfDisk(fhout=f, format=format)
                if extfiles:
                    d.createxdf(d.rootdir, '', extfiles)
                d.flush()
        else:
            usage()
    except (DiskFull, RootDirFull, FileNotFound) as e:
        print(sys.argv[0] + ': error: ' + str(e))
        sys.exit(1)

    sys.exit(0)
