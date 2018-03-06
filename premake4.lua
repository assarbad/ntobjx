--[[
        This premake4.lua _requires_ windirstat/premake-stable to work properly.
        If you don't want to use the code-signed build that can be found in the
        ./common/ subfolder, you can build from the WDS-branch over at:

        https://bitbucket.org/windirstat/premake-stable
  ]]
local action = _ACTION or ""
local release = false
local tgtname = "ntobjx"
local cmdline = false
if _OPTIONS["cmdline"] then
    cmdline = true
end
if _OPTIONS["release"] then
    print "INFO: Creating release build solution."
    release = true
end
do
    -- This is mainly to support older premake4 builds
    if not premake.project.getbasename then
        print "Magic happens ..."
        -- override the function to establish the behavior we'd get after patching Premake to have premake.project.getbasename
        premake.project.getbasename = function(prjname, pattern)
            return pattern:gsub("%%%%", prjname)
        end
        -- obviously we also need to overwrite the following to generate functioning VS solution files
        premake.vstudio.projectfile = function(prj)
            local pattern
            if prj.language == "C#" then
                pattern = "%%.csproj"
            else
                pattern = iif(_ACTION > "vs2008", "%%.vcxproj", "%%.vcproj")
            end

            local fname = premake.project.getbasename(prj.name, pattern)
            fname = path.join(prj.location, fname)
            return fname
        end
        -- we simply overwrite the original function on older Premake versions
        premake.project.getfilename = function(prj, pattern)
            local fname = premake.project.getbasename(prj.name, pattern)
            fname = path.join(prj.location, fname)
            return path.getrelative(os.getcwd(), fname)
        end
    end
    -- Make UUID generation for filters deterministic
    if os.str2uuid ~= nil then
        local vc2010 = premake.vstudio.vc2010
        vc2010.filteridgroup = function(prj)
            local filters = { }
            local filterfound = false

            for file in premake.project.eachfile(prj) do
                -- split the path into its component parts
                local folders = string.explode(file.vpath, "/", true)
                local path = ""
                for i = 1, #folders - 1 do
                    -- element is only written if there *are* filters
                    if not filterfound then
                        filterfound = true
                        _p(1,'<ItemGroup>')
                    end
                    
                    path = path .. folders[i]

                    -- have I seen this path before?
                    if not filters[path] then
                        local seed = path .. (prj.uuid or "")
                        local deterministic_uuid = os.str2uuid(seed)
                        filters[path] = true
                        _p(2, '<Filter Include="%s">', path)
                        _p(3, '<UniqueIdentifier>{%s}</UniqueIdentifier>', deterministic_uuid)
                        _p(2, '</Filter>')
                    end

                    -- prepare for the next subfolder
                    path = path .. "\\"
                end
            end
            
            if filterfound then
                _p(1,'</ItemGroup>')
            end
        end
    end
    -- Name the project files after their VS version
    local orig_getbasename = premake.project.getbasename
    premake.project.getbasename = function(prjname, pattern)
        -- The below is used to insert the .vs(8|9|10|11|12|14|15) into the file names for projects and solutions
        if _ACTION then
            name_map = {vs2002 = "vs7", vs2003 = "vs7_1", vs2005 = "vs8", vs2008 = "vs9", vs2010 = "vs10", vs2012 = "vs11", vs2013 = "vs12", vs2015 = "vs14", vs2017 = "vs15"}
            if name_map[_ACTION] then
                pattern = pattern:gsub("%%%%", "%%%%." .. name_map[_ACTION])
            else
                pattern = pattern:gsub("%%%%", "%%%%." .. _ACTION)
            end
        end
        return orig_getbasename(prjname, pattern)
    end
    -- Make sure we can generate XP-compatible projects for newer Visual Studio versions
    local orig_vc2010_configurationPropertyGroup = premake.vstudio.vc2010.configurationPropertyGroup
    premake.vstudio.vc2010.configurationPropertyGroup = function(cfg, cfginfo)
        local old_captured = io.captured -- save io.captured state
        io.capture() -- this sets io.captured = ''
        orig_vc2010_configurationPropertyGroup(cfg, cfginfo)
        local captured = io.endcapture()
        assert(captured ~= nil)
        local toolsets = { vs2012 = "v110", vs2013 = "v120", vs2015 = "v140", vs2017 = "v141" }
        local toolset = toolsets[_ACTION]
        if toolset then
            if _OPTIONS["xp"] then
                toolset = toolset .. "_xp"
                captured = captured:gsub("(</PlatformToolset>)", "_xp%1")
            end
        end
        if old_captured ~= nil then
            io.captured = old_captured .. captured -- restore outer captured state, if any
        else
            io.write(captured)
        end
    end
    -- Premake4 sets the PDB file name for the compiler's PDB to the default
    -- value used by the linker's PDB. This causes error C1052 on VS2017. Fix it.
    local orig_premake_vs2010_vcxproj = premake.vs2010_vcxproj
    premake.vs2010_vcxproj = function(prj)
        local old_captured = io.captured -- save io.captured state
        io.capture() -- this sets io.captured = ''
        orig_premake_vs2010_vcxproj(prj)
        local captured = io.endcapture()
        assert(captured ~= nil)
        captured = captured:gsub("%s+<ProgramDataBaseFileName>[^<]+</ProgramDataBaseFileName>", "")
        if old_captured ~= nil then
            io.captured = old_captured .. captured -- restore outer captured state, if any
        else
            io.write(captured)
        end
    end
    -- ... same as above but for VS200x this time
    local function wrap_remove_pdb_attribute(origfunc)
        local fct = function(cfg)
            local old_captured = io.captured -- save io.captured state
            io.capture() -- this sets io.captured = ''
            origfunc(cfg)
            local captured = io.endcapture()
            assert(captured ~= nil)
            captured = captured:gsub('%s+ProgramDataBaseFileName=\"[^"]+\"', "")
            if old_captured ~= nil then
                io.captured = old_captured .. captured -- restore outer captured state, if any
            else
                io.write(captured)
            end
        end
        return fct
    end
    premake.vstudio.vc200x.VCLinkerTool = wrap_remove_pdb_attribute(premake.vstudio.vc200x.VCLinkerTool)
    premake.vstudio.vc200x.toolmap.VCLinkerTool = premake.vstudio.vc200x.VCLinkerTool -- this is important as well
    premake.vstudio.vc200x.VCCLCompilerTool = wrap_remove_pdb_attribute(premake.vstudio.vc200x.VCCLCompilerTool)
    premake.vstudio.vc200x.toolmap.VCCLCompilerTool = premake.vstudio.vc200x.VCCLCompilerTool -- this is important as well
    -- Override the object directory paths ... don't make them "unique" inside premake4
    local orig_gettarget = premake.gettarget
    premake.gettarget = function(cfg, direction, pathstyle, namestyle, system)
        local r = orig_gettarget(cfg, direction, pathstyle, namestyle, system)
        if (cfg.objectsdir) and (cfg.objdir) then
            cfg.objectsdir = cfg.objdir
        end
        return r
    end
    -- Silently suppress generation of the .user files ...
    local orig_generate = premake.generate
    premake.generate = function(obj, filename, callback)
        if filename:find('.vcproj.user') or filename:find('.vcxproj.user') then
            return
        end
        orig_generate(obj, filename, callback)
    end
    -- Fix up premake.getlinks() to not do stupid stuff with object files we pass
    local orig_premake_getlinks = premake.getlinks
    premake.getlinks = function(cfg, kind, part)
        local origret = orig_premake_getlinks(cfg, kind, part)
        local ret = {}
        for k,v in ipairs(origret) do
            local dep = v:gsub(".obj.lib", ".obj")
            dep = dep:gsub(".lib.lib", ".lib")
            table.insert(ret, dep)
        end
        return ret
    end
