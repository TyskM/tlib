#pragma once

#ifdef SCRIPTBUILDER_H
static_assert(false, "Don't include the original script builder");
#endif

#include "../thirdparty/angelscript/angelscript.h"
#include "../String.hpp"
#include "../Macros.hpp"
#include "Logging.hpp"

#include <vector>
#include <set>
using namespace std;

#include <stdio.h>
#if defined(_MSC_VER) && !defined(_WIN32_WCE) && !defined(__S3E__)
#include <direct.h>
#endif
#ifdef _WIN32_WCE
#include <windows.h> // For GetModuleFileName()
#endif

#if defined(__S3E__) || defined(__APPLE__) || defined(__GNUC__)
#include <unistd.h> // For getcwd()
#endif

struct ScriptBuilder;

// This callback will be called for each #include directive encountered by the
// builder. The callback should call the AddSectionFromFile or AddSectionFromMemory
// to add the included section to the script. If the include cannot be resolved
// then the function should return a negative value to abort the compilation.
typedef int (*INCLUDECALLBACK_t)(const char* include, const char* from, ScriptBuilder* builder, void* userParam);

// This callback will be called for each #pragma directive encountered by the builder.
// The application can interpret the pragmaText and decide what do to based on that.
// If the callback returns a negative value the builder will report an error and abort the compilation.
typedef int(*PRAGMACALLBACK_t)(const std::string& pragmaText, ScriptBuilder& builder, void* userParam);

// Helper functions
static String GetCurrentDir();
static String GetAbsolutePath(const String& path);

// CScriptBuilder extended to support more preprocessor macros
struct ScriptBuilder
{
    asIScriptEngine* engine;
    asIScriptModule* module;
    std::string                modifiedScript;

    INCLUDECALLBACK_t  includeCallback;
    void* includeParam;

    PRAGMACALLBACK_t  pragmaCallback;
    void* pragmaParam;

    #if AS_PROCESS_METADATA == 1
    int  ExtractMetadata(int pos, std::vector<std::string>& outMetadata);
    int  ExtractDeclaration(int pos, std::string& outName, std::string& outDeclaration, int& outType);

    enum METADATATYPE
    {
        MDT_TYPE = 1,
        MDT_FUNC = 2,
        MDT_VAR = 3,
        MDT_VIRTPROP = 4,
        MDT_FUNC_OR_VAR = 5
    };

    // Temporary structure for storing metadata and declaration
    struct SMetadataDecl
    {
        SMetadataDecl(std::vector<std::string> m, std::string n, std::string d, int t, std::string c, std::string ns) : metadata(m), name(n), declaration(d), type(t), parentClass(c), nameSpace(ns) {}
        std::vector<std::string> metadata;
        std::string              name;
        std::string              declaration;
        int                      type;
        std::string              parentClass;
        std::string              nameSpace;
    };
    std::vector<SMetadataDecl> foundDeclarations;
    std::string currentClass;
    std::string currentNamespace;

    // Storage of metadata for global declarations
    std::map<int, std::vector<std::string> > typeMetadataMap;
    std::map<int, std::vector<std::string> > funcMetadataMap;
    std::map<int, std::vector<std::string> > varMetadataMap;

    // Storage of metadata for class member declarations
    struct SClassMetadata
    {
        SClassMetadata(const std::string& aName) : className(aName) {}
        std::string className;
        std::map<int, std::vector<std::string> > funcMetadataMap;
        std::map<int, std::vector<std::string> > varMetadataMap;
    };
    std::map<int, SClassMetadata> classMetadataMap;

    #endif

    #ifdef _WIN32
    // On Windows the filenames are case insensitive so the comparisons to
    // avoid duplicate includes must also be case insensitive. True case insensitive
    // is not easy as it must be language aware, but a simple implementation such
    // as strcmpi should suffice in almost all cases.
    //
    // ref: http://www.gotw.ca/gotw/029.htm
    // ref: https://msdn.microsoft.com/en-us/library/windows/desktop/dd317761(v=vs.85).aspx
    // ref: http://site.icu-project.org/

    // TODO: Strings by default are treated as UTF8 encoded. If the application choses to
    //       use a different encoding, the comparison algorithm should be adjusted as well

    struct ci_less
    {
        bool operator()(const std::string& a, const std::string& b) const
        {
            return _stricmp(a.c_str(), b.c_str()) < 0;
        }
    };
    std::set<std::string, ci_less> includedScripts;
    #else
    std::set<std::string>      includedScripts;
    #endif

    std::set<std::string>      definedWords;

    ScriptBuilder()
    {
        engine = 0;
        module = 0;
    
        includeCallback = 0;
        includeParam = 0;
    
        pragmaCallback = 0;
        pragmaParam = 0;
    }
    
