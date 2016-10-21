--[[
        This premake4.lua _requires_ windirstat/premake-stable to work properly.
        If you don't want to use the code-signed build that can be found in the
        ./common/ subfolder, you can build from the WDS-branch over at:

        https://bitbucket.org/windirstat/premake-stable
  ]]
local action = _ACTION or ""
local release = false
local cmdline = false
local slnname = ""
local pfx = ""
if _OPTIONS["cmdline"] then
    cmdline = true
end
if _OPTIONS["release"] then
    print "INFO: Creating release build solution."
    release = true
    slnname = "ntobjx_release"
    pfx = slnname .. "_"
    _OPTIONS["release"] = pfx
    cmdline = false
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
    -- Name the project files after their VS version
    local orig_getbasename = premake.project.getbasename
    premake.project.getbasename = function(prjname, pattern)
        -- The below is used to insert the .vs(8|9|10|11|12|14) into the file names for projects and solutions
        if _ACTION then
            name_map = {vs2002 = "vs7", vs2003 = "vs7_1", vs2005 = "vs8", vs2008 = "vs9", vs2010 = "vs10", vs2012 = "vs11", vs2013 = "vs12", vs2015 = "vs14"}
            if name_map[_ACTION] then
                pattern = pattern:gsub("%%%%", "%%%%." .. name_map[_ACTION])
            else
                pattern = pattern:gsub("%%%%", "%%%%." .. _ACTION)
            end
        end
        return orig_getbasename(prjname, pattern)
    end
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
newoption { trigger = "cmdline", description = "Also creates the project for the command line tool." }

solution (iif(release, slnname, "ntobjx"))
    configurations  (iif(release, {"Release"}, {"Debug", "Release"}))
    platforms       (iif(_ACTION < "vs2005", {"x32"}, {"x32", "x64"}))
    location        ('.')

    -- Main ntobjx project
    project (iif(release, slnname, "ntobjx"))
        local int_dir   = pfx.."intermediate/" .. action .. "_$(" .. transformMN("Platform") .. ")_$(" .. transformMN("Configuration") .. ")\\$(ProjectName)"
        uuid            ("DE5A7539-C36C-4F2E-9CE3-18087DC72346")
        language        ("C++")
        kind            ("WindowedApp")
        targetname      ("ntobjx")
        flags           {"StaticRuntime", "Unicode", "NativeWChar", "ExtraWarnings", "WinMain", "NoMinimalRebuild", "NoIncrementalLink", "NoEditAndContinue"}
        targetdir       (iif(release, slnname, "build"))
        includedirs     {"wtl/Include", "pugixml"}
        objdir          (int_dir)
        links           {"ntdll-delayed", "version"}
        resoptions      {"/nologo", "/l409"}
        resincludedirs  {".", "$(IntDir)"}
        linkoptions     {"\"/libpath:$(IntDir)\"", "/pdbaltpath:%_PDB%", "/delay:nobind","/delayload:ntdll-delayed.dll","/delayload:version.dll"}
        defines         {"WIN32", "_WINDOWS", "STRICT"}

        files
        {
            "wtl/Include/*.h",
            "ntdll-stubs/*.txt",
            "pugixml/*.hpp",
            "*.h", "util/*.h", "util/*.hpp",
            "*.rc",
            "*.cpp",
            "*.hpp",
            "*.manifest",
            "*.cmd", "*.txt", "*.md", "*.rst", "premake4.lua",
        }

        excludes
        {
            "ntobjx_c.cpp",
            "hgid.h", "ntobjx.txt",
        }

        vpaths
        {
            ["Header Files/WTL/*"] = { "wtl/Include/*.h" },
            ["Header Files/Utility classes/*"] = { "util/*.h", "util/*.hpp" },
            ["Header Files/*"] = { "*.h" },
            ["Resource Files/*"] = { "**.rc" },
            ["Resource Files/res/*"] = { "res/*.*" },
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

        configuration {"vs2012 or vs2013 or vs2015", "Release", "x32"}
            linkoptions     {"/subsystem:windows,5.01"}

        configuration {"Release", "x64"}
            linkoptions     {"/subsystem:windows,5.02"}

        configuration {"vs2013 or vs2015"}
            defines         {"WINVER=0x0501"}

        configuration {"vs2002 or vs2003 or vs2005 or vs2008 or vs2010 or vs2012", "x32"}
            defines         {"WINVER=0x0500"}

        configuration {"vs2005 or vs2008 or vs2010 or vs2012", "x64"}
            defines         {"WINVER=0x0501"}

        configuration {"vs2005 or vs2008", "Release"}
            linkoptions     {"/opt:nowin98"}

    if cmdline then
        -- ntobjx_c project
        project (iif(release, slnname, "ntobjx_c"))
            local int_dir   = pfx.."intermediate/" .. action .. "_$(" .. transformMN("Platform") .. ")_$(" .. transformMN("Configuration") .. ")\\$(ProjectName)"
            uuid            ("CA1F91D0-7D7E-4A9C-9DD1-6C65C2F37A59")
            language        ("C++")
            kind            ("ConsoleApp")
            targetname      ("ntobjx_c")
            flags           {"StaticRuntime", "Unicode", "NativeWChar", "ExtraWarnings", "WinMain", "NoMinimalRebuild", "NoIncrementalLink", "NoEditAndContinue"}
            targetdir       (iif(release, slnname, "build"))
            objdir          (int_dir)
            links           {"ntdll-delayed"}
            resoptions      {"/nologo", "/l409"}
            resincludedirs  {".", "$(IntDir)"}
            linkoptions     {"\"/libpath:$(IntDir)\"", "/pdbaltpath:%_PDB%", "/delay:nobind","/delayload:ntdll-delayed.dll"}
            defines         {"WIN32", "_WINDOWS", "STRICT", "_CONSOLE"}

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

            configuration {"vs2012 or vs2013 or vs2015", "Release", "x32"}
                linkoptions     {"/subsystem:windows,5.01"}

            configuration {"Release", "x64"}
                linkoptions     {"/subsystem:console,5.02"}

            configuration {"vs2013 or vs2015"}
                defines         {"WINVER=0x0501"}

            configuration {"vs2002 or vs2003 or vs2005 or vs2008 or vs2010 or vs2012", "x32"}
                defines         {"WINVER=0x0500"}

            configuration {"vs2005 or vs2008 or vs2010 or vs2012", "x64"}
                defines         {"WINVER=0x0501"}

            configuration {"vs2005 or vs2008", "Release"}
                linkoptions     {"/opt:nowin98"}
    end