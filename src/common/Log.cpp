/*
===========================================================================
Daemon BSD Source Code
Copyright (c) 2013, Daemon Developers
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
===========================================================================
*/

#include "Log.h"

#include "../engine/framework/LogSystem.h"
#include "../engine/qcommon/qcommon.h"

namespace Log {

    Logger::Logger(Str::StringRef name, Level defaultLevel)
    :filterLevel("logs.logLevel." + name, "Log::Level - filters out the logs from '" + name + "' below the level specified", 0, defaultLevel) {
    }

    bool ParseCvarValue(std::string value, Log::Level& result) {
        if (value == "error" or value == "err") {
            result = Log::ERROR;
            return true;
        }

        if (value == "warning" or value == "warn") {
            result = Log::WARNING;
            return true;
        }

        if (value == "info" or value == "notice") {
            result = Log::NOTICE;
            return true;
        }

        if (value == "debug" or value == "all") {
            result = Log::DEBUG;
            return true;
        }

        return false;
    }

    std::string SerializeCvarValue(Log::Level value) {
        switch(value) {
            case Log::ERROR:
                return "error";
            case Log::WARNING:
                return "warning";
            case Log::NOTICE:
                return "notice";
            case Log::DEBUG:
                return "debug";
            default:
                return "";
        }
    }

	//TODO add the time (broken for now because it is journaled) use Sys_Milliseconds instead (Utils::Milliseconds ?)
    static const int errorTargets = (1 << GRAPHICAL_CONSOLE) | (1 << TTY_CONSOLE) | (1 << CRASHLOG) | (1 << HUD) | (1 << LOGFILE);
    void CodeSourceError(std::string message) {
        Log::Dispatch({/*Com_Milliseconds()*/0, "^1Error: " + message}, errorTargets);
    }

    static const int warnTargets = (1 << GRAPHICAL_CONSOLE) | (1 << TTY_CONSOLE) | (1 << CRASHLOG) | (1 << LOGFILE);
    void CodeSourceWarn(std::string message) {
        Log::Dispatch({/*Com_Milliseconds()*/0, "^3Warn: " + message}, warnTargets);
    }

    static const int noticeTargets = (1 << GRAPHICAL_CONSOLE) | (1 << TTY_CONSOLE) | (1 << CRASHLOG) | (1 << LOGFILE);
    void CodeSourceNotice(std::string message) {
        Log::Dispatch({/*Com_Milliseconds()*/0, message}, noticeTargets);
    }

    static const int debugTargets = (1 << GRAPHICAL_CONSOLE) | (1 << TTY_CONSOLE);
    void CodeSourceDebug(std::string message) {
        Log::Dispatch({/*Com_Milliseconds()*/0, "^5Debug: " + message}, debugTargets);
    }
}

namespace Cvar {
    template<>
    std::string GetCvarTypeName<Log::Level>() {
        return "log level";
    }
}
