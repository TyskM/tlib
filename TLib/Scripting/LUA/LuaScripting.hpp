#pragma once

#include "../../String.hpp"
#include "../Logging.hpp"
#include <deque>
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <magic_enum.hpp>

namespace DocGen
{
#define defineAndDoc(usertype, clas, ret, name, ...)     \
        usertype[STRING_NAME(name)]    = &VecType::name; \
        clas.addMethod(Function(ret, STRING_NAME(name), __VA_ARGS__));

namespace Types
{
    String void_str   = "nil";
    String int_str    = "int";
    String float_str  = "float";
    String double_str = "double";
    String string_str = "string";
    String bool_str   = "boolean";
};

struct Arg
{
    String name;
    String type;
    String docs = ""; // Will only be used with fields, not function args

    Arg(String name, String type = Types::void_str) :
        name{ name }, type{ type } { }
};

struct Function
{
    String name;
    Arg ret = { "", Types::void_str };
    std::vector<Arg> args;
    String docs = ""; // TODO: unused

    Function(String name, std::initializer_list<Arg> args, Arg ret) :
        name{ name }, args{ args }, ret{ ret }
    { }

    Function(String name, std::initializer_list<Arg> args, String ret) :
        name{ name }, args{ args }, ret{ "", ret }
    { }

    template <typename... Args>
    Function(String ret, String name, Args... args)
    {
        constexpr size_t size = sizeof...(args);
        static_assert(size % 2 == 0, "Must have even args count");
        std::vector<String> argsu = { args... };
        
        for (size_t i = 0; i < argsu.size(); i += 2)
        { this->args.push_back(Arg(argsu[i+1], argsu[i])); }

        this->ret  = ret;
        this->name = name;
    }
};

struct Class
{
    String name;
    std::vector<Arg>      fields;
    std::vector<Function> methods;
    //std::vector<Class>    classes; TODO: handle class in class
    String docs = "";

    void addField(const Arg& arg)
    { fields.push_back(arg); }

    void addMethod(const Function& func)
    { methods.push_back(func); }
};

struct DocGenerator
{
    std::deque<Class> classes;
    Class             freeScope;

    // Dont copy by accident
    Class& addClass(const String& name, const String& docs = "")
    {
        classes.emplace_back();
        classes.back().name = name;
        classes.back().docs = docs;
        return classes.back();
    }

    String classToStub(const Class& c)
    {
        std::stringstream ss;
        ss << fmt::format("---@class {}", c.name);
        if (c.docs != "") { ss << "@" << c.docs; }
        ss << "\n";

        for (auto& f : c.fields)
        {
            ss << indent(4) << argToFieldStub(f) << "\n";
        }
        ss << indent(4) << fmt::format("{} = {{ }}", c.name) << "\n\n";

        for (auto& f : c.methods)
        {
            ss << funcToStub(f, &c, 4) << "\n\n";
        }

        return ss.str();
    }

    String funcToStub(const Function& f, const Class* parent = nullptr, size_t indentSize = 0)
    {
        std::stringstream ss;

        // @type
        ss << indent(indentSize) << "---@type fun(";
        for (int i = 0; i < f.args.size(); i++)
        {
            if (i > 0) { ss << ", "; }
            ss << fmt::format("{}: {}", f.args[i].name, f.args[i].type);
        }
        ss << fmt::format(") : {}[]", f.ret.type);
        ss << "\n";

        // @params
        for (auto& arg : f.args)
        {
            ss << indent(indentSize) << fmt::format("---@param {} {}", arg.name, arg.type) << "\n";
        }

        // @return
        ss << indent(indentSize) << fmt::format("---@return {}", f.ret.type);
        if (f.ret.name != "") { ss << " @ " << f.ret.name; }
        ss << "\n";

        // Actual signature
        std::stringstream ssArgList;
        for (int i = 0; i < f.args.size(); i++)
        {
            if (i > 0) { ssArgList << ", "; }
            ssArgList << f.args[i].name;
        }

        if (parent)
        { ss << indent(indentSize) << fmt::format("function {}:{}({}) end;", parent->name, f.name, ssArgList.str()); }
        else
        { ss << indent(indentSize) << fmt::format("function {}({}) end;", f.name, ssArgList.str()); }

        return ss.str();
    }

    String argToFieldStub(const Arg& a)
    {
        return fmt::format("---@field public {} {}", a.name, a.type);
    }