    void setIncludeCallback(INCLUDECALLBACK_t callback, void* userParam)
    {
        includeCallback = callback;
        includeParam   = userParam;
    }
    
    void setPragmaCallback(PRAGMACALLBACK_t callback, void* userParam)
    {
        pragmaCallback = callback;
        pragmaParam = userParam;
    }
    
    int startNewModule(asIScriptEngine* inEngine, const char* moduleName)
    {
        if (inEngine == 0) return -1;
    
        engine = inEngine;
        module = inEngine->GetModule(moduleName, asGM_ALWAYS_CREATE);
        if (module == 0)
            return -1;
    
        clearAll();
    
        return 0;
    }
    
    asIScriptEngine* getEngine()
    {
        return engine;
    }
    
    asIScriptModule* getModule()
    {
        return module;
    }
    
    unsigned int getSectionCount() const
    {
        return (unsigned int)(includedScripts.size());
    }
    
    String getSectionName(unsigned int idx) const
    {
        if (idx >= includedScripts.size()) return "";
    
        #ifdef _WIN32
        set<String, ci_less>::const_iterator it = includedScripts.begin();
        #else
        set<String>::const_iterator it = includedScripts.begin();
        #endif
        while (idx-- > 0) it++;
        return *it;
    }
    
    // Returns 1 if the section was included
    // Returns 0 if the section was not included because it had already been included before
    // Returns <0 if there was an error
    int addSectionFromFile(const char* filename)
    {
        // The file name stored in the set should be the fully resolved name because
        // it is possible to name the same file in multiple ways using relative paths.
        String fullpath = GetAbsolutePath(filename);
    
        if (includeIfNotAlreadyIncluded(fullpath.c_str()))
        {
            int r = loadScriptSection(fullpath.c_str());
            if (r < 0)
                return r;
            else
                return 1;
        }
    
        return 0;
    }
    
    // Returns 1 if the section was included
    // Returns 0 if the section was not included because it had already been included before
    // Returns <0 if there was an error
    int addSectionFromMemory(const char* sectionName, const char* scriptCode, unsigned int scriptLength, int lineOffset)
    {
        if (includeIfNotAlreadyIncluded(sectionName))
        {
            int r = processScriptSection(scriptCode, scriptLength, sectionName, lineOffset);
            if (r < 0)
                return r;
            else
                return 1;
        }
    
        return 0;
    }
    
    int buildModule()
    {
        return build();
    }
    
    void defineWord(const char* word)
    {
        String sword = word;
        if (definedWords.find(sword) == definedWords.end())
        {
            //selog->info("Defined word: {}", sword);
            definedWords.insert(sword);
        }
    }
    
    void clearAll()
    {
        includedScripts.clear();
    
        #if AS_PROCESS_METADATA == 1
        currentClass = "";
        currentNamespace = "";
    
        foundDeclarations.clear();
        typeMetadataMap.clear();
        funcMetadataMap.clear();
        varMetadataMap.clear();
        #endif
    }
    
    bool includeIfNotAlreadyIncluded(const char* filename)
    {
        String scriptFile = filename;
        if (includedScripts.find(scriptFile) != includedScripts.end())
        {
            // Already included
            return false;
        }
    
        // Add the file to the set of included sections
        includedScripts.insert(scriptFile);
    
        return true;
    }
    
    int loadScriptSection(const char* filename)
    {
        // Open the script file
        String scriptFile = filename;
        #if _MSC_VER >= 1500 && !defined(__S3E__)
        FILE* f = 0;
        fopen_s(&f, scriptFile.c_str(), "rb");
        #else
        FILE* f = fopen(scriptFile.c_str(), "rb");
        #endif
        if (f == 0)
        {
            // Write a message to the engine's message callback
            String msg = "Failed to open script file '" + GetAbsolutePath(scriptFile) + "'";
            engine->WriteMessage(filename, 0, 0, asMSGTYPE_ERROR, msg.c_str());
    
            // TODO: Write the file where this one was included from
    
            return -1;
        }
    
        // Determine size of the file
        fseek(f, 0, SEEK_END);
        int len = ftell(f);
        fseek(f, 0, SEEK_SET);
    
        // On Win32 it is possible to do the following instead
        // int len = _filelength(_fileno(f));
    
        // Read the entire file
        String code;
        size_t c = 0;
        if (len > 0)
        {
            code.resize(len);
            c = fread(&code[0], len, 1, f);
        }
    
        fclose(f);
    
        if (c == 0 && len > 0)
        {
            // Write a message to the engine's message callback
            String msg = "Failed to load script file '" + GetAbsolutePath(scriptFile) + "'";
            engine->WriteMessage(filename, 0, 0, asMSGTYPE_ERROR, msg.c_str());
            return -1;
        }
    
        // Process the script section even if it is zero length so that the name is registered
        return processScriptSection(code.c_str(), (unsigned int)(code.length()), filename, 0);
    }
    