end
local function transformMN(input) -- transform the macro names for older Visual Studio versions
    local new_map   = { vs2002 = 0, vs2003 = 0, vs2005 = 0, vs2008 = 0 }
    local replacements = { Platform = "PlatformName", Configuration = "ConfigurationName" }
    if new_map[action] ~= nil then
        for k,v in pairs(replacements) do
            if input:find(k) then
                input = input:gsub(k, v)
            end
        end
    end
    return input
end
newoption { trigger = "release", description = "Creates a solution suitable for a release build." }
newoption { trigger = "msvcrt", description = "Uses the Vista/2008 WDK to link dynamically against msvcrt.dll." }
newoption { trigger = "cmdline", description = "Also creates the project for the command line tool." }
newoption { trigger = "xp", description = "Enable XP-compatible build for newer Visual Studio versions." }
if _OPTIONS["msvcrt"] then
    local basedir = os.getenv("WLHBASE")
    print("BASEDIR = " .. basedir)
    if os.isdir(basedir) then
        print("Using value of WLHBASE=" .. basedir .. " as WDK location to find msvcrt import library.")
    else
        print("ERROR: WLHBASE not set or base directory " .. (basedir or "<null>") .. "does not exist.")
        os.exit(1)
    end
end

solution (tgtname .. iif(release, "_release", ""))
    configurations  (iif(release, {"Release"}, {"Debug", "Release"}))
    platforms       (iif(_ACTION < "vs2005", {"x32"}, {"x32", "x64"}))
    location        ('.')

    project (tgtname .. iif(release, "_release", ""))
        local int_dir   = iif(release, "release_", "").."intermediate/" .. action .. "_$(" .. transformMN("Platform") .. ")_$(" .. transformMN("Configuration") .. ")\\$(ProjectName)"
        uuid            ("DE5A7539-C36C-4F2E-9CE3-18087DC72346")
        language        ("C++")
        kind            ("WindowedApp")
        targetname      (tgtname)
        flags           {"Unicode", "NativeWChar", "ExtraWarnings", "WinMain",}
        targetdir       (iif(release, tgtname .. "_release", "build." .. action))
        includedirs     {"wtl/Include", "pugixml"}
        objdir          (int_dir)
        libdirs         {"$(IntDir)"}
        links           {"ntdll-delayed", "version"}
        resoptions      {"/nologo", "/l409"}
        resincludedirs  {".", "$(IntDir)"}
        linkoptions     {"/delay:nobind","/delayload:ntdll-delayed.dll","/delayload:version.dll"}
        defines         {"WIN32", "_WINDOWS", "STRICT"}
        if not _OPTIONS["msvcrt"] then
            flags       {"StaticRuntime"}
        end

        excludes
        {
            "ntobjx_c.cpp",
            "ntobjx.txt",
            "hgid.h",
            "Backup*.*",
        }

        files
        {
            "ntdll-stubs/*.txt",
            "wtl/Include/*.h",
            "pugixml/*.hpp",
            "util/*.h", "util/*.hpp",
            "res/*.ico",
            "*.rc",
            "*.cpp",
            "*.h",
            "*.hpp",
            "*.manifest",
            "*.cmd", "*.txt", "*.md", "*.rst", "premake4.lua",
        }

        vpaths
        {
            ["Header Files/WTL/*"] = { "wtl/Include/*.h" },
            ["Header Files/pugixml/*"] = { "pugixml/*.hpp" },
            ["Header Files/Utility classes/*"] = { "util/*.h", "util/*.hpp" },
            ["Header Files/*"] = { "*.h" },
            ["Resource Files/*"] = { "**.rc", "res/*.ico" },
            ["Source Files/*"] = { "*.cpp" },
            ["Special Files/*"] = { "**.cmd", "premake4.lua", "**.manifest", },
            ["Special Files/Module Definition Files/*"] = { "ntdll-stubs/*.txt", },
        }

        configuration {"*"}
            prebuildcommands{"call \"$(ProjectDir)\\hgid.cmd\"",}
            buildoptions    {"/GS"}
            linkoptions     {"/dynamicbase","/nxcompat"}

        configuration {"x64"}
            prebuildcommands{"lib.exe /nologo /nodefaultlib \"/def:ntdll-stubs\\ntdll-delayed.txt\" \"/out:$(IntDir)\\ntdll-delayed.lib\" /machine:x64",}

        configuration {"x32"}
            prebuildcommands{"cl.exe /nologo /c /TC /Ob0 /Gz ntdll-stubs\\ntdll-delayed-stubs.c \"/Fo$(IntDir)\\ntdll-delayed-stubs.obj\"", "lib.exe /nologo \"/def:ntdll-stubs\\ntdll-delayed.txt\" \"/out:$(IntDir)\\ntdll-delayed.lib\" /machine:x86 \"$(IntDir)\\ntdll-delayed-stubs.obj\"",}

        configuration {"Debug", "x32"}
            targetsuffix    ("32D")

        configuration {"Debug", "x64"}
            targetsuffix    ("64D")

        configuration {"Release", "x32"}
            targetsuffix    ("32")
            linkoptions     {"/safeseh"}
            if _OPTIONS["msvcrt"] then
                links       {"comdlg32", "$(WLHBASE)\\lib\\crt\\i386\\msvcrt.lib", "$(WLHBASE)\\lib\\w2k\\i386\\strsafe.lib", "$(WLHBASE)\\lib\\w2k\\i386\\msvcrt_win2000.obj"}
                defines     {"USE_W2K_COMPAT"}
            end

        configuration {"Release", "x64"}
            targetsuffix    ("64")
            if _OPTIONS["msvcrt"] then
                links       {"comdlg32", "$(WLHBASE)\\lib\\crt\\amd64\\msvcrt.lib", "$(WLHBASE)\\lib\\wnet\\amd64\\strsafe.lib", "$(WLHBASE)\\lib\\wnet\\amd64\\msvcrt_win2003.obj"}
                defines     {"USE_W2K_COMPAT"}
            end

        configuration {"Debug"}
            defines         {"_DEBUG"}
            flags           {"Symbols",}

        configuration {"Release"}
            defines         {"NDEBUG"}
            flags           {"Optimize", "Symbols", "NoMinimalRebuild", "NoIncrementalLink", "NoEditAndContinue"}
            linkoptions     {"/release"}
            buildoptions    {"/Oi", "/Os", "/Gy"}

        configuration {"vs2002 or vs2003 or vs2005 or vs2008", "Release"}
            buildoptions    {"/Oy"}

        configuration {"vs2002 or vs2003 or vs2005 or vs2008 or vs2010", "Release", "x32"}
            linkoptions     {"/subsystem:windows,5.00"}

        configuration {"vs2012 or vs2013 or vs2015 or vs2017", "Release", "x32"}
            linkoptions     {"/subsystem:windows,5.01"}

        configuration {"Release", "x64"}
            linkoptions     {"/subsystem:windows,5.02"}

        configuration {"vs2013 or vs2015 or vs2017"}
            defines         {"WINVER=0x0501", "_ALLOW_RTCc_IN_STL"}

        configuration {"vs2002 or vs2003 or vs2005 or vs2008 or vs2010 or vs2012", "x32"}
            defines         {"WINVER=0x0500"}

        configuration {"vs2005 or vs2008 or vs2010 or vs2012", "x64"}
            defines         {"WINVER=0x0501"}

        configuration {"vs2005 or vs2008", "Release"}
            linkoptions     {"/opt:nowin98"}

    if cmdline then
        -- ntobjx_c project
        project (tgtname .. "_c" .. iif(release, "_release", ""))
            local int_dir   = iif(release, "release_", "").."intermediate/" .. action .. "_$(" .. transformMN("Platform") .. ")_$(" .. transformMN("Configuration") .. ")\\$(ProjectName)"
            uuid            ("CA1F91D0-7D7E-4A9C-9DD1-6C65C2F37A59")
            language        ("C++")
            kind            ("ConsoleApp")
            targetname      (tgtname .. "_c")
            flags           {"StaticRuntime", "Unicode", "NativeWChar", "ExtraWarnings", "WinMain", "NoMinimalRebuild", "NoIncrementalLink", "NoEditAndContinue", "NoPCH"}
            targetdir       (iif(release, tgtname .. "_release", "build." .. action))
            objdir          (int_dir)
            links           {"ntdll-delayed"}
            resoptions      {"/nologo", "/l409"}
            resincludedirs  {".", "$(IntDir)"}
            linkoptions     {"\"/libpath:$(IntDir)\"", "/delay:nobind","/delayload:ntdll-delayed.dll"}
            defines         {"WIN32", "_WINDOWS", "STRICT", "_CONSOLE", "_ALLOW_RTCc_IN_STL"}

            files
            {
                "ntdll-stubs/*.txt",
                "ntnative.h",
                "*.cpp",
                "*.hpp",
                "*.cmd", "*.txt", "*.md", "*.rst", "premake4.lua",
            }

            excludes
            {
                "ntobjx.cpp",
                "hgid.h",
                "fake_commodule.hpp",
                "stdafx.*", "ntobjx.txt",
                "reslang.*", "resource.h",
            }

            vpaths
            {
                ["Header Files/*"] = { "*.h" },
                ["Source Files/*"] = { "*.cpp" },
                ["Special Files/*"] = { "**.cmd", "premake4.lua", "**.manifest", },
                ["Special Files/Module Definition Files/*"] = { "ntdll-stubs/*.txt", },
            }

            configuration {"Debug", "x32"}
                targetsuffix    ("32D")

            configuration {"Debug", "x64"}
                targetsuffix    ("64D")

            configuration {"Release", "x32"}
                targetsuffix    ("32")

            configuration {"Release", "x64"}
                targetsuffix    ("64")

            configuration {"Debug"}
                flags           {"Symbols"}

            configuration {"x64"}
                prebuildcommands{"lib.exe /nologo /nodefaultlib \"/def:ntdll-stubs\\ntdll-delayed.txt\" \"/out:$(IntDir)\\ntdll-delayed.lib\" /machine:x64", "\"$(ProjectDir)\\hgid.cmd\""}

            configuration {"x32"}
                prebuildcommands{"cl.exe /nologo /c /TC /Ob0 /Gz ntdll-stubs\\ntdll-delayed-stubs.c \"/Fo$(IntDir)\\ntdll-delayed-stubs.obj\"", "lib.exe /nologo \"/def:ntdll-stubs\\ntdll-delayed.txt\" \"/out:$(IntDir)\\ntdll-delayed.lib\" /machine:x86 \"$(IntDir)\\ntdll-delayed-stubs.obj\"", "\"$(ProjectDir)\\hgid.cmd\""}

            configuration {"Release"}
                defines         ("NDEBUG")
                flags           {"Optimize", "Symbols"}
                linkoptions     {"/release"}
                buildoptions    {"/Oi", "/Os", "/Gy"}

            configuration {"vs2002 or vs2003 or vs2005 or vs2008", "Release"}
                buildoptions    {"/Oy"}

            configuration {"vs2002 or vs2003 or vs2005 or vs2008 or vs2010", "Release", "x32"}
                linkoptions     {"/subsystem:windows,5.00"}

            configuration {"vs2012 or vs2013 or vs2015 or vs2017", "Release", "x32"}
                linkoptions     {"/subsystem:windows,5.01"}

            configuration {"Release", "x64"}
                linkoptions     {"/subsystem:console,5.02"}

            configuration {"vs2013 or vs2015 or vs2017"}
                defines         {"WINVER=0x0501"}

            configuration {"vs2002 or vs2003 or vs2005 or vs2008 or vs2010 or vs2012", "x32"}
                defines         {"WINVER=0x0500"}

            configuration {"vs2005 or vs2008 or vs2010 or vs2012", "x64"}
                defines         {"WINVER=0x0501"}

            configuration {"vs2005 or vs2008", "Release"}
                linkoptions     {"/opt:nowin98"}
    end