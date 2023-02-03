#pragma once
#include "../../Logging.hpp"
#include "../../Pointers.hpp"
#include "../../NonAssignable.hpp"
#include "../../String.hpp"
#include "../../Files.hpp"
#include "../../Macros.hpp"
#include "../../Resolve.hpp"
#include <magic_enum.hpp>

// For registering only
#include "../../DataStructures.hpp"
#include "../../Media/Window.hpp" // TODO: Make these separate headers
#include "../../Media/Texture.hpp"

/*
    HOW TO UNFRICK ANGELSCRIPT:
    https://stackoverflow.com/questions/4548763/compiling-assembly-in-visual-studio
    1. Don't use vcpkg its trash
    2. Enable macro assembler
       Go Project->(right click)->Build Dependencies->Build Customizations...->(check)masm
    3. Right-click the .asm file, Properties, change Item Type to "Microsoft Macro Assembler".
    4. ???
    5. Profit
*/
#define AS_GENERATE_DOCUMENTATION 1
#include "../../thirdparty/angelscript/angelscript.h"
#include "../../thirdparty/angelscript/as_scriptengine.h"
#include "../../thirdparty/angelscript/scriptstdstring/scriptstdstring.h"
#include "../../thirdparty/angelscript/scriptstdstring/scriptstdstring.h"
#include "../../thirdparty/angelscript-docgen/docgen.h"
#include "ScriptBuilder.hpp"
#include "../Logging.hpp"

#include <new>

using asDocGen::DocumentationGenerator;
using asDocGen::ScriptDocumentationOptions;

struct ScriptingEngine
{
    String defaultNamespace = "___DEFAULT_NAMESPACE__";
    asIScriptEngine* engine = nullptr;
    UPtr<DocumentationGenerator> docGen;
    fs::path docsOutDir = "mods/Documentation.html";
    fs::path stubsOutDir = "mods/Stubs.hpp";
    ScriptDocumentationOptions _docgenOptions; // Read only

    bool created() { return engine != nullptr; }

    void reset()
    {
        if (engine) { engine->ShutDownAndRelease(); }
        if (docGen) { docGen.reset(); }
    }