    int processScriptSection(const char* script, unsigned int length, const char* sectionname, int lineOffset)
    {
        vector<String> includes;
    
        // Perform a superficial parsing of the script first to store the metadata
        if (length)
            modifiedScript.assign(script, length);
        else
            modifiedScript = script;
    
        // First perform the checks for #if directives to exclude code that shouldn't be compiled
        unsigned int pos = 0;
        int nested = 0;
        while (pos < modifiedScript.size())
        {
            asUINT len = 0;
            asETokenClass t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
            if (t == asTC_UNKNOWN && modifiedScript[pos] == '#' && (pos + 1 < modifiedScript.size()))
            {
                int start = pos++;
    
                // Is this an #if directive?
                t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
    
                String token;
                token.assign(&modifiedScript[pos], len);
    
                pos += len;
                //selog->info("Found token: {}", token);
                if (token == "if")
                {
                    t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
                    if (t == asTC_WHITESPACE)
                    {
                        pos += len;
                        t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
                    }

                    if (t == asTC_IDENTIFIER)
                    {
                        String word;
                        word.assign(&modifiedScript[pos], len);
                        //selog->info("Found word: {}", word);
                        // Overwrite the #if directive with space characters to avoid compiler error
                        pos += len;
                        overwriteCode(start, pos-start);

                        // Has this identifier been defined by the application or not?
                        if (definedWords.find(word) == definedWords.end())
                        {
                            // Exclude all the code until and including the #endif
                            pos = excludeCode(pos);
                        }
                        else
                        {
                            nested++;
                        }
                    }
                }
                else if (token == "endif")
                {
                    // Only remove the #endif if there was a matching #if
                    if (nested > 0)
                    {
                        overwriteCode(start, pos-start);
                        nested--;
                    }
                }

                
                else if (token == "pragma")
                {
                    pos += len;
                    for (; pos < modifiedScript.size() && modifiedScript[pos] != '\n'; pos++);
                    
                    // Ptr, count
                    // 7?? or 8????
                    String pragmaText(&modifiedScript[start + 8], pos - start - 8);
                    //selog->info("Found pragma: {}, {}", pragmaText, pragmaText.size());

                    // Sets the macro and everything after too blank.
                    if (pragmaText == "break")
                    { overwriteCode(start, modifiedScript.size() - start); }
                    // Remove the pragma
                    else
                    { overwriteCode(start, pos - start); }
                }
            }
            else
            { pos += len; }
        }
    
        #if AS_PROCESS_METADATA == 1
        // Preallocate memory
    String name, declaration;
    vector<String> metadata;
    declaration.reserve(100);
        #endif
    
            // Then check for meta data and pre-processor directives
        pos = 0;
        while (pos < modifiedScript.size())
    {
        asUINT len = 0;
        asETokenClass t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
        if (t == asTC_COMMENT || t == asTC_WHITESPACE)
        {
            pos += len;
            continue;
        }
        String token;
        token.assign(&modifiedScript[pos], len);

        #if AS_PROCESS_METADATA == 1
                // Skip possible decorators before class and interface declarations
        if (token == "shared" || token == "abstract" || token == "mixin" || token == "external")
        {
            pos += len;
            continue;
        }

        // Check if class or interface so the metadata for members can be gathered
        if (currentClass == "" && (token == "class" || token == "interface"))
        {
            // Get the identifier after "class"
            do
            {
                pos += len;
                if (pos >= modifiedScript.size())
                {
                    t = asTC_UNKNOWN;
                    break;
                }
                t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
            } while (t == asTC_COMMENT || t == asTC_WHITESPACE);

            if (t == asTC_IDENTIFIER)
            {
                currentClass = modifiedScript.substr(pos, len);

                // Search until first { or ; is encountered
                while (pos < modifiedScript.length())
                {
                    engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);

                    // If start of class section encountered stop
                    if (modifiedScript[pos] == '{')
                    {
                        pos += len;
                        break;
                    }
                    else if (modifiedScript[pos] == ';')
                    {
                        // The class declaration has ended and there are no children
                        currentClass = "";
                        pos += len;
                        break;
                    }

                    // Check next symbol
                    pos += len;
                }
            }

            continue;
        }

        // Check if end of class
        if (currentClass != "" && token == "}")
        {
            currentClass = "";
            pos += len;
            continue;
        }

        // Check if namespace so the metadata for members can be gathered
        if (token == "namespace")
        {
            // Get the identifier after "namespace"
            do
            {
                pos += len;
                t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
            } while (t == asTC_COMMENT || t == asTC_WHITESPACE);

            if (currentNamespace != "")
                currentNamespace += "::";
            currentNamespace += modifiedScript.substr(pos, len);

            // Search until first { is encountered
            while (pos < modifiedScript.length())
            {
                engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);

                // If start of namespace section encountered stop
                if (modifiedScript[pos] == '{')
                {
                    pos += len;
                    break;
                }

                // Check next symbol
                pos += len;
            }

            continue;
        }

