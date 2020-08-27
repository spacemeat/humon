def getVersion():
    version = { 'major': 0, 
                'minor': 0, 
                'patch': 0 }
    fi = open("include/humon/version.h", "rt")
    for l in fi.readlines():
        if l.strip().startswith('#define '):
            vdef = l.strip().split()
            if vdef[1] == "HUMON_MAJOR_VERSION":
                version['major'] = vdef[2]
            elif vdef[1] == "HUMON_MINOR_VERSION":
                version['minor'] = vdef[2]
            elif vdef[1] == "HUMON_PATCH_VERSION":
                version['patch'] = vdef[2]
            else:
                raise RuntimeError(f"Unrecognized version label '{vdef[1]}'")

        elif not l.strip().startswith('#'):
            raise RuntimeError("Unrecognized content in version file.")
    fi.close()
    return version
