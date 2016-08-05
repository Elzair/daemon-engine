#!/usr/bin/python

# Daemon CBSE Source Code
# Copyright (c) 2014-2016, Daemon Developers
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of Daemon CBSE nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


import yaml
import os
import re

if __name__ == '__main__':
    # Find directory path for this file
    pwd = os.path.dirname(os.path.abspath(__file__))

    # Read in default contents
    with open(os.path.join(pwd, "default.yaml"), 'r') as d:
        default = yaml.load(d.read())

    # Get new defs file, if present
    yaml_files = [f for f in os.listdir(pwd) if re.match(".*\.yaml\Z", f) and f != "default.yaml"]
    if len(yaml_files) == 1:
        with open(os.path.join(pwd, yaml_files[0]), 'r') as dt:
            data = yaml.load(dt.read())
    else:
        data = dict()

    # Began building Defs.h file
    src = ['#ifndef COMMON_DEFS_H_', '#define COMMON_DEFS_H_']

    # Name
    key = 'PRODUCT_NAME'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    src.append('#define {key} "{val}"'.format(key=key+'_UPPER', val=val.upper()))
    src.append('#define {key} "{val}"'.format(key=key+'_LOWER', val=val.lower()))

    # Version
    key = 'PRODUCT_VERSION'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))

    # Base Package
    key = 'DEFAULT_BASE_PAK'
    val = ('"'+data[key]+'"') if key in data else 'PRODUCT_NAME_LOWER'
    src.append('#define {key} {val}'.format(key=key, val=val))

    # Game String
    key = 'GAMENAME_STRING'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))

    # Game for Master
    key = 'GAMENAME_FOR_MASTER'
    val = ('"'+data[key]+'"') if key in data else 'PRODUCT_NAME_UPPER'
    src.append('#define {key} {val}'.format(key=key, val=val))

    # Master Server Name
    key = 'MASTER_SERVER_NAME'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # IRC Server
    key = 'IRC_SERVER'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # IRC Channel
    key = 'IRC_CHANNEL'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # Base URL
    key = 'WWW_BASEURL'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # Autoexec file
    key = 'AUTOEXEC_NAME'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # Config
    key = 'CONFIG_NAME'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # Keybindings
    key = 'KEYBINDINGS_NAME'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # Team Config
    key = 'TEAMCONFIG_NAME'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # Server Config
    key = 'SERVERCONFIG_NAME'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # Default Player Name
    key = 'UNNAMED_PLAYER'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # Default Server Name
    key = 'UNNAMED_SERVER'
    val = ('"'+data[key]+'"') if key in data else 'PRODUCT_NAME " " PRODUCT_VERSION " Server"'
    src.append('#define {key} {val}'.format(key=key, val=val))
    
    # RSA Key
    key = 'RSAKEY_FILE'
    val = data[key] if key in data else default[key]
    src.append('#define {key} "{val}"'.format(key=key, val=val))
    
    # End of file
    src.append('#endif')

    # Write to Defs.h
    out = open(os.path.abspath(os.path.join(pwd, '..', '..', 'src', 'common', 'Defs.h')), 'w')
    out.write(os.linesep.join(src))
    out.close()
    #print(path)

    #print(os.linesep.join(src))
    #for key in default:
    #    if key in data:
    #        print(data[key])
    #    else:
    #        print(default[key])


    