        // Check if end of namespace
        if (currentNamespace != "" && token == "}")
        {
            size_t found = currentNamespace.rfind("::");
            if (found != String::npos)
            {
                currentNamespace.erase(found);
            }
            else
            {
                currentNamespace = "";
            }
            pos += len;
            continue;
        }

        // Is this the start of metadata?
        if (token == "[")
        {
            // Get the metadata String
            pos = ExtractMetadata(pos, metadata);

            // Determine what this metadata is for
            int type;
            ExtractDeclaration(pos, name, declaration, type);

            // Store away the declaration in a map for lookup after the build has completed
            if (type > 0)
            {
                SMetadataDecl decl(metadata, name, declaration, type, currentClass, currentNamespace);
                foundDeclarations.push_back(decl);
            }
        }
        else
            #endif
                    // Is this a preprocessor directive?
            if (token == "#" && (pos + 1 < modifiedScript.size()))
            {
                int start = pos++;

                t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
                if (t == asTC_IDENTIFIER)
                {
                    token.assign(&modifiedScript[pos], len);
                    if (token == "include")
                    {
                        pos += len;
                        t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
                        if (t == asTC_WHITESPACE)
                        {
                            pos += len;
                            t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
                        }

                        if (t == asTC_VALUE && len > 2 && (modifiedScript[pos] == '"' || modifiedScript[pos] == '\''))
                        {
                            // Get the include file
                            String includefile;
                            includefile.assign(&modifiedScript[pos + 1], len - 2);
                            pos += len;

                            // Store it for later processing
                            includes.push_back(includefile);

                            // Overwrite the include directive with space characters to avoid compiler error
                            overwriteCode(start, pos - start);
                        }
                    }
                    else if (token == "pragma")
                    {
                        // Read until the end of the line
                        pos += len;
                        for (; pos < modifiedScript.size() && modifiedScript[pos] != '\n'; pos++);

                        // Call the pragma callback
                        String pragmaText(&modifiedScript[start + 7], pos - start - 7);
                        int r = pragmaCallback ? pragmaCallback(pragmaText, *this, pragmaParam) : -1;
                        if (r < 0)
                        {
                            // TODO: Report the correct line number
                            engine->WriteMessage(sectionname, 0, 0, asMSGTYPE_ERROR, "Invalid #pragma directive");
                            return r;
                        }

                        // Overwrite the pragma directive with space characters to avoid compiler error
                        overwriteCode(start, pos - start);
                    }
                }
                else
                {
                    // Check for lines starting with #!, e.g. shebang interpreter directive. These will be treated as comments and removed by the preprocessor
                    if (modifiedScript[pos] == '!')
                    {
                        // Read until the end of the line
                        pos += len;
                        for (; pos < modifiedScript.size() && modifiedScript[pos] != '\n'; pos++);

                        // Overwrite the directive with space characters to avoid compiler error
                        overwriteCode(start, pos - start);
                    }
                }
            }
            // Don't search for metadata/includes within statement blocks or between tokens in statements
            else
            {
                pos = skipStatement(pos);
            }
    }
    
        // Build the actual script
        engine->SetEngineProperty(asEP_COPY_SCRIPT_SECTIONS, true);
        module->AddScriptSection(sectionname, modifiedScript.c_str(), modifiedScript.size(), lineOffset);
    
        if (includes.size() > 0)
        {
            // If the callback has been set, then call it for each included file
            if (includeCallback)
            {
                for (int n = 0; n < (int)includes.size(); n++)
                {
                    int r = includeCallback(includes[n].c_str(), sectionname, this, includeParam);
                    if (r < 0)
                        return r;
                }
            }
            else
            {
                // By default we try to load the included file from the relative directory of the current file
    
                // Determine the path of the current script so that we can resolve relative paths for includes
                String path = sectionname;
                size_t posOfSlash = path.find_last_of("/\\");
                if (posOfSlash != String::npos)
                    path.resize(posOfSlash+1);
                else
                    path = "";
    
                // Load the included scripts
                for (int n = 0; n < (int)includes.size(); n++)
                {
                    // If the include is a relative path, then prepend the path of the originating script
                    if (includes[n].find_first_of("/\\") != 0 &&
                        includes[n].find_first_of(":") == String::npos)
                    {
                        includes[n] = path + includes[n];
                    }
    
                    // Include the script section
                    int r = addSectionFromFile(includes[n].c_str());
                    if (r < 0)
                        return r;
                }
            }
        }
    
        return 0;
    }
    
    int build()
    {
        int r = module->Build();
        if (r < 0)
            return r;
    
        #if AS_PROCESS_METADATA == 1
        // After the script has been built, the metadata strings should be
        // stored for later lookup by function id, type id, and variable index
    for (int n = 0; n < (int)foundDeclarations.size(); n++)
    {
        SMetadataDecl* decl = &foundDeclarations[n];
        module->SetDefaultNamespace(decl->nameSpace.c_str());
        if (decl->type == MDT_TYPE)
        {
            // Find the type id
            int typeId = module->GetTypeIdByDecl(decl->declaration.c_str());
            assert(typeId >= 0);
            if (typeId >= 0)
                typeMetadataMap.insert(map<int, vector<String> >::value_type(typeId, decl->metadata));
        }
        else if (decl->type == MDT_FUNC)
        {
            if (decl->parentClass == "")
            {
                // Find the function id
                asIScriptFunction* func = module->GetFunctionByDecl(decl->declaration.c_str());
                assert(func);
                if (func)
                    funcMetadataMap.insert(map<int, vector<String> >::value_type(func->GetId(), decl->metadata));
            }
            else
            {
                // Find the method id
                int typeId = module->GetTypeIdByDecl(decl->parentClass.c_str());
                assert(typeId > 0);
                map<int, SClassMetadata>::iterator it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end())
                {
                    classMetadataMap.insert(map<int, SClassMetadata>::value_type(typeId, SClassMetadata(decl->parentClass)));
                    it = classMetadataMap.find(typeId);
                }

                asITypeInfo* type = engine->GetTypeInfoById(typeId);
                asIScriptFunction* func = type->GetMethodByDecl(decl->declaration.c_str());
                assert(func);
                if (func)
                    it->second.funcMetadataMap.insert(map<int, vector<String> >::value_type(func->GetId(), decl->metadata));
            }
        }
        else if (decl->type == MDT_VIRTPROP)
        {
            if (decl->parentClass == "")
            {
                // Find the global virtual property accessors
                asIScriptFunction* func = module->GetFunctionByName(("get_" + decl->declaration).c_str());
                if (func)
                    funcMetadataMap.insert(map<int, vector<String> >::value_type(func->GetId(), decl->metadata));
                func = module->GetFunctionByName(("set_" + decl->declaration).c_str());
                if (func)
                    funcMetadataMap.insert(map<int, vector<String> >::value_type(func->GetId(), decl->metadata));
            }
            else
            {
                // Find the method virtual property accessors
                int typeId = module->GetTypeIdByDecl(decl->parentClass.c_str());
                assert(typeId > 0);
                map<int, SClassMetadata>::iterator it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end())
                {
                    classMetadataMap.insert(map<int, SClassMetadata>::value_type(typeId, SClassMetadata(decl->parentClass)));
                    it = classMetadataMap.find(typeId);
                }

                asITypeInfo* type = engine->GetTypeInfoById(typeId);
                asIScriptFunction* func = type->GetMethodByName(("get_" + decl->declaration).c_str());
                if (func)
                    it->second.funcMetadataMap.insert(map<int, vector<String> >::value_type(func->GetId(), decl->metadata));
                func = type->GetMethodByName(("set_" + decl->declaration).c_str());
                if (func)
                    it->second.funcMetadataMap.insert(map<int, vector<String> >::value_type(func->GetId(), decl->metadata));
            }
        }
        else if (decl->type == MDT_VAR)
        {
            if (decl->parentClass == "")
            {
                // Find the global variable index
                int varIdx = module->GetGlobalVarIndexByName(decl->declaration.c_str());
                assert(varIdx >= 0);
                if (varIdx >= 0)
                    varMetadataMap.insert(map<int, vector<String> >::value_type(varIdx, decl->metadata));
            }
            else
            {
                int typeId = module->GetTypeIdByDecl(decl->parentClass.c_str());
                assert(typeId > 0);

                // Add the classes if needed
                map<int, SClassMetadata>::iterator it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end())
                {
                    classMetadataMap.insert(map<int, SClassMetadata>::value_type(typeId, SClassMetadata(decl->parentClass)));
                    it = classMetadataMap.find(typeId);
                }

                // Add the variable to class
                asITypeInfo* objectType = engine->GetTypeInfoById(typeId);
                int idx = -1;

                // Search through all properties to get proper declaration
                for (asUINT i = 0; i < (asUINT)objectType->GetPropertyCount(); ++i)
                {
                    const char* name;
                    objectType->GetProperty(i, &name);
                    if (decl->declaration == name)
                    {
                        idx = i;
                        break;
                    }
                }

                // If found, add it
                assert(idx >= 0);
                if (idx >= 0) it->second.varMetadataMap.insert(map<int, vector<String> >::value_type(idx, decl->metadata));
            }
        }
        else if (decl->type == MDT_FUNC_OR_VAR)
        {
            if (decl->parentClass == "")
            {
                // Find the global variable index
                int varIdx = module->GetGlobalVarIndexByName(decl->name.c_str());
                if (varIdx >= 0)
                    varMetadataMap.insert(map<int, vector<String> >::value_type(varIdx, decl->metadata));
                else
                {
                    asIScriptFunction* func = module->GetFunctionByDecl(decl->declaration.c_str());
                    assert(func);
                    if (func)
                        funcMetadataMap.insert(map<int, vector<String> >::value_type(func->GetId(), decl->metadata));
                }
            }
            else
            {
                int typeId = module->GetTypeIdByDecl(decl->parentClass.c_str());
                assert(typeId > 0);

                // Add the classes if needed
                map<int, SClassMetadata>::iterator it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end())
                {
                    classMetadataMap.insert(map<int, SClassMetadata>::value_type(typeId, SClassMetadata(decl->parentClass)));
                    it = classMetadataMap.find(typeId);
                }

                // Add the variable to class
                asITypeInfo* objectType = engine->GetTypeInfoById(typeId);
                int idx = -1;

                // Search through all properties to get proper declaration
                for (asUINT i = 0; i < (asUINT)objectType->GetPropertyCount(); ++i)
                {
                    const char* name;
                    objectType->GetProperty(i, &name);
                    if (decl->name == name)
                    {
                        idx = i;
                        break;
                    }
                }

                // If found, add it
                if (idx >= 0)
                    it->second.varMetadataMap.insert(map<int, vector<String> >::value_type(idx, decl->metadata));
                else
                {
                    // Look for the matching method instead
                    asITypeInfo* type = engine->GetTypeInfoById(typeId);
                    asIScriptFunction* func = type->GetMethodByDecl(decl->declaration.c_str());
                    assert(func);
                    if (func)
                        it->second.funcMetadataMap.insert(map<int, vector<String> >::value_type(func->GetId(), decl->metadata));
                }
            }
        }
    }
    module->SetDefaultNamespace("");
        #endif
    
        return 0;
    }
    
    int skipStatement(int pos)
    {
        asUINT len = 0;
    
        // Skip until ; or { whichever comes first
        while (pos < (int)modifiedScript.length() && modifiedScript[pos] != ';' && modifiedScript[pos] != '{')
        {
            engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
            pos += len;
        }
    
        // Skip entire statement block
        if (pos < (int)modifiedScript.length() && modifiedScript[pos] == '{')
        {
            pos += 1;
    
            // Find the end of the statement block
            int level = 1;
            while (level > 0 && pos < (int)modifiedScript.size())
            {
                asETokenClass t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
                if (t == asTC_KEYWORD)
                {
                    if (modifiedScript[pos] == '{')
                        level++;
                    else if (modifiedScript[pos] == '}')
                        level--;
                }
    
                pos += len;
            }
        }
        else
            pos += 1;
    
        return pos;
    }
    
    // Overwrite all code with blanks until the matching #endif
    int excludeCode(int pos)
    {
        asUINT len = 0;
        int nested = 0;
        while (pos < (int)modifiedScript.size())
        {
            engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
            if (modifiedScript[pos] == '#')
            {
                modifiedScript[pos] = ' ';
                pos++;
    
                // Is it an #if or #endif directive?
                engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
                String token;
                token.assign(&modifiedScript[pos], len);
                overwriteCode(pos, len);
    
                if (token == "if")
                {
                    nested++;
                }
                else if (token == "endif")
                {
                    if (nested-- == 0)
                    {
                        pos += len;
                        break;
                    }
                }
            }
            else if (modifiedScript[pos] != '\n')
            {
                overwriteCode(pos, len);
            }
            pos += len;
        }
    
        return pos;
    }
    
    // Overwrite all characters except line breaks with blanks
    void overwriteCode(int start, int len)
    {
        char* code = &modifiedScript[start];
        for (int n = 0; n < len; n++)
        {
            if (*code != '\n')
                *code = ' ';
            code++;
        }
    }
    
    #if AS_PROCESS_METADATA == 1