    void create(const ScriptDocumentationOptions& docgenOptions = getDefaultDocOptions())
    {
        reset();
        selog->info("Scripting engine created");

        if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
             selog->info("Using generic calling conventions!!!!");
        else selog->info("Using native calling conventions.");

        engine = asCreateScriptEngine(); ASSERT(engine != NULL);

        docGen = make_unique<DocumentationGenerator>(engine, docgenOptions);
        _docgenOptions = docgenOptions;

        //engine->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE, true);
        engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE, true);
        engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, true);

        int r = engine->SetMessageCallback(asFUNCTION(scriptMessageCallback), 0, asCALL_CDECL); ASSERT(r >= 0);
        RegisterStdString(engine);

        r = registerGlobalFunction("void print(const String& in)", &scriptPrint);
        r = docGen->DocumentObjectMethod(r, "Prints the string to console"); ASSERT(r >= 0);

        
        registerGlobalFunction("String toString(float   value)", resolve<float>         (&std::to_string));
        registerGlobalFunction("String toString(double  value)", resolve<double>        (&std::to_string));
        registerGlobalFunction("String toString(int     value)", resolve<int>           (&std::to_string));
        registerGlobalFunction("String toString(int64   value)", resolve<long>          (&std::to_string));
        registerGlobalFunction("String toString(uint64  value)", resolve<unsigned long> (&std::to_string));
        // Default promoted, use int
        registerGlobalFunction("String toString(uint8   value)", resolve<unsigned int>  (&std::to_string));
        registerGlobalFunction("String toString(uint16  value)", resolve<unsigned int>  (&std::to_string));
        registerGlobalFunction("String toString(int8    value)", resolve<int>           (&std::to_string));
        registerGlobalFunction("String toString(int16   value)", resolve<int>           (&std::to_string));

        registerVector2<Vector2f>("Vector2f", "float");
        registerVector2<Vector2i>("Vector2i", "int");
        registerGraphics();
    }

    #pragma region Registration Helpers

    // Used automatically when calling registerObjectRefType
    template <typename Inherit>
    struct RefCounted : Inherit
    {
        using Inherit::Inherit;

        size_t __refCount = 1;

        void __addRef()
        { __refCount++; }

        void __release()
        {
            if (--__refCount == 0)
            { delete this; }
        }

        RefCounted& __copyAndCopyAssignConstructor(const RefCounted& other)
        {
            __addRef();
            return *this;
        }

        static RefCounted* __refCountedFactory()
        {
            return new RefCounted<Inherit>();
        }
    };

    using FuncPtr = asSFuncPtr;

    void resetNamespace() { engine->SetDefaultNamespace(""); }
    void setNamespace(const String& ns) { engine->SetDefaultNamespace(ns.c_str()); }

    template <typename EnumType>
    int registerEnum(const String& name)
    {
        selog->info("Registering enum: {}", name);
        int rr = engine->RegisterEnum(name.c_str()); ASSERT(rr >= 0);
        for (auto& v : magic_enum::enum_entries<EnumType>())
        {
            int r = engine->RegisterEnumValue(name.c_str(), String(v.second).c_str(), static_cast<int>(v.first));
            ASSERT(r >= 0);
        }
        return rr;
    }

    template <typename FuncAddr>
    int registerGlobalFunction(const String& signature, FuncAddr addr)
    {
        selog->info("Registering global function: {}", signature);
        int r = engine->RegisterGlobalFunction(signature.c_str(), asFunctionPtr(addr), asCALL_CDECL);
        ASSERT(r >= 0);
        return r;
    }

    template <typename ObjectType>
    int registerObjectValueType(const String& name)
    {
        selog->info("Registering object value type: {}", name);
        int rr = engine->RegisterObjectType(name.c_str(),
                                           sizeof(ObjectType), asOBJ_VALUE | asGetTypeTraits<ObjectType>());
        ASSERT(rr >= 0);

        const char* delSig = "void del()";
        selog->info("Registering object destructor: {}, {}", name, delSig);
        // Destructors are all the same, right?
        int r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_DESTRUCT,
                                            delSig, asFUNCTION(destructor<ObjectType>),
                                            asCALL_CDECL_OBJFIRST);
        ASSERT(r >= 0);

        if (std::is_default_constructible<ObjectType>::value)
        {
            r = registerObjectConstructor<ObjectType>(name, "void new()");
            ASSERT(r >= 0);
        }

        if (std::is_copy_constructible<ObjectType>::value)
        {
            r = registerObjectConstructor<ObjectType>( name, fmt::format(fmt::runtime("void new(const {}&in)"), name));
            ASSERT(r >= 0);
        }

        return rr;
    }

    template <typename ObjectType>
    int registerObjectRefType(const String& name)
    {
        selog->info("Registering object reference type: {}", name);
        using RefObjectType = RefCounted<ObjectType>;

        int rr = engine->RegisterObjectType((name).c_str(), 0, asOBJ_REF); assert(rr >= 0);
        int r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_FACTORY,
                                            fmt::format(fmt::runtime("{}@ new()"), name).c_str(),
                                            asFunctionPtr(&RefCounted<RefObjectType>::__refCountedFactory),
                                            asCALL_CDECL); assert(r >= 0);

        r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ADDREF, "void ref()",
                                            getMethodPtr<RefObjectType>(&RefObjectType::__addRef), asCALL_THISCALL); assert(r >= 0);

        r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_RELEASE, "void release()",
                                            getMethodPtr<RefObjectType>(&RefObjectType::__release), asCALL_THISCALL); assert(r >= 0);

        return rr;
    }

    // Signature should return void
    // Only the params in the signature matter ie. "coolSig(float, float)" is the same as "lameSig(float, float)"
    template <typename ObjectType, typename... ConstructorParams>
    int registerObjectConstructor(const String& name, const String& signature)
    {
        selog->info("Registering object constructor: {}, {}", name, signature);
        int r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_CONSTRUCT,
                                            signature.c_str(), getConstructorFuncPtr<ObjectType, ConstructorParams...>(),
                                            asCALL_CDECL_OBJFIRST); ASSERT(r >= 0);
        return r;
    }

    // Signature example: "float x"
    // funcAddr example: &Type::x
    template <typename FuncAddr>
    int registerObjectProperty(const String& name, const String& signature, FuncAddr funcAddr)
    {
        selog->info("Registering object property: {}, {}", name, signature);
        int r = engine->RegisterObjectProperty(name.c_str(), signature.c_str(), offsetOf(funcAddr));
        ASSERT(r >= 0);
        return r;
    }

    // Signature example:
    // "String toString()"
    template <typename ObjectType, typename FuncAddr>
    int registerObjectMethod(const String& name, const String& signature, FuncAddr funcAddr)
    {
        selog->info("Registering method: {}, {}", name, signature);
        int r = engine->RegisterObjectMethod(name.c_str(), signature.c_str() ,
                                         getMethodPtr<ObjectType>(funcAddr), asCALL_THISCALL);

        ASSERT(r >= 0);
    }

    template <typename T, typename... Args>
    static void constructor(T* thisPointer, Args... args)
    { new(thisPointer) T(args...); }

    // offsetOf(&YourType::YourTypesParameter);
    template<typename T, typename U>
    constexpr size_t offsetOf(U T::* member)
    { return (char*)&((T*)nullptr->*member) - (char*)nullptr; }

    // getMethodPtr<YourClass>(&YourClass::YourMethod)
    template <typename ValueType, typename MethodAddr>
    FuncPtr getMethodPtr(MethodAddr addr)
    { return asSMethodPtr<sizeof(void (ValueType::*)())>::Convert((void (ValueType::*)())(addr)); }

    template <typename T, typename... Params>
    static FuncPtr getConstructorFuncPtr()
    { return asFunctionPtr(&constructor<T, Params...>); }

    // Works poorly, don't bother
    //template <typename T>
    //void autoRegisterType()
    //{
    //    using namespace refl;
    //    using value_type = T;
    //
    //    auto ty = type::get<value_type>();
    //    const String className = ty.get_name().to_string();
    //
    //    // Main
    //    selog->info("Registering Type: {}", className);
    //    int r = engine->RegisterObjectType(className.c_str(), sizeof(value_type), asOBJ_VALUE | asGetTypeTraits<value_type>()); ASSERT(r >= 0);
    //
    //    // Constructors
    //    for (auto& c : ty.get_constructors())
    //    {
    //        std::stringstream ss;
    //        ss << "void new";
    //
    //        bool first = true;
    //        ss << "(";
    //        for (auto& param : c.get_parameter_infos())
    //        {
    //            if (!first) { ss << ", "; }
    //            first = false;
    //
    //            if (param.get_name().size() > 0)
    //            { ss << param.get_type().get_name().to_string() << " " << param.get_name().to_string(); }
    //            else
    //            { ss << param.get_type().get_name().to_string(); }
    //        }
    //        ss << ")";
    //
    //        selog->info("Registering constructor: {}", ss.str());
    //        r = engine->RegisterObjectBehaviour(className.c_str(), asBEHAVE_CONSTRUCT,
    //                                            ss.str().c_str(), refl::getConstructorPtr(c), asCALL_CDECL_OBJFIRST); ASSERT(r >= 0);
    //    }
    //
    //    // Destructor
    //    r = engine->RegisterObjectBehaviour(className.c_str(), asBEHAVE_DESTRUCT, "void del()", asFUNCTION(destructor<value_type>), asCALL_CDECL_OBJFIRST); ASSERT(r >= 0);
    //
    //    // Methods
    //    for (method& m : ty.get_methods())
    //    {
    //        const String ret = m.get_return_type().get_name().to_string();
    //        String methodName = m.get_name().to_string();
    //
    //        // float angleTo( Vector2f&in )
    //        std::stringstream ss;
    //        m.get_return_type().
    //
    //            ss << ret << " ";
    //
    //        String opStr = cppOpToASOp(methodName, m.get_parameter_infos().size() > 0);
    //        if (opStr == "DROP") { continue; }
    //        else if (opStr != "NULL") { methodName = opStr; }
    //
    //
    //        ss << methodName;
    //
    //        bool first = true;
    //        ss << "(";
    //        for (auto& param : m.get_parameter_infos())
    //        {
    //            if (!first) { ss << ", "; }
    //            first = false;
    //
    //            if (param.get_name().size() > 0)
    //            { ss << param.get_type().get_name().to_string() << " " << param.get_name().to_string(); }
    //            else
    //            { ss << param.get_type().get_name().to_string(); }
    //        }
    //        ss << ")";
    //
    //        selog->info("Registering method: {}", ss.str());
    //        r = engine->RegisterObjectMethod(className.c_str(), ss.str().c_str(),
    //                                         getMethodPtr(m), asCALL_THISCALL); /*ASSERT(r >= 0);*/
    //    }
    //}

    #pragma endregion

    #pragma region Registration Presets

    template <typename T = Vector2f>
    void registerVector2(const char* type = "Vector2f", const char* wrapped = "float")
    {
        using wrappedType = T::value_type;
        using namespace fmt::literals;

        auto f = [type, wrapped](const String& str)
        {
            return fmt::format(fmt::runtime(str), fmt::arg("type", type), fmt::arg("wrapped", wrapped));
        };

        registerObjectValueType<T>(type);
        registerObjectConstructor<T, float, float>(type, f("void new({wrapped} x, {wrapped} y)"));
        registerObjectProperty (type, f("{wrapped} x"), &T::x);
        registerObjectProperty (type, f("{wrapped} y"), &T::y);
        registerObjectMethod<T>(type, f("{wrapped} angle()"), &T::angle);
        registerObjectMethod<T>(type, f("{wrapped} angleTo({type}&in other)"), &T::angleTo);
        registerObjectMethod<T>(type, f("{wrapped} angleToRel({type}&in other)"), &T::angleToRel);
        registerObjectMethod<T>(type, f("void      rotate({wrapped} radians)"), &T::rotate);
        registerObjectMethod<T>(type, f("{type}    rotated({wrapped})"), resolve<const wrappedType>(&T::rotated));
        registerObjectMethod<T>(type, f("{type}    rotated({wrapped} radians, {type}&in origin)"), resolve<const wrappedType, const T&>(&T::rotated));
        registerObjectMethod<T>(type, f("void      normalize()"), &T::normalize);
        registerObjectMethod<T>(type, f("{type}    normalized()"), &T::normalized);
        registerObjectMethod<T>(type, f("{wrapped} dot({type}&in other)"), &T::dot);
        registerObjectMethod<T>(type, f("{wrapped} cross({type}&in other)"), &T::cross);
        registerObjectMethod<T>(type, f("{type}    reflect({type}&in normal)"), &T::reflect);
        registerObjectMethod<T>(type, f("{wrapped} length()"), &T::length);
        registerObjectMethod<T>(type, f("{wrapped} lengthSquared()"), &T::lengthSquared);
        registerObjectMethod<T>(type, f("{wrapped} distanceTo({type}&in other)"), &T::distanceTo);
        registerObjectMethod<T>(type, f("{wrapped} distanceToSquared({type}&in other)"), &T::distanceToSquared);
        registerObjectMethod<T>(type, f("{type}    floored()"), &T::floored);
        registerObjectMethod<T>(type, f("{type}    ceiled()"), &T::ceiled);
        registerObjectMethod<T>(type, f("{type}    rounded()"), &T::rounded);
        registerObjectMethod<T>(type, f("{type}    abs()"), &T::abs);
        registerObjectMethod<T>(type, f("{type}    sqrt()"), &T::sqrt);
        registerObjectMethod<T>(type, f("{type}    pow({wrapped} value)"), &T::pow);
        registerObjectMethod<T>(type, f("String    toString()"), &T::toString);
        registerObjectMethod<T>(type, f("{type}&   opAssign({type}&in other)"), resolve<const T&>(&T::operator=));
        registerObjectMethod<T>(type, f("bool      opEquals({type}&in other)"), &Vector2f::operator==);
        registerObjectMethod<T>(type, f("{type}    opNeg()"), resolve< >(&T::operator-));
        registerObjectMethod<T>(type, f("{type}    opAdd(const {type}&in other)"), &Vector2f::operator+);
        registerObjectMethod<T>(type, f("{type}    opSub(const {type}&in other)"), resolve<const T&>(&T::operator-));
        registerObjectMethod<T>(type, f("{type}    opMul(const {type}&in other)"), resolve<const T&>(&T::operator*));
        registerObjectMethod<T>(type, f("{type}    opDiv(const {type}&in other)"), resolve<const T&>(&T::operator/));
        registerObjectMethod<T>(type, f("{type}    opMul(const int&in other)"), resolve<const int&>(&T::operator*));
        registerObjectMethod<T>(type, f("{type}    opDiv(const int&in other)"), resolve<const int&>(&T::operator/));
        registerObjectMethod<T>(type, f("{type}    opMul(const float&in other)"), resolve<const float&>(&T::operator*));
        registerObjectMethod<T>(type, f("{type}    opDiv(const float&in other)"), resolve<const float&>(&T::operator/));
        registerObjectMethod<T>(type, f("{type}    opAddAssign(const {type}&in other)"), &T::operator+=);
        registerObjectMethod<T>(type, f("{type}    opSubAssign(const {type}&in other)"), &T::operator-=);
        registerObjectMethod<T>(type, f("{type}    opMulAssign(const {type}&in other)"), resolve<const T&>(&T::operator*=));
        registerObjectMethod<T>(type, f("{type}    opDivAssign(const {type}&in other)"), resolve<const T&>(&T::operator/=));
        registerObjectMethod<T>(type, f("{type}    opMulAssign(const int&in other)"), resolve<const int&>(&T::operator*=));
        registerObjectMethod<T>(type, f("{type}    opDivAssign(const int&in other)"), resolve<const int&>(&T::operator/=));
        registerObjectMethod<T>(type, f("{type}    opMulAssign(const float&in other)"), resolve<const float&>(&T::operator*=));
        registerObjectMethod<T>(type, f("{type}    opDivAssign(const float&in other)"), resolve<const float&>(&T::operator/=));
    }

    void registerGraphics()
    {
        registerEnum<WindowFlags>("WindowFlags");

        const String winCreateParams = "WindowCreateParams";

        registerObjectValueType<WindowCreateParams>(winCreateParams);
        registerObjectProperty(winCreateParams, "String      title", &WindowCreateParams::title);
        registerObjectProperty(winCreateParams, "Vector2i    size",  &WindowCreateParams::size);
        registerObjectProperty(winCreateParams, "Vector2i    pos",   &WindowCreateParams::pos);
        registerObjectProperty(winCreateParams, "WindowFlags flags", &WindowCreateParams::flags);

        const String winName = "Window";
        registerObjectRefType<Window>(winName);
        registerObjectMethod<Window>(winName, "void create(const WindowCreateParams&in params)", &Window::create);
        registerObjectMethod<Window>(winName, "Vector2i getSize() const", &Window::getSize);

        registerEnum<TextureFiltering>("TextureFiltering");
        const String texName = "Texture";
        registerObjectRefType<Texture>(texName);
        registerObjectMethod<Texture>(texName, "bool loadFromFile(String path)", &Texture::loadFromFile);
        registerObjectMethod<Texture>(texName, "bool loadFromFile(String path, TextureFiltering filterMode)", &Texture::loadFromFile);
        registerObjectMethod<Texture>(texName, "void reset()", &Texture::reset);
        registerObjectMethod<Texture>(texName, "Vector2i getSize()", &Texture::getSize);
    }

    #pragma endregion

    #pragma region Execution Helpers

    using ScriptFunc = asIScriptFunction*;

    struct ScriptContext : NonCopyable
    {
        asIScriptContext* ptr = nullptr;

        ScriptContext() = default;
        ~ScriptContext() { reset(); }
        ScriptContext(ScriptContext&& other) noexcept
        {
            ptr = other.ptr;
            other.ptr = nullptr;
        }

        void create(ScriptingEngine& engine)
        {
            ptr = engine.engine->CreateContext();
        }

        void reset()
        {
            if (ptr) { ptr->Release(); }
        }

        bool created() { return ptr != nullptr; }

        void prepare(asIScriptFunction* func)
        {
            ASSERT(created());
            ptr->Prepare(func);
        }

        void execute()
        {
            ASSERT(created());

            int r = ptr->Execute();
            if (r != asEXECUTION_FINISHED)
            {
                // The execution didn't complete as expected. Determine what happened.
                if (r == asEXECUTION_EXCEPTION)
                {
                    // An exception occurred, let the script writer know what happened so it can be corrected.
                    selog->error("An exception '{}' occurred. Please correct the code and try again.", ptr->GetExceptionString());
                }
            }
        }

        // 0 is first index
        // Call after prepare, and before execute
        void setArg(asUINT index, float value) { ptr->SetArgFloat(index, value); }
        void setArg(asUINT index, double value) { ptr->SetArgDouble(index, value); }
    };

    // 1. Init by calling create()
    // 2. Then add script sections with addSectionFromFile()
    // 3. Finally, call build()
    // Then all methods are valid :)))
    struct ScriptModule
    {
        String name;
        ScriptBuilder builder;
        ScriptingEngine* engine = nullptr;
        asIScriptModule* module = nullptr;

        bool valid() { return engine != nullptr && module != nullptr; }

        void create(ScriptingEngine& engine, const String& name)
        {
            this->name = name;
            this->engine = &engine;
            int r = builder.startNewModule(engine.engine, name.c_str());
            if (r < 0)
            {
                // If the code fails here it is usually because there
                // is no more memory to allocate the module
                printf("Unrecoverable error while starting a new module.\n");
                return;
            }
        }

        void addSectionFromFile(const String& path)
        {
            int r = builder.addSectionFromFile(path.c_str());
            if (r < 0)
            {
                selog->error(R"(
                    Unable to load file {}.
                    Maybe the file has been removed,
                    or the wrong name was given,
                    or some preprocessing commands are incorrectly written)", path);
                return;
            }
        }

        void build()
        {
            int r = builder.buildModule();
            if (r < 0)
            {
                // An error occurred. Instruct the script writer to fix the 
                // compilation errors that were listed in the output stream.
                selog->error(R"(A script error occured,
                           Please correct the errors in the script and try again.)");
                ASSERT(false);
                return;
            }
            module = engine->engine->GetModule(name.c_str());
        }

        void setNamespace(const String& ns)
        {
            ASSERT(valid());
            module->SetDefaultNamespace(ns.c_str());
        }

        void resetNamespace()
        {
            ASSERT(valid());
            module->SetDefaultNamespace("");
        }

        ScriptFunc getFuncByDecl(const String& funcDecl)
        {
            ASSERT(valid());
            return module->GetFunctionByDecl(funcDecl.c_str());
        }
    };

    void runFile(const String& path, const String& entryPointSig = "void main()", const String& moduleName = "_runfile")
    {
        ScriptModule smod;
        smod.create(*this, moduleName);
        smod.addSectionFromFile(path);
        smod.build();
        ScriptFunc func = smod.getFuncByDecl(entryPointSig);
        ScriptContext ctx;
        ctx.create(*this);
        ctx.prepare(func);
        ctx.execute();
    }


    #pragma endregion

    #pragma region Misc

    static ScriptDocumentationOptions getDefaultDocOptions()
    {
        ScriptDocumentationOptions docgenOptions;
        docgenOptions.projectName             = "TLib";
        docgenOptions.outputFile              = "mods/documentation.html";
        docgenOptions.documentationName       = "Mod API";
        docgenOptions.includeArrayInterface   = true;
        docgenOptions.includeStringInterface  = true;
        docgenOptions.includeWeakRefInterface = true;
        docgenOptions.htmlSafe = false; // we want to be able to style ourselves with tags, e.g. <b></b>
        return docgenOptions;
    }

    #pragma endregion

    #pragma region Script Functions

    static void scriptMessageCallback(const asSMessageInfo* msg, void* param)
    {
        String logmsg = fmt::format("[AS] {} ({}, {}) {}", msg->section, msg->row, msg->col, msg->message);
        switch (msg->type)
        {
            case asMSGTYPE_INFORMATION:
                selog->info(logmsg);
                break;
            case asMSGTYPE_WARNING:
                selog->warn(logmsg);
                break;
            case asMSGTYPE_ERROR:
            default:
                selog->error(logmsg);
                break;
        }
    }

    static void scriptPrint(const std::string& str)
    {
        selog->info(str);
    }

    template <typename T>
    static void destructor(void* memory)
    { ((T*)memory)->~T(); }

    #pragma endregion

    #pragma region Doc/Stub Generation

    static inline std::vector<String> blacklistedStubFunctions =
    {
        "formatFloat",
        "formatInt",
        "formatUInt",
        "parseFloat",
        "parseInt",
        "parseUInt"
    };

    void generateDocs()
    { docGen->Generate(); }

    void generateStubs()
    {
        std::stringstream ss;

        #pragma region Boilerplate
        ss <<
            R"(
#pragma break

#define int8 signed char
#define int16 signed short
#define int64 long
#define uint unsigned
#define uint8 unsigned char
#define uint16 unsigned short
#define uint64 unsigned long
#define null 0
#define in
#define out
#define inout
#define is ==
#define interface struct
#define cast reinterpret_cast
#define mixin
#define funcdef
#define protected
#define private

)";
        #pragma endregion

        #pragma region Enums

        size_t enumCount = engine->GetEnumCount();
        for (size_t i = 0; i < enumCount; ++i)
        {
            auto enumm = engine->GetEnumByIndex(i);
            ss << "enum class " << enumm->GetName() << "\n{\n";

            size_t valueCount = enumm->GetEnumValueCount();
            for (size_t k = 0; k < valueCount - 1; k++)
            {
                int value;
                const char* name = enumm->GetEnumValueByIndex(k, &value);
                ss << indent(4) << name << " = " << std::to_string(value) << ",\n";
            }

            // Write last one without ending comma
            int value;
            const char* name = enumm->GetEnumValueByIndex(valueCount - 1, &value);
            ss << indent(4) << name << " = " << std::to_string(value) << "\n";

            ss << "};\n";
        }

        #pragma endregion

        #pragma region Classes
        ss << "\n";

        // Get and sort
        auto comp = [](const asITypeInfo* a, const asITypeInfo* b)
        {
            int i = strcmp(a->GetName(), b->GetName());
            if (i < 0) return true;
            if (i > 0) return false;
            return a < b;
        };

        std::set<const asITypeInfo*, decltype(comp)> objectTypes;
        for (int t = 0, tcount = engine->GetObjectTypeCount(); t < tcount; ++t)
        {
            const asITypeInfo* const typeInfo = engine->GetObjectTypeByIndex(t);
            objectTypes.insert(typeInfo);
        }

        for (auto& type : objectTypes)
        {
            // Write class docs
            if (docGen->impl->objectTypeDocumentation.contains(type))
            { ss << "// " << docGen->impl->objectTypeDocumentation[type] << "\n"; }

            // Write class signature
            String sigStr = type->GetName();
            ss << "struct " << sigStr << "\n{\n";

            // Properties
            size_t propertyCount = type->GetPropertyCount();
            for (size_t i = 0; i < propertyCount; ++i)
            {
                String propSig = type->GetPropertyDeclaration(i);
                ss << indent(4) << propSig << ";" << "\n";
            }

            // Methods
            size_t methodCount = type->GetMethodCount();
            if (methodCount > 0 && propertyCount > 0) { ss << "\n"; }
            for (size_t i = 0; i < methodCount; ++i)
            {
                auto method = type->GetMethodByIndex(i);
                if (docGen->impl->functionDocumentation.contains(method))
                { ss << indent(4) << "\n// " << docGen->impl->functionDocumentation[method] << "\n"; }

                String sigStr = method->GetDeclaration(false, false, true);
                String methodName = method->GetName();

                String opStr = asOpToCppOp(methodName);
                if (opStr == "DROP") { continue; }
                else if (opStr != "NULL")
                { strhelp::replaceFirst(sigStr, methodName, opStr); }

                ss << indent(4) << sigStr << ";\n";

                // Generate a not equals op, because AS doesn't have one.
                if (opStr == "operator==")
                {
                    strhelp::replaceFirst(sigStr, "operator==", "operator!=");
                    ss << indent(4) << sigStr << ";\n";
                }

                // TODO: Handle less-than, and greater-than ops zzzz
            }

            // Behaviors ie. constructor, destructor
            const asEBehaviours behaviors[] ={ asBEHAVE_CONSTRUCT, asBEHAVE_FACTORY, asBEHAVE_LIST_FACTORY, asBEHAVE_DESTRUCT };
            size_t behCount = type->GetBehaviourCount();

            for (auto beh : behaviors)
            {
                for (size_t i = 0; i < behCount; ++i)
                {
                    asEBehaviours b;
                    auto behFunc = type->GetBehaviourByIndex(i, &b);
                    if (beh == b)
                    { ss << indent(4) << behFunc->GetDeclaration(false, false, true) << ";\n"; }
                }
            }

            // HACK: A hack to stop intellisense errors
            String name = type->GetName();
            if (name == String("String"))
            { ss << indent(4) << "String(const char*); // Ignore\n"; }

            ss << "};\n\n";
        }
        #pragma endregion

        #pragma region Globals
        const int funcCount = engine->GetGlobalFunctionCount();
        std::vector<const asIScriptFunction*> globalFunctions;
        if (funcCount)
        {
            // Get global functions
            globalFunctions.reserve(funcCount);
            for (int i = 0; i < funcCount; ++i)
            {
                globalFunctions.push_back(engine->GetGlobalFunctionByIndex(i));
                auto& func = globalFunctions.back();
            }

            // Sort global functions
            // std::sort(globalFunctions.begin(), globalFunctions.end(), [](const asIScriptFunction* a, const asIScriptFunction* b)
            // { return strcmp(a->GetName(), b->GetName()) < 0; });

            for (auto& func : globalFunctions)
            {
                if (std::find(blacklistedStubFunctions.begin(), blacklistedStubFunctions.end(),
                              func->GetName()) != blacklistedStubFunctions.end())
                {
                    continue;
                }

                // Write docs
                if (docGen->impl->functionDocumentation.contains(func))
                { ss << "\n// " << docGen->impl->functionDocumentation[func] << "\n"; }

                // Write signature
                String sigStr = func->GetDeclaration(false, false, true);
                //strhelp::replace(sigStr, "&", " ");
                ss << sigStr << ";\n";
            }
        }
        #pragma endregion

        // Ending boilerplate
        ss << "#define class struct\n";

        selog->info("Writing stubs to file: \"{}\"", stubsOutDir.string());
        writeToFile(stubsOutDir, ss.str());
    }

    // "NULL" if not an operator
    // "DROP" if should not parse
    static String cppOpToASOp(String cppOp, bool hasParams)
    {
        if (cppOp == "operator[]") { return "opIndex"; }
        else if (cppOp == "operator-" && !hasParams) { return "opNeg"; }
        else if (cppOp == "operator~") { return "opCom"; }
        else if (cppOp == "operator++") { return "opPreInc"; }
        else if (cppOp == "operator--") { return "opPreDec"; }
        else if (cppOp == "operator++") { return "opPostInc"; }
        else if (cppOp == "operator--") { return "opPostDec"; }
        else if (cppOp == "operator==") { return "opEquals"; }
        else if (cppOp == "operator!=") { return "DROP" /*"opEquals"*/; } // TODO: Handle duped operators
        else if (cppOp == "operator<") { return "opCmp"; }
        else if (cppOp == "operator<=") { return "DROP" /*"opCmp"*/; }
        else if (cppOp == "operator>") { return "DROP" /*"opCmp"*/; }
        else if (cppOp == "operator>=") { return "DROP" /*"opCmp"*/; }
        else if (cppOp == "operator=") { return "opAssign"; }
        else if (cppOp == "operator+=") { return "opAddAssign"; }
        else if (cppOp == "operator-=") { return "opSubAssign"; }
        else if (cppOp == "operator*=") { return "opMulAssign"; }
        else if (cppOp == "operator/=") { return "opDivAssign"; }
        else if (cppOp == "operator%=") { return "opModAssign"; }
        else if (cppOp == "operator**=") { return "opPowAssign"; }
        else if (cppOp == "operator&=") { return "opAndAssign"; }
        else if (cppOp == "operator|=") { return "opOrAssign"; }
        else if (cppOp == "operator^=") { return "opXorAssign"; }
        else if (cppOp == "operator<<=") { return "opShlAssign"; }
        else if (cppOp == "operator>>=") { return "opShrAssign"; }
        else if (cppOp == "operator>>>=") { return "opUShrAssign"; }
        else if (cppOp == "operator+") { return "opAdd"; }
        else if (cppOp == "operator-") { return "opSub"; }
        else if (cppOp == "operator*") { return "opMul"; }
        else if (cppOp == "operator/") { return "opDiv"; }
        else if (cppOp == "operator%") { return "opMod"; }
        else if (cppOp == "operator**") { return "opPow"; }
        else if (cppOp == "operator&") { return "opAnd"; }
        else if (cppOp == "operator|") { return "opOr"; }
        else if (cppOp == "operator^") { return "opXor"; }
        else if (cppOp == "operator<<") { return "opShl"; }
        else if (cppOp == "operator>>") { return "opShr"; }
        else if (cppOp == "operator>>>") { return "opUShr"; }

        return "NULL";
    }

    static String asOpToCppOp(String asOp)
    {
        if (asOp == "opIndex") { return "operator[]"  ; }
        else if (asOp == "opNeg") { return "operator-"   ; }
        else if (asOp == "opCom") { return "operator~"   ; }
        else if (asOp == "opPreInc") { return "operator++"  ; }
        else if (asOp == "opPreDec") { return "operator--"  ; }
        else if (asOp == "opPostInc") { return "operator++"  ; }
        else if (asOp == "opPostDec") { return "operator--"  ; }
        else if (asOp == "opEquals") { return "operator=="  ; }
        else if (asOp == "opEquals") { return "operator!="  ; }
        else if (asOp == "opCmp") { return "operator<"   ; }
        else if (asOp == "opCmp") { return "operator<="  ; }
        else if (asOp == "opCmp") { return "operator>"   ; }
        else if (asOp == "opCmp") { return "operator>="  ; }
        else if (asOp == "opAssign") { return "operator="   ; }
        else if (asOp == "opAddAssign") { return "operator+="  ; }
        else if (asOp == "opSubAssign") { return "operator-="  ; }
        else if (asOp == "opMulAssign") { return "operator*="  ; }
        else if (asOp == "opDivAssign") { return "operator/="  ; }
        else if (asOp == "opModAssign") { return "operator%="  ; }
        else if (asOp == "opPowAssign") { return "operator**=" ; }
        else if (asOp == "opAndAssign") { return "operator&="  ; }
        else if (asOp == "opOrAssign") { return "operator|="  ; }
        else if (asOp == "opXorAssign") { return "operator^="  ; }
        else if (asOp == "opShlAssign") { return "operator<<=" ; }
        else if (asOp == "opShrAssign") { return "operator>>=" ; }
        else if (asOp == "opUShrAssign") { return "operator>>>="; }
        else if (asOp == "opAdd") { return "operator+"   ; }
        else if (asOp == "opSub") { return "operator-"   ; }
        else if (asOp == "opMul") { return "operator*"   ; }
        else if (asOp == "opDiv") { return "operator/"   ; }
        else if (asOp == "opMod") { return "operator%"   ; }
        else if (asOp == "opPow") { return "operator**"  ; }
        else if (asOp == "opAnd") { return "operator&"   ; }
        else if (asOp == "opOr") { return "operator|"   ; }
        else if (asOp == "opXor") { return "operator^"   ; }
        else if (asOp == "opShl") { return "operator<<"  ; }
        else if (asOp == "opShr") { return "operator>>"  ; }
        else if (asOp == "opUShr") { return "operator>>>" ; }

        else if (asOp == "opAdd_r") { return "DROP"; }
        else if (asOp == "opSub_r") { return "DROP"; }
        else if (asOp == "opMul_r") { return "DROP"; }
        else if (asOp == "opDiv_r") { return "DROP"; }
        else if (asOp == "opMod_r") { return "DROP"; }
        else if (asOp == "opPow_r") { return "DROP"; }
        else if (asOp == "opAnd_r") { return "DROP"; }
        else if (asOp == "opOr_r") { return "DROP"; }
        else if (asOp == "opXor_r") { return "DROP"; }
        else if (asOp == "opShl_r") { return "DROP"; }
        else if (asOp == "opShr_r") { return "DROP"; }
        else if (asOp == "opUShr_r") { return "DROP"; }

        return "NULL";
    }

    static String indent(size_t count)
    {
        std::stringstream ss;
        for (size_t i = 0; i < count; i++)
        { ss << " "; }
        return ss.str();
    }

    #pragma endregion
};

using ScriptModule  = ScriptingEngine::ScriptModule;
using ScriptContext = ScriptingEngine::ScriptContext;
using ScriptFunc    = ScriptingEngine::ScriptFunc;