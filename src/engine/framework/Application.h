/*
===========================================================================
Daemon BSD Source Code
Copyright (c) 2015, Daemon Developers
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Daemon developers nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL DAEMON DEVELOPERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
===========================================================================
*/

#ifndef FRAMEWORK_APPLICATION_H_
#define FRAMEWORK_APPLICATION_H_

namespace Application {

struct Configuration {
    // TODO remove the need for that
    bool isClient = false;
    bool isTTYClient = false;
    bool isServer = false;
};

class Application {
    public:
        Application();

        virtual void LoadInitialConfig(bool resetConfig);
        virtual void Initialize() {}
        virtual void Frame() {}

        virtual void OnDrop(Str::StringRef reason);
        virtual void Shutdown(bool error, Str::StringRef message);

        virtual void OnUnhandledCommand(const Cmd::Args&);

        const Configuration& GetConfig() const;

    protected:
        Configuration config;
};

Application& GetApp();
const Configuration& GetAppConfig();

#define INSTANCIATE_APPLICATION(classname) \
    Application& GetApp() { \
        static classname app; \
        return app; \
    }
#define TRY_SHUTDOWN(code) try {code;} catch (Sys::DropErr&) {}

}

#endif // FRAMEWORK_APPLICATION_H_