int CScriptBuilder::ExtractMetadata(int pos, vector<String>& metadata)
{
    metadata.clear();

    // Extract all metadata. They can be separated by whitespace and comments
    for (;;)
    {
        String metadataString = "";

        // Overwrite the metadata with space characters to allow compilation
        modifiedScript[pos] = ' ';

        // Skip opening brackets
        pos += 1;

        int level = 1;
        asUINT len = 0;
        while (level > 0 && pos < (int)modifiedScript.size())
        {
            asETokenClass t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
            if (t == asTC_KEYWORD)
            {
                if (modifiedScript[pos] == '[')
                    level++;
                else if (modifiedScript[pos] == ']')
                    level--;
            }

            // Copy the metadata to our buffer
            if (level > 0)
                metadataString.append(&modifiedScript[pos], len);

            // Overwrite the metadata with space characters to allow compilation
            if (t != asTC_WHITESPACE)
                OverwriteCode(pos, len);

            pos += len;
        }

        metadata.push_back(metadataString);

        // Check for more metadata. Possibly separated by comments
        asETokenClass t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
        while (t == asTC_COMMENT || t == asTC_WHITESPACE)
        {
            pos += len;
            t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
        }

        if (modifiedScript[pos] != '[')
            break;
    }

    return pos;
}