    String toStub()
    {
        // ---@class {CLASS_NAME}
        // ---@field public {FIELD_NAME} {FIELD_TYPE}
        // ---@type fun({ARG_NAME_1}: {ARG_TYPE_1}, {ARG_NAME_2}: {ARG_TYPE_2}) : {RET_TYPE}[]

        std::stringstream ss;
        for (auto& f : freeScope.methods)
        {
            ss << funcToStub(f) << '\n\n';
        }
        ss << "\n\n";
        for (auto& c : classes)
        {
            ss << classToStub(c) << "\n";
        }

        return ss.str();
    }

    String indent(size_t count)
    {
        std::stringstream ss;
        for (size_t i = 0; i < count; i++)
        { ss << " "; }
        return ss.str();
    }

    void stubsToFile(String path)
    {
        writeToFile(path, toStub());
    }
};
}

struct ScriptingEngine : NonAssignable
{
    sol::state lua;
    DocGen::DocGenerator dg;

    ScriptingEngine()
    {
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::string);
    }

    template <typename T>
    static T copy(const T& v) { T newcopy = v; return newcopy; }

    template <typename EnumType>
    void registerEnum(const String& name)
    {
        using namespace DocGen;

        auto table = lua[name].get_or_create<sol::table>();
        Class& enumClass = dg.addClass(name);
        selog->info("Registering enum type: {}", name);
        for (auto& v : magic_enum::enum_entries<EnumType>())
        {
            table[v.second] = v.first;
            enumClass.addField(Arg(String(v.second), Types::int_str));
            selog->info("Registering enum value: {}", String(v.second));
        }
    }

    void registerTLib()
    {
        using namespace DocGen;
        registerEnum<WindowFlags>("WindowFlags");
        registerVec2<Vector2f>(STRING_NAME(Vector2f), Types::float_str);
        registerVec2<Vector2i>(STRING_NAME(Vector2i), Types::int_str);
    }

    template <typename VecType>
    void registerVec2(String vecName, String containedTypeStr)
    {
        using namespace DocGen;
        using namespace Types;

        auto vt = lua.new_usertype<VecType>(
            vecName,
            sol::constructors<
                VecType(),
                VecType(typename VecType::value_type, typename VecType::value_type)>());

        const String& cont = containedTypeStr;
        const String& vector2 = vecName;

        Class& c = dg.addClass(vector2);
        c.addMethod(Function("new", { Arg("x", cont), Arg("y", cont) }, vector2));

        vt["x"]          = &VecType::x;
        c.addField(Arg("x", cont));

        vt["y"]          = &VecType::y;
        c.addField(Arg("y", cont));

        vt["copy"]       = &copy<VecType>;
        c.addMethod(Function(vector2, "copy"));

        defineAndDoc(vt, c, void_str, rotate, cont, "radians");

        vt["rotated"]    = sol::resolve<VecType(const VecType::value_type) const>(&VecType::rotated);
        c.addMethod(Function(vector2, "rotated", cont, "radians"));

        vt["rotated"]    = sol::resolve<VecType(const VecType::value_type, const VecType&) const>(&VecType::rotated);
        c.addMethod(Function(vector2, "rotated", cont, "radians", vector2, "origin" ));

        defineAndDoc(vt, c, void_str, normalize);
        defineAndDoc(vt, c, cont, normalized);
        defineAndDoc(vt, c, cont, dot, vector2, "other");
        defineAndDoc(vt, c, cont, cross, vector2, "other");
        defineAndDoc(vt, c, vector2, reflect, vector2, "normal");
        defineAndDoc(vt, c, cont, length);
        defineAndDoc(vt, c, cont, lengthSquared);
        defineAndDoc(vt, c, cont, distanceTo, vector2, "other");
        defineAndDoc(vt, c, string_str, toString);
        // No rel multiply, it's scuffed
        defineAndDoc(vt, c, vector2, floored);
        defineAndDoc(vt, c, vector2, ceiled);
        defineAndDoc(vt, c, vector2, rounded);
        defineAndDoc(vt, c, vector2, abs);
        defineAndDoc(vt, c, vector2, sqrt);
        defineAndDoc(vt, c, vector2, pow, cont, "value");

        vt[sol::meta_function::addition]       = sol::resolve<VecType(const VecType&) const>( &VecType::operator+   );
        vt[sol::meta_function::subtraction]    = sol::resolve<VecType(const VecType&) const>( &VecType::operator-   );
        vt[sol::meta_function::division]       = sol::resolve<VecType(const VecType&) const>( &VecType::operator/   );
        vt[sol::meta_function::multiplication] = sol::resolve<VecType(const VecType&) const>( &VecType::operator*   );
        vt[sol::meta_function::equal_to]       = sol::resolve<bool(const VecType&) const>   ( &VecType::operator==  );
        vt[sol::meta_function::to_string]      = sol::resolve<String() const>               ( &VecType::toString    );
    }
};