int CScriptBuilder::ExtractDeclaration(int pos, String& name, String& declaration, int& type)
{
    declaration = "";
    type = 0;

    int start = pos;

    std::String token;
    asUINT len = 0;
    asETokenClass t = asTC_WHITESPACE;

    // Skip white spaces, comments, and leading decorators
    do
    {
        pos += len;
        t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
        token.assign(&modifiedScript[pos], len);
    } while (t == asTC_WHITESPACE || t == asTC_COMMENT ||
             token == "private" || token == "protected" ||
             token == "shared" || token == "external" ||
             token == "final" || token == "abstract");

   // We're expecting, either a class, interface, function, or variable declaration
    if (t == asTC_KEYWORD || t == asTC_IDENTIFIER)
    {
        token.assign(&modifiedScript[pos], len);
        if (token == "interface" || token == "class" || token == "enum")
        {
            // Skip white spaces and comments
            do
            {
                pos += len;
                t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
            } while (t == asTC_WHITESPACE || t == asTC_COMMENT);

            if (t == asTC_IDENTIFIER)
            {
                type = MDT_TYPE;
                declaration.assign(&modifiedScript[pos], len);
                pos += len;
                return pos;
            }
        }
        else
        {
            // For function declarations, store everything up to the start of the 
            // statement block, except for succeeding decorators (final, override, etc)

            // For variable declaration store just the name as there can only be one

            // We'll only know if the declaration is a variable or function declaration
            // when we see the statement block, or absense of a statement block.
            bool hasParenthesis = false;
            int nestedParenthesis = 0;
            declaration.append(&modifiedScript[pos], len);
            pos += len;
            for (; pos < (int)modifiedScript.size();)
            {
                t = engine->ParseToken(&modifiedScript[pos], modifiedScript.size() - pos, &len);
                token.assign(&modifiedScript[pos], len);
                if (t == asTC_KEYWORD)
                {
                    if (token == "{" && nestedParenthesis == 0)
                    {
                        if (hasParenthesis)
                        {
                            // We've found the end of a function signature
                            type = MDT_FUNC;
                        }
                        else
                        {
                            // We've found a virtual property. Just keep the name
                            declaration = name;
                            type = MDT_VIRTPROP;
                        }
                        return pos;
                    }
                    if ((token == "=" && !hasParenthesis) || token == ";")
                    {
                        if (hasParenthesis)
                        {
                            // The declaration is ambigous. It can be a variable with initialization, or a function prototype
                            type = MDT_FUNC_OR_VAR;
                        }
                        else
                        {
                            // Substitute the declaration with just the name
                            declaration = name;
                            type = MDT_VAR;
                        }
                        return pos;
                    }
                    else if (token == "(")
                    {
                        nestedParenthesis++;

                        // This is the first parenthesis we encounter. If the parenthesis isn't followed
                        // by a statement block, then this is a variable declaration, in which case we
                        // should only store the type and name of the variable, not the initialization parameters.
                        hasParenthesis = true;
                    }
                    else if (token == ")")
                    {
                        nestedParenthesis--;
                    }
                }
                else if (t == asTC_IDENTIFIER)
                {
                    name = token;
                }

                // Skip trailing decorators
                if (!hasParenthesis || nestedParenthesis > 0 || t != asTC_IDENTIFIER || (token != "final" && token != "override"))
                    declaration += token;

                pos += len;
            }
        }
    }

    return start;
}

vector<String> CScriptBuilder::GetMetadataForType(int typeId)
{
    map<int, vector<String> >::iterator it = typeMetadataMap.find(typeId);
    if (it != typeMetadataMap.end())
        return it->second;

    return vector<String>();
}

vector<String> CScriptBuilder::GetMetadataForFunc(asIScriptFunction* func)
{
    if (func)
    {
        map<int, vector<String> >::iterator it = funcMetadataMap.find(func->GetId());
        if (it != funcMetadataMap.end())
            return it->second;
    }

    return vector<String>();
}

vector<String> CScriptBuilder::GetMetadataForVar(int varIdx)
{
    map<int, vector<String> >::iterator it = varMetadataMap.find(varIdx);
    if (it != varMetadataMap.end())
        return it->second;

    return vector<String>();
}

vector<String> CScriptBuilder::GetMetadataForTypeProperty(int typeId, int varIdx)
{
    map<int, SClassMetadata>::iterator typeIt = classMetadataMap.find(typeId);
    if (typeIt == classMetadataMap.end()) return vector<String>();

    map<int, vector<String> >::iterator propIt = typeIt->second.varMetadataMap.find(varIdx);
    if (propIt == typeIt->second.varMetadataMap.end()) return vector<String>();

    return propIt->second;
}

vector<String> CScriptBuilder::GetMetadataForTypeMethod(int typeId, asIScriptFunction* method)
{
    if (method)
    {
        map<int, SClassMetadata>::iterator typeIt = classMetadataMap.find(typeId);
        if (typeIt == classMetadataMap.end()) return vector<String>();

        map<int, vector<String> >::iterator methodIt = typeIt->second.funcMetadataMap.find(method->GetId());
        if (methodIt == typeIt->second.funcMetadataMap.end()) return vector<String>();

        return methodIt->second;
    }

    return vector<String>();
}
    #endif
};

String GetAbsolutePath(const String& file)
{
    String str = file;

    // If this is a relative path, complement it with the current path
    if (!((str.length() > 0 && (str[0] == '/' || str[0] == '\\')) ||
          str.find(":") != String::npos))
    {
        str = GetCurrentDir() + "/" + str;
    }

    // Replace backslashes for forward slashes
    size_t pos = 0;
    while ((pos = str.find("\\", pos)) != String::npos)
        str[pos] = '/';

    // Replace /./ with /
    pos = 0;
    while ((pos = str.find("/./", pos)) != String::npos)
        str.erase(pos+1, 2);

    // For each /../ remove the parent dir and the /../
    pos = 0;
    while ((pos = str.find("/../")) != String::npos)
    {
        size_t pos2 = str.rfind("/", pos-1);
        if (pos2 != String::npos)
            str.erase(pos2, pos+3-pos2);
        else
        {
            // The path is invalid
            break;
        }
    }

    return str;
}

String GetCurrentDir()
{
    char buffer[1024];
    #if defined(_MSC_VER) || defined(_WIN32)
    #ifdef _WIN32_WCE
    static TCHAR apppath[MAX_PATH] = TEXT("");
    if (!apppath[0])
    {
        GetModuleFileName(NULL, apppath, MAX_PATH);

        int appLen = _tcslen(apppath);

        // Look for the last backslash in the path, which would be the end
        // of the path itself and the start of the filename.  We only want
        // the path part of the exe's full-path filename
        // Safety is that we make sure not to walk off the front of the
        // array (in case the path is nothing more than a filename)
        while (appLen > 1)
        {
            if (apppath[appLen-1] == TEXT('\\'))
                break;
            appLen--;
        }

        // Terminate the String after the trailing backslash
        apppath[appLen] = TEXT('\0');
    }
    #ifdef _UNICODE
    wcstombs(buffer, apppath, min(1024, wcslen(apppath)*sizeof(wchar_t)));
    #else
    memcpy(buffer, apppath, min(1024, strlen(apppath)));
    #endif

    return buffer;
    #elif defined(__S3E__)
    // Marmalade uses its own portable C library
    return getcwd(buffer, (int)1024);
    #elif _XBOX_VER >= 200
    // XBox 360 doesn't support the getcwd function, just use the root folder
    return "game:/";
    #elif defined(_M_ARM)
    // TODO: How to determine current working dir on Windows Phone?
    return "";
    #else
    return _getcwd(buffer, (int)1024);
    #endif // _MSC_VER
    #elif defined(__APPLE__) || defined(__linux__)
    return getcwd(buffer, 1024);
    #else
    return "";
    #endif
}
